include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(CheckFunctionExists)
include(TestBigEndian)

function(lcms2_detect_features)
  # Endianness
  test_big_endian(LCMS2_WORDS_BIGENDIAN)
  if(LCMS2_WORDS_BIGENDIAN)
    set(LCMS2_DEFINE_WORDS_BIGENDIAN 1 PARENT_SCOPE)
  else()
    set(LCMS2_DEFINE_WORDS_BIGENDIAN 0 PARENT_SCOPE)
  endif()

  # gmtime_r / gmtime_s
  set(CMAKE_REQUIRED_DEFINITIONS "")
  set(CMAKE_REQUIRED_LIBRARIES "")

  check_function_exists(gmtime_r LCMS2_HAVE_GMTIME_R)

  set(_gmtime_s_src "
    #include <time.h>
    int main(void) {
      time_t t = (time_t)0;
      struct tm m;
      (void)gmtime_s(&m, &t);
      return 0;
    }
  ")
  check_c_source_compiles("${_gmtime_s_src}" LCMS2_HAVE_GMTIME_S)

  set(LCMS2_DEFINE_HAVE_GMTIME_R ${LCMS2_HAVE_GMTIME_R} PARENT_SCOPE)
  set(LCMS2_DEFINE_HAVE_GMTIME_S ${LCMS2_HAVE_GMTIME_S} PARENT_SCOPE)

  # Visibility attribute support (GCC/Clang).
  set(_vis_src "
    __attribute__((visibility(\"default\"))) int foo(void) { return 0; }
    int main(void) { return foo(); }
  ")
  check_c_source_compiles("${_vis_src}" LCMS2_HAVE_FUNC_ATTRIBUTE_VISIBILITY)
  set(LCMS2_DEFINE_HAVE_FUNC_ATTRIBUTE_VISIBILITY ${LCMS2_HAVE_FUNC_ATTRIBUTE_VISIBILITY} PARENT_SCOPE)

  # -fvisibility=hidden support (non-Windows, GCC/Clang).
  check_c_compiler_flag("-fvisibility=hidden" LCMS2_HAVE_FLAG_FVISIBILITY_HIDDEN)
  set(LCMS2_USE_FLAG_FVISIBILITY_HIDDEN ${LCMS2_HAVE_FLAG_FVISIBILITY_HIDDEN} PARENT_SCOPE)

  # SSE2 intrinsics
  set(_sse2_src "
    #include <emmintrin.h>
    int main(void) {
      __m128i n = _mm_set1_epi8(42);
      (void)n;
      return 0;
    }
  ")
  check_c_source_compiles("${_sse2_src}" LCMS2_COMPILER_SUPPORTS_SSE2)
  if(LCMS2_COMPILER_SUPPORTS_SSE2)
    set(LCMS2_DEFINE_CMS_DONT_USE_SSE2 0 PARENT_SCOPE)
  else()
    set(LCMS2_DEFINE_CMS_DONT_USE_SSE2 1 PARENT_SCOPE)
  endif()

  # libm detection (for Linux and similar platforms).
  find_library(LCMS2_MATH_LIB m)
  if(LCMS2_MATH_LIB)
    set(LCMS2_LINK_LIBM 1 PARENT_SCOPE)
  else()
    set(LCMS2_LINK_LIBM 0 PARENT_SCOPE)
  endif()
endfunction()


