#pragma once

#include <cstdint>
#include <cstring>
#include <memory>

/**
 * @class FetchList
 * @brief Colony-style allocator with separate bitmap and element storage
 *
 * Grows dynamically by allocating blocks of 8 * multiplier elements.
 * Bitmaps are stored separately from element blocks for better cache efficiency.
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
template <typename T, typename Allocator = std::allocator<T>> class FetchList {
   private:
  uint8_t *bitmaps_;          ///< All bitmaps (multiplier bytes per block)
  T      **blocks_;           ///< Array of element blocks
  size_t   block_count_;      ///< Number of allocated blocks
  size_t   block_capacity_;   ///< Capacity for blocks array
  size_t   size_;             ///< Current number of occupied elements
  size_t   multiplier_;       ///< Size multiplier per block
  size_t   elements_per_block_; ///< 8 * multiplier
  size_t   bitmap_bytes_per_block_; ///< multiplier bytes

   protected:
  /**
   * @brief Check if element at index is occupied
   * @param index Element index
   * @return true if occupied, false if free
   */
  bool is_occupied(size_t index) const
  {
    size_t byte_idx = index / 8;
    size_t bit_idx  = index % 8;
    return (bitmaps_[byte_idx] & (1 << bit_idx)) != 0;
  }

  /**
   * @brief Mark element at index as occupied
   * @param index Element index
   */
  void set_occupied(size_t index)
  {
    size_t byte_idx = index / 8;
    size_t bit_idx  = index % 8;
    if (!(bitmaps_[byte_idx] & (1 << bit_idx))) {
      bitmaps_[byte_idx] |= (1 << bit_idx);
      ++size_;
    }
  }

  /**
   * @brief Mark element at index as free
   * @param index Element index
   */
  void set_free(size_t index)
  {
    size_t byte_idx = index / 8;
    size_t bit_idx  = index % 8;
    if (bitmaps_[byte_idx] & (1 << bit_idx)) {
      bitmaps_[byte_idx] &= ~(1 << bit_idx);
      --size_;
    }
  }

  /**
   * @brief Find first free slot
   * @return Index of free slot, or -1 if full
   */
  int find_free_slot() const
  {
    size_t total_elements = block_count_ * elements_per_block_;
    for (size_t i = 0; i < total_elements; ++i) {
      if (!is_occupied(i)) {
        return static_cast<int>(i);
      }
    }
    return -1;
  }

   public:
  /**
   * @brief Construct a new FetchList
   * @param multiplier Size multiplier (default: 1)
   *                   Each block holds 8 * multiplier elements
   *                   Each block's bitmap is multiplier bytes
   */
  FetchList(size_t multiplier = 1)
      : bitmaps_(nullptr)
      , blocks_(nullptr)
      , block_count_(0)
      , block_capacity_(0)
      , size_(0)
      , multiplier_(multiplier)
      , elements_per_block_(8 * multiplier)
      , bitmap_bytes_per_block_(multiplier)
  {
  }

  ~FetchList()
  {
    for (size_t i = 0; i < block_count_; ++i) {
      delete[] blocks_[i];
    }
    if (blocks_) {
      delete[] blocks_;
    }
    if (bitmaps_) {
      delete[] bitmaps_;
    }
  }

  size_t size() const { return size_; }

  /**
   * @brief Access element at global index
   * @param index Global element index across all blocks
   * @return Reference to element
   */
  T &operator[](size_t index)
  {
    size_t block_idx   = index / elements_per_block_;
    size_t element_idx = index % elements_per_block_;
    return blocks_[block_idx][element_idx];
  }

  const T &operator[](size_t index) const
  {
    size_t block_idx   = index / elements_per_block_;
    size_t element_idx = index % elements_per_block_;
    return blocks_[block_idx][element_idx];
  }
};
