/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 *
 * @modified    Tom Clay, 2026 - Adapted for ReXGlue runtime
 */

#pragma once

// This file contains the main platform switches used by rex as well as any
// fixups required to normalize the environment. Everything in here should be
// largely portable.
// Platform-specific headers, like platform_win.h, are used to house any
// super platform-specific stuff that implies code is not platform-agnostic.
//
// NOTE: ordering matters here as sometimes multiple flags are defined on
// certain platforms.
//
// Great resource on predefined macros:
// https://sourceforge.net/p/predef/wiki/OperatingSystems/
// Original link: https://predef.sourceforge.net/preos.html

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__SWITCH__) || defined(__SWITCH2__)
#define REX_PLATFORM_SWITCH2 1
#define REX_PLATFORM_LINUX 1  // POSIX-like: Horizon via libounce/newlib
#elif defined(TARGET_OS_MAC) && TARGET_OS_MAC
#define REX_PLATFORM_MAC 1
#elif defined(WIN32) || defined(_WIN32)
#define REX_PLATFORM_WIN32 1
#elif defined(__ANDROID__)
#define REX_PLATFORM_ANDROID 1
#define REX_PLATFORM_LINUX 1
#elif defined(__gnu_linux__)
#define REX_PLATFORM_GNU_LINUX 1
#define REX_PLATFORM_LINUX 1
#else
#error Unsupported target OS.
#endif

// Ensure all platform macros are always defined (0 when inactive)
// so they can be used in static_assert and regular expressions.
#ifndef REX_PLATFORM_SWITCH2
#define REX_PLATFORM_SWITCH2 0
#endif
#ifndef REX_PLATFORM_MAC
#define REX_PLATFORM_MAC 0
#endif
#ifndef REX_PLATFORM_WIN32
#define REX_PLATFORM_WIN32 0
#endif
#ifndef REX_PLATFORM_ANDROID
#define REX_PLATFORM_ANDROID 0
#endif
#ifndef REX_PLATFORM_GNU_LINUX
#define REX_PLATFORM_GNU_LINUX 0
#endif
#ifndef REX_PLATFORM_LINUX
#define REX_PLATFORM_LINUX 0
#endif

#if defined(__clang__)
#define REX_COMPILER_CLANG 1
#elif defined(__GNUC__)
#define REX_COMPILER_GNUC 1
#elif defined(_MSC_VER)
#define REX_COMPILER_MSVC 1
#elif defined(__MINGW32)
#define REX_COMPILER_MINGW32 1
#elif defined(__INTEL_COMPILER)
#define REX_COMPILER_INTEL 1
#else
#define REX_COMPILER_UNKNOWN 1
#endif

#if defined(_M_AMD64) || defined(__amd64__)
#define REX_ARCH_AMD64 1
#elif defined(_M_ARM64) || defined(__aarch64__)
#define REX_ARCH_ARM64 1
#elif defined(_M_IX86) || defined(__i386__) || defined(_M_ARM) || defined(__arm__)
#error Rex is not supported on 32-bit platforms.
#elif defined(_M_PPC) || defined(__powerpc__)
#define REX_ARCH_PPC 1
#endif

#if REX_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX  // Don't want windows.h including min/max macros.
#endif            // REX_PLATFORM_WIN32

#if REX_PLATFORM_WIN32
#include <intrin.h>
#elif REX_ARCH_AMD64
#include <x86intrin.h>
#endif  // REX_PLATFORM_WIN32

#if REX_PLATFORM_MAC
#include <libkern/OSByteOrder.h>
#endif  // REX_PLATFORM_MAC

#include <bit>
#include <cstdint>

//=============================================================================
// Compiler Polyfills
//=============================================================================

#if defined(__clang__)
// Clang has builtin rotate functions and debugtrap
#elif defined(__GNUC__)
#ifndef __builtin_rotateleft32
#define __builtin_rotateleft32(x, n) std::rotl(static_cast<uint32_t>(x), static_cast<int>(n))
#endif
#ifndef __builtin_rotateleft64
#define __builtin_rotateleft64(x, n) std::rotl(static_cast<uint64_t>(x), static_cast<int>(n))
#endif
#ifndef __builtin_debugtrap
#if defined(__x86_64__) || defined(__i386__)
#define __builtin_debugtrap() __asm__ __volatile__("int3")
#else
#define __builtin_debugtrap() __builtin_trap()
#endif
#endif
#endif

#if REX_COMPILER_MSVC
#define _REXPACKEDSCOPE(body) __pragma(pack(push, 1)) body __pragma(pack(pop));
#else
#define _REXPACKEDSCOPE(body)    \
  _Pragma("pack(push, 1)") body; \
  _Pragma("pack(pop)");
#endif  // REX_COMPILER_MSVC

#define REXPACKEDSTRUCT(name, value) _REXPACKEDSCOPE(struct name value)
#define REXPACKEDSTRUCTANONYMOUS(value) _REXPACKEDSCOPE(struct value)
#define REXPACKEDUNION(name, value) _REXPACKEDSCOPE(union name value)

// Compiler capability macros
#if REX_COMPILER_CLANG || REX_COMPILER_GNUC
#define REX_HAS_BUILTIN_STRLEN 1
#define REX_LACKS_FLOAT_FROM_CHARS 1
#else
#define REX_HAS_BUILTIN_STRLEN 0
#define REX_LACKS_FLOAT_FROM_CHARS 0
#endif

namespace rex {

#if REX_PLATFORM_WIN32
const char kPathSeparator = '\\';
#else
const char kPathSeparator = '/';
#endif  // REX_PLATFORM_WIN32

const char kGuestPathSeparator = '\\';

}  // namespace rex

#if REX_PLATFORM_SWITCH2 || defined(REX_SIM_ARENA)
#define REX_ARENA_MEMORY 1
#else
#define REX_ARENA_MEMORY 0
#endif

#if REX_ARENA_MEMORY
//=============================================================================
// Switch 2 guest memory layout (also used on PC with REX_SIM_ARENA for testing).
// Horizon cannot alias one backing object at several addresses, so the guest
// 4 GB view is one flat arena and the alias ranges are folded onto it:
//   arena + [0x00000000, 0x7F000000)  guest virtual (identity)
//   arena + [0x80000000, 0x90000000)  xex (0x90000000-0x9FFFFFFF folds here)
//   arena + 0x90000000 + [0, 0x20001000) physical (0x7F000000 view, 0xA0000000
//                                         .. 0xFFFFFFFF all fold here)
//=============================================================================
#define REX_S2_PHYS_BASE 0x90000000ull
#define REX_S2_ARENA_SIZE (0x90000000ull + 0x20001000ull)

// Out-of-line slow path for the alias ranges (physical views, 0x7F000000, 0x90000000+).
__attribute__((noinline)) inline int64_t rex_s2_fold_delta_slow(uint32_t a) {
  int64_t f;
  if (a < 0x7F000000u) {
    f = a;
  } else if (a < 0x80000000u) {
    f = REX_S2_PHYS_BASE + (a - 0x7F000000u);
  } else if (a < 0x90000000u) {
    f = a;
  } else if (a < 0xA0000000u) {
    f = int64_t(a) - 0x10000000;
  } else if (a < 0xE0000000u) {
    f = REX_S2_PHYS_BASE + (a & 0x1FFFFFFFu);
  } else {
    f = REX_S2_PHYS_BASE + 0x1000 + (a - 0xE0000000u);
  }
  return f - int64_t(a);
}

// Called for guest memory accesses from generated code. Nearly all of them target guest virtual
// memory (< 0x7F000000) or the xex image (0x80000000-0x8FFFFFFF), whose delta is 0: keep that
// path inline (GCC would not inline the full chain into the huge generated functions).
__attribute__((always_inline)) inline int64_t rex_s2_fold_delta(uint32_t a) {
  if (__builtin_expect(a < 0x7F000000u || (a - 0x80000000u) < 0x10000000u, 1)) {
    return 0;
  }
  return rex_s2_fold_delta_slow(a);
}
#endif

#if REX_PLATFORM_SWITCH2
#include <cstdio>
// Unbuffered step trace for on-device bring-up (survives hard crashes).
inline void rex_s2_trace(const char* msg) {
  if (FILE* f = std::fopen("sdmc:/switch/skate3/trace.txt", "a")) {
    std::fprintf(f, "%s\n", msg);
    std::fflush(f);
    std::fclose(f);
  }
}
#define REX_S2_TRACE(m) rex_s2_trace(m)
#else
#define REX_S2_TRACE(m) ((void)0)
#endif
