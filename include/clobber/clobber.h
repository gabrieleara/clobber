// Copyright 2023 Gabriele Ara
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Most of this code is originally distributed by Google as part of the
// "Benchmark" project.
//
// Check out the original project here: https://github.com/google/benchmark
//
// Changes applied to the original code:
// - Changed namespace and prefixes of all macros to use "clobber", instead
//   of "benchmark";
// - Removed backward compatibility with pre-C++17;
// - Removed all references to benchmark classes and objects (only the
//   "DoNotOptimize" and "ClobberMemory" functions are kept in this
//   project).
//
// In compliance with the Apachev2 license, the original copyright notice
// follows.

// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CLOBBER_CLOBBER_H
#define CLOBBER_CLOBBER_H

#define CLOBBER_HAS_CXX11
#define CLOBBER_HAS_CXX17

#include <atomic>
// #include <initializer_list>
// #include <type_traits>
// #include <utility>

#if defined(_MSC_VER)
#include <intrin.h> // for _ReadWriteBarrier
#endif

// Used to annotate functions, methods and classes so they
// are not optimized by the compiler. Useful for tests
// where you expect loops to stay in place churning cycles
#if defined(__clang__)
#define CLOBBER_DONT_OPTIMIZE __attribute__((optnone))
#elif defined(__GNUC__) || defined(__GNUG__)
#define CLOBBER_DONT_OPTIMIZE __attribute__((optimize(0)))
#else
// MSVC & Intel do not have a no-optimize attribute, only line pragmas
#define CLOBBER_DONT_OPTIMIZE
#endif

#if defined(__GNUC__) || defined(__clang__)
#define CLOBBER_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER) && !defined(__clang__)
#define CLOBBER_ALWAYS_INLINE __forceinline
#define __func__ __FUNCTION__
#else
#define CLOBBER_ALWAYS_INLINE
#endif

#define CLOBBER_INTERNAL_TOSTRING2(x) #x
#define CLOBBER_INTERNAL_TOSTRING(x) CLOBBER_INTERNAL_TOSTRING2(x)

// clang-format off
#if (defined(__GNUC__) && !defined(__NVCC__) && !defined(__NVCOMPILER)) || defined(__clang__)
#define CLOBBER_BUILTIN_EXPECT(x, y) __builtin_expect(x, y)
#define CLOBBER_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#define CLOBBER_DISABLE_DEPRECATED_WARNING \
  _Pragma("GCC diagnostic push")             \
  _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define CLOBBER_RESTORE_DEPRECATED_WARNING _Pragma("GCC diagnostic pop")
#elif defined(__NVCOMPILER)
#define CLOBBER_BUILTIN_EXPECT(x, y) __builtin_expect(x, y)
#define CLOBBER_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#define CLOBBER_DISABLE_DEPRECATED_WARNING \
  _Pragma("diagnostic push") \
  _Pragma("diag_suppress deprecated_entity_with_custom_message")
#define CLOBBER_RESTORE_DEPRECATED_WARNING _Pragma("diagnostic pop")
#else
#define CLOBBER_BUILTIN_EXPECT(x, y) x
#define CLOBBER_DEPRECATED_MSG(msg)
#define CLOBBER_WARNING_MSG(msg)                           \
  __pragma(message(__FILE__ "(" CLOBBER_INTERNAL_TOSTRING( \
      __LINE__) ") : warning note: " msg))
#define CLOBBER_DISABLE_DEPRECATED_WARNING
#define CLOBBER_RESTORE_DEPRECATED_WARNING
#endif
// clang-format on

#if defined(__GNUC__) && !defined(__clang__)
#define CLOBBER_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if defined(__GNUC__) || __has_builtin(__builtin_unreachable)
#define CLOBBER_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define CLOBBER_UNREACHABLE() __assume(false)
#else
#define CLOBBER_UNREACHABLE() ((void)0)
#endif

#ifdef CLOBBER_HAS_CXX11
#define CLOBBER_OVERRIDE override
#else
#define CLOBBER_OVERRIDE
#endif

#if defined(_MSC_VER)
#pragma warning(push)
// C4251: <symbol> needs to have dll-interface to be used by clients of class
#pragma warning(disable : 4251)
#endif

namespace clobber {
    namespace internal {
        void UseCharPointer(char const volatile *);
    }
}

namespace clobber {

#if (!defined(__GNUC__) && !defined(__clang__)) || defined(__pnacl__) ||       \
    defined(__EMSCRIPTEN__)
#define CLOBBER_HAS_NO_INLINE_ASSEMBLY
#endif

// Force the compiler to flush pending writes to global memory. Acts as an
// effective read/write barrier
#ifdef CLOBBER_HAS_CXX11
    inline CLOBBER_ALWAYS_INLINE void ClobberMemory() {
        std::atomic_signal_fence(std::memory_order_acq_rel);
    }
#endif

// The DoNotOptimize(...) function can be used to prevent a value or
// expression from being optimized away by the compiler. This function is
// intended to add little to no overhead.
// See: https://youtu.be/nXaxk27zwlk?t=2441
#ifndef CLOBBER_HAS_NO_INLINE_ASSEMBLY
#if !defined(__GNUC__) || defined(__llvm__) || defined(__INTEL_COMPILER)
    template <class Tp>
    CLOBBER_DEPRECATED_MSG("The const-ref version of this method can permit "
                           "undesired compiler optimizations in CLOBBERs")
    inline CLOBBER_ALWAYS_INLINE void DoNotOptimize(Tp const &value) {
        asm volatile("" : : "r,m"(value) : "memory");
    }

    template <class Tp>
    inline CLOBBER_ALWAYS_INLINE void DoNotOptimize(Tp &value) {
#if defined(__clang__)
        asm volatile("" : "+r,m"(value) : : "memory");
#else
        asm volatile("" : "+m,r"(value) : : "memory");
#endif
    }

#ifdef CLOBBER_HAS_CXX11
    template <class Tp>
    inline CLOBBER_ALWAYS_INLINE void DoNotOptimize(Tp &&value) {
#if defined(__clang__)
        asm volatile("" : "+r,m"(value) : : "memory");
#else
        asm volatile("" : "+m,r"(value) : : "memory");
#endif
    }
#endif
#elif defined(CLOBBER_HAS_CXX11) && (__GNUC__ >= 5)
    // Workaround for a bug with full argument copy overhead with GCC.
    // See: #1340 and https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105519
    template <class Tp>
    CLOBBER_DEPRECATED_MSG("The const-ref version of this method can permit "
                           "undesired compiler optimizations in CLOBBERs")
    inline CLOBBER_ALWAYS_INLINE
        typename std::enable_if<std::is_trivially_copyable<Tp>::value &&
                                (sizeof(Tp) <= sizeof(Tp *))>::type
        DoNotOptimize(Tp const &value) {
        asm volatile("" : : "r,m"(value) : "memory");
    }

    template <class Tp>
    CLOBBER_DEPRECATED_MSG("The const-ref version of this method can permit "
                           "undesired compiler optimizations in CLOBBERs")
    inline CLOBBER_ALWAYS_INLINE
        typename std::enable_if<!std::is_trivially_copyable<Tp>::value ||
                                (sizeof(Tp) > sizeof(Tp *))>::type
        DoNotOptimize(Tp const &value) {
        asm volatile("" : : "m"(value) : "memory");
    }

    template <class Tp>
    inline CLOBBER_ALWAYS_INLINE
        typename std::enable_if<std::is_trivially_copyable<Tp>::value &&
                                (sizeof(Tp) <= sizeof(Tp *))>::type
        DoNotOptimize(Tp &value) {
        asm volatile("" : "+m,r"(value) : : "memory");
    }

    template <class Tp>
    inline CLOBBER_ALWAYS_INLINE
        typename std::enable_if<!std::is_trivially_copyable<Tp>::value ||
                                (sizeof(Tp) > sizeof(Tp *))>::type
        DoNotOptimize(Tp &value) {
        asm volatile("" : "+m"(value) : : "memory");
    }

    template <class Tp>
    inline CLOBBER_ALWAYS_INLINE
        typename std::enable_if<std::is_trivially_copyable<Tp>::value &&
                                (sizeof(Tp) <= sizeof(Tp *))>::type
        DoNotOptimize(Tp &&value) {
        asm volatile("" : "+m,r"(value) : : "memory");
    }

    template <class Tp>
    inline CLOBBER_ALWAYS_INLINE
        typename std::enable_if<!std::is_trivially_copyable<Tp>::value ||
                                (sizeof(Tp) > sizeof(Tp *))>::type
        DoNotOptimize(Tp &&value) {
        asm volatile("" : "+m"(value) : : "memory");
    }

#else
    // Fallback for GCC < 5. Can add some overhead because the compiler is
    // forced to use memory operations instead of operations with registers.
    // TODO: Remove if GCC < 5 will be unsupported.
    template <class Tp>
    CLOBBER_DEPRECATED_MSG("The const-ref version of this method can permit "
                           "undesired compiler optimizations in CLOBBERs")
    inline CLOBBER_ALWAYS_INLINE void DoNotOptimize(Tp const &value) {
        asm volatile("" : : "m"(value) : "memory");
    }

    template <class Tp>
    inline CLOBBER_ALWAYS_INLINE void DoNotOptimize(Tp &value) {
        asm volatile("" : "+m"(value) : : "memory");
    }

#ifdef CLOBBER_HAS_CXX11
    template <class Tp>
    inline CLOBBER_ALWAYS_INLINE void DoNotOptimize(Tp &&value) {
        asm volatile("" : "+m"(value) : : "memory");
    }
#endif
#endif

#ifndef CLOBBER_HAS_CXX11
    inline CLOBBER_ALWAYS_INLINE void ClobberMemory() {
        asm volatile("" : : : "memory");
    }
#endif
#elif defined(_MSC_VER)
    template <class Tp>
    CLOBBER_DEPRECATED_MSG("The const-ref version of this method can permit "
                           "undesired compiler optimizations in CLOBBERs")
    inline CLOBBER_ALWAYS_INLINE void DoNotOptimize(Tp const &value) {
        internal::UseCharPointer(
            &reinterpret_cast<char const volatile &>(value));
        _ReadWriteBarrier();
    }

#ifndef CLOBBER_HAS_CXX11
    inline CLOBBER_ALWAYS_INLINE void ClobberMemory() {
        _ReadWriteBarrier();
    }
#endif
#else
    template <class Tp>
    CLOBBER_DEPRECATED_MSG("The const-ref version of this method can permit "
                           "undesired compiler optimizations in CLOBBERs")
    inline CLOBBER_ALWAYS_INLINE void DoNotOptimize(Tp const &value) {
        internal::UseCharPointer(
            &reinterpret_cast<char const volatile &>(value));
    }
// FIXME Add ClobberMemory() for non-gnu and non-msvc compilers, before C++11.
#endif

} // namespace clobber

#endif // CLOBBER_CLOBBER_H
