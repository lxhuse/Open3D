// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018-2021 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#pragma once

#include "open3d/core/Dtype.h"
#include "open3d/utility/Logging.h"
#include "open3d/utility/MiniVec.h"

// TODO: dispatch more combinations.
#define DISPATCH_DTYPE_AND_DIM_TO_TEMPLATE(DTYPE, DIM, ...)                  \
    [&] {                                                                    \
        if (DTYPE == open3d::core::Int32) {                                  \
            if (DIM == 1) {                                                  \
                using key_t = utility::MiniVec<int, 1>;                      \
                using hash_t = utility::MiniVecHash<int, 1>;                 \
                using eq_t = utility::MiniVecEq<int, 1>;                     \
                return __VA_ARGS__();                                        \
            } else if (DIM == 2) {                                           \
                using key_t = utility::MiniVec<int, 2>;                      \
                using hash_t = utility::MiniVecHash<int, 2>;                 \
                using eq_t = utility::MiniVecEq<int, 2>;                     \
                return __VA_ARGS__();                                        \
            } else if (DIM == 3) {                                           \
                using key_t = utility::MiniVec<int, 3>;                      \
                using hash_t = utility::MiniVecHash<int, 3>;                 \
                using eq_t = utility::MiniVecEq<int, 3>;                     \
                return __VA_ARGS__();                                        \
            }                                                                \
        } else if (DTYPE == open3d::core::Int64) {                           \
            if (DIM == 1) {                                                  \
                using key_t = utility::MiniVec<int64_t, 1>;                  \
                using hash_t = utility::MiniVecHash<int64_t, 1>;             \
                using eq_t = utility::MiniVecEq<int64_t, 1>;                 \
                return __VA_ARGS__();                                        \
            } else if (DIM == 2) {                                           \
                using key_t = utility::MiniVec<int64_t, 2>;                  \
                using hash_t = utility::MiniVecHash<int64_t, 2>;             \
                using eq_t = utility::MiniVecEq<int64_t, 2>;                 \
                return __VA_ARGS__();                                        \
            } else if (DIM == 3) {                                           \
                using key_t = utility::MiniVec<int64_t, 3>;                  \
                using hash_t = utility::MiniVecHash<int64_t, 3>;             \
                using eq_t = utility::MiniVecEq<int64_t, 3>;                 \
                return __VA_ARGS__();                                        \
            }                                                                \
        } else {                                                             \
            utility::LogError("Unsupported dtype {} and dim {} combination", \
                              DTYPE.ToString(), DIM);                        \
        }                                                                    \
    }()

#ifdef __CUDACC__
// Reinterpret hash maps' void* value arrays as CUDA primitive types arrays, to
// avoid slow memcpy or byte-by-byte copy in kernels.
// Not used in the CPU version since memcpy is relatively fast on CPU.
#define DISPATCH_DIVISOR_SIZE_TO_BLOCK_T(DIVISOR, ...) \
    [&] {                                              \
        if (DIVISOR == 16) {                           \
            using block_t = int4;                      \
            return __VA_ARGS__();                      \
        } else if (DIVISOR == 12) {                    \
            using block_t = int3;                      \
            return __VA_ARGS__();                      \
        } else if (DIVISOR == 8) {                     \
            using block_t = int2;                      \
            return __VA_ARGS__();                      \
        } else if (DIVISOR == 4) {                     \
            using block_t = int;                       \
            return __VA_ARGS__();                      \
        } else if (DIVISOR == 2) {                     \
            using block_t = int16_t;                   \
            return __VA_ARGS__();                      \
        } else {                                       \
            using block_t = uint8_t;                   \
            return __VA_ARGS__();                      \
        }                                              \
    }()
#endif

namespace open3d {
namespace utility {

template <typename T, int N>
struct MiniVecHash {
public:
    OPEN3D_HOST_DEVICE uint64_t operator()(const MiniVec<T, N>& key) const {
        uint64_t hash = UINT64_C(14695981039346656037);
#if defined(__CUDA_ARCH__)
#pragma unroll
#endif
        for (int i = 0; i < N; ++i) {
            hash ^= static_cast<uint64_t>(key[i]);
            hash *= UINT64_C(1099511628211);
        }
        return hash;
    }
};

template <typename T, int N>
struct MiniVecEq {
public:
    OPEN3D_HOST_DEVICE bool operator()(const MiniVec<T, N>& lhs,
                                       const MiniVec<T, N>& rhs) const {
        bool is_equal = true;
#if defined(__CUDA_ARCH__)
#pragma unroll
#endif
        for (int i = 0; i < N; ++i) {
            is_equal = is_equal && (lhs[i] == rhs[i]);
        }
        return is_equal;
    }
};

}  // namespace utility
}  // namespace open3d
