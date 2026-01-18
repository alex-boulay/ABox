#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>

// Platform-specific headers
#if defined(_WIN32) || defined(_WIN64)
    #include <malloc.h>
#elif defined(__APPLE__)
    #include <malloc/malloc.h>
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__unix__)
    #include <malloc.h>
#endif

// Platform-specific allocation size query
size_t get_actual_allocation_size(void* ptr, size_t requested) {
#if defined(_WIN32) || defined(_WIN64)
    return _msize(ptr);
#elif defined(__APPLE__)
    return malloc_size(ptr);
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__unix__)
    return malloc_usable_size(ptr);
#else
    return requested; // Fallback: assume exact allocation
#endif
}

struct SizeClass {
    size_t actual_size;
    size_t first_request;  // First request that yields this actual size
    size_t last_request;   // Last request that yields this actual size
    size_t count;          // How many different request sizes map to this
};

int main() {
    std::cout << "=== Allocator Pattern Solver ===\n\n";

    // Platform detection
    std::cout << "Platform: ";
#if defined(_WIN32) || defined(_WIN64)
    std::cout << "Windows";
#elif defined(__APPLE__)
    std::cout << "macOS/iOS";
#elif defined(__linux__)
    std::cout << "Linux";
#elif defined(__FreeBSD__)
    std::cout << "FreeBSD";
#else
    std::cout << "Unknown";
#endif
    std::cout << "\n\n";

    // Phase 1: Find pattern by scanning early allocations
    std::cout << "[Phase 1] Detecting allocation pattern...\n\n";

    std::vector<size_t> actual_sizes;
    std::vector<size_t> steps;

    // Scan first 512 bytes to find pattern
    size_t prev_actual = 0;
    for (size_t req = 1; req <= 512; ++req) {
        void* ptr = std::malloc(req);
        if (!ptr) continue;
        size_t actual = get_actual_allocation_size(ptr, req);
        std::free(ptr);

        if (actual != prev_actual) {
            actual_sizes.push_back(actual);
            if (prev_actual > 0) {
                steps.push_back(actual - prev_actual);
            }
            prev_actual = actual;
        }
    }

    // Detect pattern: n + a * x
    size_t n = actual_sizes[0];  // Minimum (base)

    // Find most common step size (granularity)
    std::map<size_t, size_t> step_freq;
    for (auto s : steps) step_freq[s]++;
    size_t a = std::max_element(step_freq.begin(), step_freq.end(),
                                [](const auto& p1, const auto& p2) {
                                    return p1.second < p2.second;
                                })->first;

    std::cout << "Pattern detected: actual_size = " << n << " + " << a << " × x\n";
    std::cout << "  n (base/minimum) = " << n << " bytes\n";
    std::cout << "  a (granularity)  = " << a << " bytes\n\n";

    // Phase 2: Show only when step changes (compact view)
    std::cout << "[Phase 2] Allocation size classes (showing step changes only):\n\n";
    std::cout << std::setw(6) << "x"
              << std::setw(14) << "Formula"
              << std::setw(15) << "Actual Size"
              << std::setw(20) << "Request Range"
              << std::setw(10) << "Step\n";
    std::cout << std::string(65, '-') << "\n";

    prev_actual = 0;
    size_t prev_step = 0;
    size_t x = 0;
    size_t last_req = 1;
    size_t last_x = 0;
    size_t last_actual = 0;
    size_t last_last_req = 1;

    const size_t display_limit = 1024 * 1024;  // Scan up to 1MB

    for (size_t req = 1; req <= display_limit; ++req) {
        void* ptr = std::malloc(req);
        if (!ptr) break;
        size_t actual = get_actual_allocation_size(ptr, req);
        std::free(ptr);

        if (actual != prev_actual) {
            if (prev_actual > 0 && actual < prev_actual) {
                // Skip backwards jumps (allocator weirdness)
                continue;
            }

            size_t step = (prev_actual > 0) ? (actual - prev_actual) : 0;

            // Print only when step changes or first entry
            if (step != prev_step || prev_step == 0) {
                std::cout << std::setw(5) << x;
                std::cout << std::setw(8) << n << "+" << a << "×" << x;

                std::cout << std::setw(12);
                if (actual >= 1024*1024) {
                    std::cout << std::fixed << std::setprecision(1)
                             << (actual / (1024.0*1024.0)) << " MB";
                } else if (actual >= 1024) {
                    std::cout << std::fixed << std::setprecision(1)
                             << (actual / 1024.0) << " KB";
                } else {
                    std::cout << actual << " B";
                }

                std::cout << std::setw(11) << last_req << " - "
                          << std::setw(6) << (req - 1) << " B";

                if (step > 0) {
                    std::cout << std::setw(9) << "+" << step << " B";
                }
                std::cout << "\n";
            }

            // Remember last entry for printing at end
            last_x = x;
            last_actual = actual;
            last_last_req = last_req;

            last_req = req;
            prev_actual = actual;
            prev_step = step;
            x++;
        }
    }

    // Print last entry
    if (last_x > 0 && last_x != x - 1) {
        std::cout << std::setw(5) << (x - 1);
        std::cout << std::setw(8) << n << "+" << a << "×" << (x - 1);

        std::cout << std::setw(12);
        if (prev_actual >= 1024*1024) {
            std::cout << std::fixed << std::setprecision(1)
                     << (prev_actual / (1024.0*1024.0)) << " MB";
        } else if (prev_actual >= 1024) {
            std::cout << std::fixed << std::setprecision(1)
                     << (prev_actual / 1024.0) << " KB";
        } else {
            std::cout << prev_actual << " B";
        }

        std::cout << std::setw(11) << last_last_req << " - "
                  << std::setw(6) << (display_limit - 1) << " B";

        std::cout << std::setw(9) << "+" << prev_step << " B";
        std::cout << "\n";
    }

    // Phase 3: Recommendations
    std::cout << "\n[Recommendations for FetchList]\n";
    std::cout << "  Target block sizes (multiples of " << a << " bytes):\n";

    for (size_t mult = 1; mult <= 256; mult *= 2) {
        size_t block_size = a * mult;
        if (block_size >= n && block_size <= 4096) {
            std::cout << "    - " << std::setw(4) << block_size << " bytes"
                      << "  (fits " << (block_size / 8) << " × 8-byte elements,"
                      << " " << (block_size / 16) << " × 16-byte elements)\n";
        }
    }

    return 0;
}
