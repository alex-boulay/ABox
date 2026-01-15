#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <typename T> class BloomFilter {
   private:
  std::vector<bool> bits_;
  size_t            num_hashes_;
  size_t            size_;

  size_t hash(const T &item, size_t seed) const
  {
    std::hash<T> hasher;
    size_t       hash_value = hasher(item);
    // Mix the seed into the hash
    hash_value ^= seed + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
    return hash_value % size_;
  }

   public:
  BloomFilter(
      size_t expected_elements   = 1000,
      double false_positive_rate = 0.01
  )
      : num_hashes_(0)
      , size_(0)
  {
    // Calculate optimal size and number of hash functions
    // size = -n * ln(p) / (ln(2)^2)
    // num_hashes = size / n * ln(2)
    if (expected_elements > 0) {
      size_ = static_cast<size_t>(
          -static_cast<double>(expected_elements) *
          std::log(false_positive_rate) / (std::log(2) * std::log(2))
      );
      num_hashes_ = static_cast<size_t>(
          static_cast<double>(size_) / expected_elements * std::log(2)
      );
      if (num_hashes_ == 0) {
        num_hashes_ = 1;
      }
      bits_.resize(size_, false);
    }
  }

  void add(const T &item)
  {
    for (size_t i = 0; i < num_hashes_; ++i) {
      bits_[hash(item, i)] = true;
    }
  }

  bool contains(const T &item) const
  {
    for (size_t i = 0; i < num_hashes_; ++i) {
      if (!bits_[hash(item, i)]) {
        return false;
      }
    }
    return true;
  }

  void clear() { std::fill(bits_.begin(), bits_.end(), false); }

  size_t size() const { return size_; }
  size_t num_hashes() const { return num_hashes_; }
};

// Status values for FetchList entries (2 bits max = 4 possible states)
enum class EntryStatus : uint8_t {
  Free     = 0, // Not allocated/available
  Active   = 1, // Currently in use
  Pending  = 2, // Pending operation
  Obsolete = 3  // Marked for deletion
};

// Metadata for tracking FetchList entry state
struct EntryMetadata {
  uint32_t    version; // Version counter for lifetime tracking (starts at 0)
  EntryStatus status;  // Current status (2 bits)

  EntryMetadata()
      : version(0)
      , status(EntryStatus::Free)
  {
  }

  void increment_version() { ++version; }

  bool is_free() const { return status == EntryStatus::Free; }
  bool is_active() const { return status == EntryStatus::Active; }
  bool is_pending() const { return status == EntryStatus::Pending; }
  bool is_obsolete() const { return status == EntryStatus::Obsolete; }
};

// Handle type - just holds an index and version for validation
template <typename T>
class FetchListHandle {
   private:
  size_t   index_;   // Index into the FetchList
  uint32_t version_; // Expected version for validation

   public:
  FetchListHandle()
      : index_(0)
      , version_(0)
  {
  }

  FetchListHandle(size_t index, uint32_t version)
      : index_(index)
      , version_(version)
  {
  }

  size_t   index() const { return index_; }
  uint32_t version() const { return version_; }

  bool operator==(const FetchListHandle &other) const
  {
    return index_ == other.index_ && version_ == other.version_;
  }

  bool operator!=(const FetchListHandle &other) const
  {
    return !(*this == other);
  }
};

// Managed FetchList with metadata tracking
template <typename T, typename Allocator = std::allocator<T>>
class ManagedFetchList {
   private:
  FetchList<T, Allocator> data_;
  std::vector<EntryMetadata> metadata_;

   public:
  using Handle = FetchListHandle<T>;

  ManagedFetchList(size_t reserve_seed = 5)
      : data_(reserve_seed)
  {
    metadata_.reserve(reserve_seed);
  }

  // Create new entry and return handle
  Handle emplace_back(const T &value)
  {
    size_t index = data_.size();
    data_.push_back(value);

    // Ensure metadata vector is large enough
    if (metadata_.size() <= index) {
      metadata_.resize(index + 1);
    }

    metadata_[index].status = EntryStatus::Active;
    metadata_[index].increment_version();

    return Handle(index, metadata_[index].version);
  }

  template <typename... Args> Handle emplace_back(Args &&...args)
  {
    size_t index = data_.size();
    data_.emplace_back(std::forward<Args>(args)...);

    if (metadata_.size() <= index) {
      metadata_.resize(index + 1);
    }

    metadata_[index].status = EntryStatus::Active;
    metadata_[index].increment_version();

    return Handle(index, metadata_[index].version);
  }

  // Access resource via handle (with version validation)
  T *get(const Handle &handle)
  {
    if (handle.index() >= data_.size()) {
      return nullptr;
    }
    if (metadata_[handle.index()].version != handle.version()) {
      return nullptr; // Stale handle
    }
    if (metadata_[handle.index()].is_free()) {
      return nullptr;
    }
    return &data_[handle.index()];
  }

  const T *get(const Handle &handle) const
  {
    if (handle.index() >= data_.size()) {
      return nullptr;
    }
    if (metadata_[handle.index()].version != handle.version()) {
      return nullptr;
    }
    if (metadata_[handle.index()].is_free()) {
      return nullptr;
    }
    return &data_[handle.index()];
  }

  // Direct access (no validation)
  T &operator[](size_t index) { return data_[index]; }
  const T &operator[](size_t index) const { return data_[index]; }

  // Metadata access
  EntryMetadata &get_metadata(size_t index) { return metadata_[index]; }
  const EntryMetadata &get_metadata(size_t index) const
  {
    return metadata_[index];
  }

  EntryMetadata *get_metadata(const Handle &handle)
  {
    if (handle.index() >= metadata_.size()) {
      return nullptr;
    }
    return &metadata_[handle.index()];
  }

  // Status management
  void set_status(const Handle &handle, EntryStatus status)
  {
    if (handle.index() < metadata_.size()) {
      metadata_[handle.index()].status = status;
    }
  }

  void set_status(size_t index, EntryStatus status)
  {
    if (index < metadata_.size()) {
      metadata_[index].status = status;
    }
  }

  // Mark entry as free (can be reused with new version)
  void free(const Handle &handle)
  {
    if (handle.index() < metadata_.size()) {
      metadata_[handle.index()].status = EntryStatus::Free;
      metadata_[handle.index()].increment_version();
    }
  }

  void free(size_t index)
  {
    if (index < metadata_.size()) {
      metadata_[index].status = EntryStatus::Free;
      metadata_[index].increment_version();
    }
  }

  // Container operations
  size_t size() const { return data_.size(); }
  bool   empty() const { return data_.empty(); }

  FetchList<T, Allocator>       &get_data() { return data_; }
  const FetchList<T, Allocator> &get_data() const { return data_; }
};

// Container mapping memory wrapped sections
template <typename T, typename Allocator = std::allocator<T>> class FetchList {
   public:
  // Standard container type aliases
  using value_type      = T;
  using allocator_type  = Allocator;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference       = value_type &;
  using const_reference = const value_type &;
  using pointer         = typename std::allocator_traits<Allocator>::pointer;
  using const_pointer =
      typename std::allocator_traits<Allocator>::const_pointer;
  using iterator               = pointer;
  using const_iterator         = const_pointer;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   private:
  pointer                         data_;
  size_type                       size_;
  size_type                       capacity_;
  [[no_unique_address]] Allocator allocator_;

  using AllocTraits = std::allocator_traits<Allocator>;

   public:
  // Constructors
  FetchList(size_type reserve_seed = 5) noexcept(noexcept(Allocator()))
      : data_(nullptr)
      , size_(0)
      , capacity_(0)
      , allocator_()
  {
    if (reserve_seed > 0) {
      reserve(reserve_seed);
    }
  }

  explicit FetchList(const Allocator &alloc, size_type reserve_seed = 5) noexcept
      : data_(nullptr)
      , size_(0)
      , capacity_(0)
      , allocator_(alloc)
  {
    if (reserve_seed > 0) {
      reserve(reserve_seed);
    }
  }

  explicit FetchList(
      size_type        count,
      const T         &value = T(),
      const Allocator &alloc = Allocator()
  )
      : data_(nullptr)
      , size_(0)
      , capacity_(0)
      , allocator_(alloc)
  {
    if (count > 0) {
      reserve(count);
      for (size_type i = 0; i < count; ++i) {
        push_back(value);
      }
    }
  }

  template <
      typename InputIt,
      typename = std::enable_if_t<std::is_base_of_v<
          std::input_iterator_tag,
          typename std::iterator_traits<InputIt>::iterator_category>>>
  FetchList(InputIt first, InputIt last, const Allocator &alloc = Allocator())
      : data_(nullptr)
      , size_(0)
      , capacity_(0)
      , allocator_(alloc)
  {
    for (auto it = first; it != last; ++it) {
      push_back(*it);
    }
  }

  // Copy constructor
  FetchList(const FetchList &other)
      : data_(nullptr)
      , size_(0)
      , capacity_(0)
      , allocator_(
            AllocTraits::select_on_container_copy_construction(other.allocator_)
        )
  {
    reserve(other.size_);
    for (size_type i = 0; i < other.size_; ++i) {
      push_back(other.data_[i]);
    }
  }

  // Copy constructor with allocator
  FetchList(const FetchList &other, const Allocator &alloc)
      : data_(nullptr)
      , size_(0)
      , capacity_(0)
      , allocator_(alloc)
  {
    reserve(other.size_);
    for (size_type i = 0; i < other.size_; ++i) {
      push_back(other.data_[i]);
    }
  }

  // Move constructor
  FetchList(FetchList &&other) noexcept
      : data_(other.data_)
      , size_(other.size_)
      , capacity_(other.capacity_)
      , allocator_(std::move(other.allocator_))
  {
    other.data_     = nullptr;
    other.size_     = 0;
    other.capacity_ = 0;
  }

  // Move constructor with allocator
  FetchList(FetchList &&other, const Allocator &alloc)
      : data_(nullptr)
      , size_(0)
      , capacity_(0)
      , allocator_(alloc)
  {
    if (allocator_ == other.allocator_) {
      data_           = other.data_;
      size_           = other.size_;
      capacity_       = other.capacity_;
      other.data_     = nullptr;
      other.size_     = 0;
      other.capacity_ = 0;
    }
    else {
      reserve(other.size_);
      for (size_type i = 0; i < other.size_; ++i) {
        push_back(std::move(other.data_[i]));
      }
    }
  }

  // Initializer list constructor
  FetchList(std::initializer_list<T> init, const Allocator &alloc = Allocator())
      : FetchList(init.begin(), init.end(), alloc)
  {
  }

  // Destructor
  ~FetchList()
  {
    clear();
    if (data_) {
      AllocTraits::deallocate(allocator_, data_, capacity_);
    }
  }

  // Copy assignment
  FetchList &operator=(const FetchList &other)
  {
    if (this != &other) {
      if constexpr (AllocTraits::propagate_on_container_copy_assignment::
                        value) {
        if (allocator_ != other.allocator_) {
          clear();
          AllocTraits::deallocate(allocator_, data_, capacity_);
          data_     = nullptr;
          capacity_ = 0;
        }
        allocator_ = other.allocator_;
      }
      clear();
      reserve(other.size_);
      for (size_type i = 0; i < other.size_; ++i) {
        push_back(other.data_[i]);
      }
    }
    return *this;
  }

  // Move assignment
  FetchList &operator=(FetchList &&other) noexcept(
      AllocTraits::propagate_on_container_move_assignment::value ||
      AllocTraits::is_always_equal::value
  )
  {
    if (this != &other) {
      clear();
      if constexpr (AllocTraits::propagate_on_container_move_assignment::
                        value) {
        AllocTraits::deallocate(allocator_, data_, capacity_);
        data_           = other.data_;
        size_           = other.size_;
        capacity_       = other.capacity_;
        allocator_      = std::move(other.allocator_);
        other.data_     = nullptr;
        other.size_     = 0;
        other.capacity_ = 0;
      }
      else if (allocator_ == other.allocator_) {
        AllocTraits::deallocate(allocator_, data_, capacity_);
        data_           = other.data_;
        size_           = other.size_;
        capacity_       = other.capacity_;
        other.data_     = nullptr;
        other.size_     = 0;
        other.capacity_ = 0;
      }
      else {
        reserve(other.size_);
        for (size_type i = 0; i < other.size_; ++i) {
          push_back(std::move(other.data_[i]));
        }
      }
    }
    return *this;
  }

  // Initializer list assignment
  FetchList &operator=(std::initializer_list<T> ilist)
  {
    clear();
    reserve(ilist.size());
    for (const auto &item : ilist) {
      push_back(item);
    }
    return *this;
  }

  // Allocator access
  allocator_type get_allocator() const noexcept { return allocator_; }

  // Element access
  reference at(size_type pos)
  {
    if (pos >= size_) {
      throw std::out_of_range("FetchList::at: index out of range");
    }
    return data_[pos];
  }

  const_reference at(size_type pos) const
  {
    if (pos >= size_) {
      throw std::out_of_range("FetchList::at: index out of range");
    }
    return data_[pos];
  }

  reference operator[](size_type pos) noexcept { return data_[pos]; }

  const_reference operator[](size_type pos) const noexcept
  {
    return data_[pos];
  }

  reference front() noexcept { return data_[0]; }

  const_reference front() const noexcept { return data_[0]; }

  reference back() noexcept { return data_[size_ - 1]; }

  const_reference back() const noexcept { return data_[size_ - 1]; }

  pointer data() noexcept { return data_; }

  const_pointer data() const noexcept { return data_; }

  // Iterators
  iterator begin() noexcept { return data_; }

  const_iterator begin() const noexcept { return data_; }

  const_iterator cbegin() const noexcept { return data_; }

  iterator end() noexcept { return data_ + size_; }

  const_iterator end() const noexcept { return data_ + size_; }

  const_iterator cend() const noexcept { return data_ + size_; }

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

  const_reverse_iterator rbegin() const noexcept
  {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator crbegin() const noexcept
  {
    return const_reverse_iterator(end());
  }

  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

  const_reverse_iterator rend() const noexcept
  {
    return const_reverse_iterator(begin());
  }

  const_reverse_iterator crend() const noexcept
  {
    return const_reverse_iterator(begin());
  }

  // Capacity
  [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

  size_type size() const noexcept { return size_; }

  size_type max_size() const noexcept
  {
    return AllocTraits::max_size(allocator_);
  }

  void reserve(size_type new_cap)
  {
    if (new_cap > capacity_) {
      pointer new_data = AllocTraits::allocate(allocator_, new_cap);

      // Move/copy elements to new storage
      for (size_type i = 0; i < size_; ++i) {
        AllocTraits::construct(
            allocator_,
            new_data + i,
            std::move_if_noexcept(data_[i])
        );
      }

      // Destroy old elements
      for (size_type i = 0; i < size_; ++i) {
        AllocTraits::destroy(allocator_, data_ + i);
      }

      // Deallocate old storage
      if (data_) {
        AllocTraits::deallocate(allocator_, data_, capacity_);
      }

      data_     = new_data;
      capacity_ = new_cap;
    }
  }

  size_type capacity() const noexcept { return capacity_; }

  void shrink_to_fit()
  {
    if (capacity_ > size_) {
      if (size_ == 0) {
        AllocTraits::deallocate(allocator_, data_, capacity_);
        data_     = nullptr;
        capacity_ = 0;
      }
      else {
        pointer new_data = AllocTraits::allocate(allocator_, size_);

        for (size_type i = 0; i < size_; ++i) {
          AllocTraits::construct(
              allocator_,
              new_data + i,
              std::move_if_noexcept(data_[i])
          );
        }

        for (size_type i = 0; i < size_; ++i) {
          AllocTraits::destroy(allocator_, data_ + i);
        }

        AllocTraits::deallocate(allocator_, data_, capacity_);
        data_     = new_data;
        capacity_ = size_;
      }
    }
  }

  // Modifiers
  void clear() noexcept
  {
    for (size_type i = 0; i < size_; ++i) {
      AllocTraits::destroy(allocator_, data_ + i);
    }
    size_ = 0;
  }

  iterator insert(const_iterator pos, const T &value)
  {
    return insert(pos, 1, value);
  }

  iterator insert(const_iterator pos, T &&value)
  {
    size_type index = pos - data_;
    if (size_ == capacity_) {
      reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }

    // Shift elements
    for (size_type i = size_; i > index; --i) {
      if (i == size_) {
        AllocTraits::construct(allocator_, data_ + i, std::move(data_[i - 1]));
      }
      else {
        data_[i] = std::move(data_[i - 1]);
      }
    }

    if (index < size_) {
      data_[index] = std::move(value);
    }
    else {
      AllocTraits::construct(allocator_, data_ + index, std::move(value));
    }
    ++size_;

    return data_ + index;
  }

  iterator insert(const_iterator pos, size_type count, const T &value)
  {
    size_type index = pos - data_;
    if (count == 0) {
      return data_ + index;
    }

    if (size_ + count > capacity_) {
      reserve(std::max(capacity_ * 2, size_ + count));
    }

    // Shift elements
    for (size_type i = size_ + count - 1; i >= index + count; --i) {
      if (i >= size_) {
        AllocTraits::construct(
            allocator_,
            data_ + i,
            std::move(data_[i - count])
        );
      }
      else {
        data_[i] = std::move(data_[i - count]);
      }
    }

    // Insert new elements
    for (size_type i = 0; i < count; ++i) {
      if (index + i < size_) {
        data_[index + i] = value;
      }
      else {
        AllocTraits::construct(allocator_, data_ + index + i, value);
      }
    }

    size_ += count;
    return data_ + index;
  }

  template <typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last)
  {
    size_type index = pos - data_;
    size_type count = std::distance(first, last);

    if (count == 0) {
      return data_ + index;
    }

    if (size_ + count > capacity_) {
      reserve(std::max(capacity_ * 2, size_ + count));
    }

    // Shift elements
    for (size_type i = size_ + count - 1; i >= index + count; --i) {
      if (i >= size_) {
        AllocTraits::construct(
            allocator_,
            data_ + i,
            std::move(data_[i - count])
        );
      }
      else {
        data_[i] = std::move(data_[i - count]);
      }
    }

    // Insert new elements
    size_type i = index;
    for (auto it = first; it != last; ++it, ++i) {
      if (i < size_) {
        data_[i] = *it;
      }
      else {
        AllocTraits::construct(allocator_, data_ + i, *it);
      }
    }

    size_ += count;
    return data_ + index;
  }

  iterator insert(const_iterator pos, std::initializer_list<T> ilist)
  {
    return insert(pos, ilist.begin(), ilist.end());
  }

  template <typename... Args>
  iterator emplace(const_iterator pos, Args &&...args)
  {
    size_type index = pos - data_;
    if (size_ == capacity_) {
      reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }

    // Shift elements
    for (size_type i = size_; i > index; --i) {
      if (i == size_) {
        AllocTraits::construct(allocator_, data_ + i, std::move(data_[i - 1]));
      }
      else {
        data_[i] = std::move(data_[i - 1]);
      }
    }

    if (index < size_) {
      AllocTraits::destroy(allocator_, data_ + index);
    }
    AllocTraits::construct(
        allocator_,
        data_ + index,
        std::forward<Args>(args)...
    );
    ++size_;

    return data_ + index;
  }

  iterator erase(const_iterator pos) { return erase(pos, pos + 1); }

  iterator erase(const_iterator first, const_iterator last)
  {
    size_type start_index = first - data_;
    size_type end_index   = last - data_;
    size_type count       = end_index - start_index;

    if (count == 0) {
      return data_ + start_index;
    }

    // Destroy erased elements
    for (size_type i = start_index; i < end_index; ++i) {
      AllocTraits::destroy(allocator_, data_ + i);
    }

    // Shift remaining elements
    for (size_type i = end_index; i < size_; ++i) {
      AllocTraits::construct(
          allocator_,
          data_ + i - count,
          std::move(data_[i])
      );
      AllocTraits::destroy(allocator_, data_ + i);
    }

    size_ -= count;
    return data_ + start_index;
  }

  void push_back(const T &value)
  {
    if (size_ == capacity_) {
      reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }
    AllocTraits::construct(allocator_, data_ + size_, value);
    ++size_;
  }

  void push_back(T &&value)
  {
    if (size_ == capacity_) {
      reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }
    AllocTraits::construct(allocator_, data_ + size_, std::move(value));
    ++size_;
  }

  template <typename... Args> reference emplace_back(Args &&...args)
  {
    if (size_ == capacity_) {
      reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }
    AllocTraits::construct(
        allocator_,
        data_ + size_,
        std::forward<Args>(args)...
    );
    ++size_;
    return data_[size_ - 1];
  }

  void pop_back()
  {
    if (size_ > 0) {
      --size_;
      AllocTraits::destroy(allocator_, data_ + size_);
    }
  }

  void resize(size_type count) { resize(count, T()); }

  void resize(size_type count, const value_type &value)
  {
    if (count < size_) {
      for (size_type i = count; i < size_; ++i) {
        AllocTraits::destroy(allocator_, data_ + i);
      }
      size_ = count;
    }
    else if (count > size_) {
      reserve(count);
      for (size_type i = size_; i < count; ++i) {
        AllocTraits::construct(allocator_, data_ + i, value);
      }
      size_ = count;
    }
  }

  void swap(FetchList &other) noexcept(
      AllocTraits::propagate_on_container_swap::value ||
      AllocTraits::is_always_equal::value
  )
  {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
    if constexpr (AllocTraits::propagate_on_container_swap::value) {
      std::swap(allocator_, other.allocator_);
    }
  }
};

// Non-member functions
template <typename T, typename Alloc>
bool operator==(const FetchList<T, Alloc> &lhs, const FetchList<T, Alloc> &rhs)
{
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename Alloc>
bool operator!=(const FetchList<T, Alloc> &lhs, const FetchList<T, Alloc> &rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename Alloc>
bool operator<(const FetchList<T, Alloc> &lhs, const FetchList<T, Alloc> &rhs)
{
  return std::lexicographical_compare(
      lhs.begin(),
      lhs.end(),
      rhs.begin(),
      rhs.end()
  );
}

template <typename T, typename Alloc>
bool operator<=(const FetchList<T, Alloc> &lhs, const FetchList<T, Alloc> &rhs)
{
  return !(rhs < lhs);
}

template <typename T, typename Alloc>
bool operator>(const FetchList<T, Alloc> &lhs, const FetchList<T, Alloc> &rhs)
{
  return rhs < lhs;
}

template <typename T, typename Alloc>
bool operator>=(const FetchList<T, Alloc> &lhs, const FetchList<T, Alloc> &rhs)
{
  return !(lhs < rhs);
}

template <typename T, typename Alloc>
void swap(FetchList<T, Alloc> &lhs, FetchList<T, Alloc> &rhs) noexcept(
    noexcept(lhs.swap(rhs))
)
{
  lhs.swap(rhs);
}
