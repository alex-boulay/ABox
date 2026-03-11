#pragma once

#include <PreProcUtils.hpp>
#include <VersionedSlot.hpp>
#include <cstdint>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>

/**
 * @brief Calculate optimal multiplier for cache-aligned blocks
 * @tparam T Element type
 * @return Multiplier value (blocks = 8 * multiplier elements)
 */
template <typename T> inline constexpr size_t cache_aligned_multplier()
{
  constexpr size_t elements_per_cache_line = ABOX_CACHE_LINE_SIZE / sizeof(T);
  constexpr size_t multiplier              = (elements_per_cache_line + 7) / 8;
  return multiplier > 0 ? (multiplier > 16 ? 16 : multiplier) : 1;
}

/**
 * @class FetchList
 * @brief Colony-style allocator with separate bitmap and element storage
 *
 * Grows dynamically by allocating blocks of 8 * multiplier elements.
 * Bitmaps are stored separately from element blocks for better cache
 * efficiency.
 *
 * @tparam T The type of elements stored
 * @tparam Allocator The allocator type (default: std::allocator<T>)
 *
 * @details
 * Memory layout:
 * - Bitmaps: Separate array of all bitmaps (multiplier bytes per block)
 * - Blocks: Array of element blocks (8 * multiplier elements per block)
 *
 * Performance characteristics:
 * - Bitmaps separated for efficient scanning
 * - Stable pointers (blocks don't move once allocated)
 * - O(1) size tracking
 * - Fast allocation/deallocation with bitmap tracking
 * - Efficient slot reuse
 */

// std::uint_fast8_t size to map
// alloc chunking ?
template <typename T, typename Allocator = std::allocator<T>>
class FetchList {
   public:
  using VersionType = VersionedSlot::UWord;

  /**
   * @brief Handle for versioned access to elements
   */
  struct Handle {
    size_t      index;
    VersionType version;

    Handle()
        : index(static_cast<size_t>(-1))
        , version(0)
    {
    }

    Handle(size_t idx, VersionType ver)
        : index(idx)
        , version(ver)
    {
    }

    bool isValid() const { return index != static_cast<size_t>(-1); }

    bool operator==(const Handle &other) const
    {
      return index == other.index && version == other.version;
    }

    bool operator!=(const Handle &other) const { return !(*this == other); }
  };

   private:
  T              **blocks_;   ///< Array of element blocks
  VersionedSlot **versions_;  ///< Array of version tracking blocks
  size_t   block_count_;    ///< Number of allocated blocks
  size_t   block_capacity_; ///< size_t sizes constrained to a chunksize
  size_t   size_;           ///< Current number of occupied elements
  size_t   multiplier_; ///< Size multiplier per block
  size_t   elements_per_block_;

   protected:
  /**
   * @brief Grow capacity by allocating a new block
   */
  void grow()
  {
    // Check if we need to expand block arrays
    if (block_count_ >= block_capacity_) {
      size_t new_capacity = block_capacity_ == 0 ? 4 : block_capacity_ * 2;

      // Reallocate block pointer arrays
      T **new_blocks = new T *[new_capacity];
      VersionedSlot **new_versions = new VersionedSlot *[new_capacity];

      // Copy existing pointers
      if (blocks_) {
        std::memcpy(new_blocks, blocks_, block_count_ * sizeof(T *));
        delete[] blocks_;
      }
      if (versions_) {
        std::memcpy(
            new_versions,
            versions_,
            block_count_ * sizeof(VersionedSlot *)
        );
        delete[] versions_;
      }

      blocks_         = new_blocks;
      versions_       = new_versions;
      block_capacity_ = new_capacity;
    }

    // Allocate new element block (raw memory, no construction)
    blocks_[block_count_] = static_cast<T *>(
        ::operator new(elements_per_block_ * sizeof(T))
    );

    // Allocate new version tracking block
    versions_[block_count_] = new VersionedSlot[elements_per_block_];

    ++block_count_;
  }

  /**
   * @brief Find first free slot across all blocks
   * @return Index of free slot, or -1 if full
   */
  int find_free_slot() const
  {
    size_t total_elements = block_count_ * elements_per_block_;
    for (size_t i = 0; i < total_elements; ++i) {
      size_t block_idx   = i / elements_per_block_;
      size_t element_idx = i % elements_per_block_;
      if (versions_[block_idx][element_idx].state() ==
          VersionedSlot::FREE) {
        return static_cast<int>(i);
      }
    }
    return -1;
  }

   public:
  /**
   * @brief Construct a new FetchList
   * @param multiplier Size multiplier (default: cache-aligned)
   *                   Each block holds 8 * multiplier elements
   */
  FetchList(size_t multiplier = cache_aligned_multplier<T>())
      : blocks_(nullptr)
      , versions_(nullptr)
      , block_count_(0)
      , block_capacity_(0)
      , size_(0)
      , multiplier_(multiplier)
      , elements_per_block_(8 * multiplier)
  {
  }

  ~FetchList()
  {
    // Destroy all constructed elements
    for (size_t i = 0; i < block_count_; ++i) {
      for (size_t j = 0; j < elements_per_block_; ++j) {
        if (versions_[i][j].state() != VersionedSlot::FREE) {
          blocks_[i][j].~T();
        }
      }
      // Free raw memory (not allocated with new[])
      ::operator delete(blocks_[i]);
      delete[] versions_[i];
    }
    if (blocks_) {
      delete[] blocks_;
    }
    if (versions_) {
      delete[] versions_;
    }
  }

  // Non-copyable, non-movable (contains stable pointers)
  FetchList(const FetchList &)            = delete;
  FetchList &operator=(const FetchList &) = delete;
  FetchList(FetchList &&)                 = delete;
  FetchList &operator=(FetchList &&)      = delete;

  /**
   * @brief Get current number of occupied elements
   */
  size_t size() const { return size_; }

  /**
   * @brief Get total capacity across all blocks
   */
  size_t capacity() const { return block_count_ * elements_per_block_; }

  /**
   * @brief Check if container is empty
   */
  bool empty() const { return size_ == 0; }

  /**
   * @brief Allocate and construct element in-place
   * @tparam Args Constructor argument types
   * @param args Constructor arguments
   * @return Handle to the allocated element
   */
  template <typename... Args> Handle emplace(Args &&...args)
  {
    int slot = find_free_slot();
    if (slot == -1) {
      grow();
      slot = find_free_slot();
      if (slot == -1) {
        // Should never happen unless out of memory
        return Handle{};
      }
    }

    size_t index       = static_cast<size_t>(slot);
    size_t block_idx   = index / elements_per_block_;
    size_t element_idx = index % elements_per_block_;

    // Try to allocate the versioned slot
    auto result = versions_[block_idx][element_idx].tryAllocate();
    if (!result.success) {
      // Slot is at end-of-life, could try next available
      return Handle{};
    }

    // Construct element in-place using placement new
    new (&blocks_[block_idx][element_idx]) T(std::forward<Args>(args)...);

    ++size_;

    return Handle{index, result.version};
  }

  /**
   * @brief Erase element at handle (safe)
   * @param handle Handle to element
   * @return true if erased, false if handle is invalid
   */
  bool erase(Handle handle)
  {
    if (!handle.isValid() || handle.index >= capacity()) {
      return false;
    }

    size_t block_idx   = handle.index / elements_per_block_;
    size_t element_idx = handle.index % elements_per_block_;

    // Validate version
    if (!versions_[block_idx][element_idx].isValid(handle.version)) {
      return false;
    }

    // Call destructor
    blocks_[block_idx][element_idx].~T();

    // Free the slot (increments version)
    if (versions_[block_idx][element_idx].free(handle.version)) {
      --size_;
      return true;
    }

    return false;
  }

  /**
   * @brief Get pointer to element (safe, validates version)
   * @param handle Handle to element
   * @return Pointer to element, or nullptr if invalid
   */
  T *get(Handle handle)
  {
    if (!handle.isValid() || handle.index >= capacity()) {
      return nullptr;
    }

    size_t block_idx   = handle.index / elements_per_block_;
    size_t element_idx = handle.index % elements_per_block_;

    if (versions_[block_idx][element_idx].isValid(handle.version)) {
      return &blocks_[block_idx][element_idx];
    }

    return nullptr;
  }

  /**
   * @brief Get const pointer to element (safe)
   */
  const T *get(Handle handle) const
  {
    return const_cast<FetchList *>(this)->get(handle);
  }

  /**
   * @brief Access element by handle (throws if invalid)
   * @param handle Handle to element
   * @return Reference to element
   * @throws std::out_of_range if handle is invalid
   */
  T &at(Handle handle)
  {
    T *ptr = get(handle);
    if (!ptr) {
      throw std::out_of_range("FetchList::at() - invalid handle");
    }
    return *ptr;
  }

  /**
   * @brief Access const element by handle (throws if invalid)
   */
  const T &at(Handle handle) const
  {
    const T *ptr = get(handle);
    if (!ptr) {
      throw std::out_of_range("FetchList::at() - invalid handle");
    }
    return *ptr;
  }

  /**
   * @brief Check if a handle points to a valid element
   * @param handle Handle to check
   * @return true if handle is valid and element exists
   */
  bool contains(Handle handle) const { return isValid(handle); }

  /**
   * @brief Check if handle is valid
   */
  bool isValid(Handle handle) const
  {
    if (!handle.isValid() || handle.index >= capacity()) {
      return false;
    }

    size_t block_idx   = handle.index / elements_per_block_;
    size_t element_idx = handle.index % elements_per_block_;

    return versions_[block_idx][element_idx].isValid(handle.version);
  }

  /**
   * @brief Iterator for range-based for loops (skips FREE slots)
   */
  class iterator {
    FetchList *list_;
    size_t     index_;

    void advance_to_valid()
    {
      while (index_ < list_->capacity()) {
        size_t block_idx   = index_ / list_->elements_per_block_;
        size_t element_idx = index_ % list_->elements_per_block_;

        if (block_idx < list_->block_count_ &&
            list_->versions_[block_idx][element_idx].state() !=
                VersionedSlot::FREE) {
          break;
        }
        ++index_;
      }
    }

     public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = T *;
    using reference         = T &;

    iterator(FetchList *list, size_t index)
        : list_(list)
        , index_(index)
    {
      advance_to_valid();
    }

    reference operator*() const
    {
      size_t block_idx   = index_ / list_->elements_per_block_;
      size_t element_idx = index_ % list_->elements_per_block_;
      return list_->blocks_[block_idx][element_idx];
    }

    pointer operator->() const
    {
      size_t block_idx   = index_ / list_->elements_per_block_;
      size_t element_idx = index_ % list_->elements_per_block_;
      return &list_->blocks_[block_idx][element_idx];
    }

    iterator &operator++()
    {
      ++index_;
      advance_to_valid();
      return *this;
    }

    iterator operator++(int)
    {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const iterator &other) const
    {
      return list_ == other.list_ && index_ == other.index_;
    }

    bool operator!=(const iterator &other) const { return !(*this == other); }
  };

  /**
   * @brief Const iterator for range-based for loops
   */
  class const_iterator {
    const FetchList *list_;
    size_t           index_;

    void advance_to_valid()
    {
      while (index_ < list_->capacity()) {
        size_t block_idx   = index_ / list_->elements_per_block_;
        size_t element_idx = index_ % list_->elements_per_block_;

        if (block_idx < list_->block_count_ &&
            list_->versions_[block_idx][element_idx].state() !=
                VersionedSlot::FREE) {
          break;
        }
        ++index_;
      }
    }

     public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = const T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = const T *;
    using reference         = const T &;

    const_iterator(const FetchList *list, size_t index)
        : list_(list)
        , index_(index)
    {
      advance_to_valid();
    }

    reference operator*() const
    {
      size_t block_idx   = index_ / list_->elements_per_block_;
      size_t element_idx = index_ % list_->elements_per_block_;
      return list_->blocks_[block_idx][element_idx];
    }

    pointer operator->() const
    {
      size_t block_idx   = index_ / list_->elements_per_block_;
      size_t element_idx = index_ % list_->elements_per_block_;
      return &list_->blocks_[block_idx][element_idx];
    }

    const_iterator &operator++()
    {
      ++index_;
      advance_to_valid();
      return *this;
    }

    const_iterator operator++(int)
    {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const const_iterator &other) const
    {
      return list_ == other.list_ && index_ == other.index_;
    }

    bool operator!=(const const_iterator &other) const
    {
      return !(*this == other);
    }
  };

  /**
   * @brief Get handle to the nth valid element (by logical index)
   * @param index Logical index (0 = first valid element, 1 = second, etc.)
   * @return Handle to element, or invalid handle if index out of range
   */
  Handle getHandleByIndex(size_t index)
  {
    size_t count = 0;
    for (size_t i = 0; i < capacity(); ++i) {
      size_t block_idx   = i / elements_per_block_;
      size_t element_idx = i % elements_per_block_;

      if (block_idx < block_count_ &&
          versions_[block_idx][element_idx].state() != VersionedSlot::FREE) {
        if (count == index) {
          auto version = versions_[block_idx][element_idx].version();
          return Handle{i, version};
        }
        ++count;
      }
    }
    return Handle{};
  }

  /**
   * @brief Get iterator to first valid element
   */
  iterator begin() { return iterator(this, 0); }

  /**
   * @brief Get iterator past last element
   */
  iterator end() { return iterator(this, capacity()); }

  /**
   * @brief Get const iterator to first valid element
   */
  const_iterator begin() const { return const_iterator(this, 0); }

  /**
   * @brief Get const iterator past last element
   */
  const_iterator end() const { return const_iterator(this, capacity()); }

  /**
   * @brief Get const iterator to first valid element
   */
  const_iterator cbegin() const { return const_iterator(this, 0); }

  /**
   * @brief Get const iterator past last element
   */
  const_iterator cend() const { return const_iterator(this, capacity()); }
};
