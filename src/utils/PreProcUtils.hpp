
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define DEBUG_VK_ABOX
#ifdef DEBUG_VK_ABOX
  #define VK_ABOX_VALIDATION_LAYERS
  #define VK_ABOX_PROFILING
  #define ABOX_RESSOURCES_DEBUG
#endif

#define AxREAL double // Might map to float on other target
#define AxUINT uint_fast32_t
#define AxINT int_fast32_t

//--- Class PreProc Utils ---

/** @brief removes the copy constructor */
#define DELETE_COPY(X)                                                         \
  X(const X &)            = delete;                                            \
  X &operator=(const X &) = delete;

/** @brief removes the move constructor */
#define DELETE_MOVE(X)                                                         \
  X(X &&)            = delete;                                                 \
  X &operator=(X &&) = delete;

/** @brief removes the copy constructor */
#define DEFAULT_COPY(X)                                                        \
  X(const X &) noexcept            = default;                                  \
  X &operator=(const X &) noexcept = default;

/** @brief removes the move constructor */
#define DEFAULT_MOVE(X)                                                        \
  X(X &&) noexcept            = default;                                       \
  X &operator=(X &&) noexcept = default;

/** @brief ostream operator automaping type */
#define OSTREAM_OP(X) std::ostream &operator<<(std::ostream &os, X)

//--- custom print functions ----
#define ABOX_FILE_DEBUG
// Define DEBUG_PRINT only if DEBUG is enabled
#ifdef ABOX_FILE_DEBUG
  #define FILE_DEBUG_PRINT(fmt, ...)                                           \
    printf("FILE_DEBUG: " fmt "\n", ##__VA_ARGS__)
#else
  #define FILE_DEBUG_PRINT(fmt, ...)
// do nothing undefined case
#endif

#ifdef ABOX_RESSOURCES_DEBUG
  #define RESS_DEBUG_PRINT(fmt, ...)                                           \
    printf("RESS_DEBUG: " fmt "\n", ##__VA_ARGS__)
#else
  #define FILE_DEBUG_PRINT(fmt, ...)
// do nothing undefined case
#endif
