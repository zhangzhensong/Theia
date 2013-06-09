// Copyright (C) 2013 The Regents of the University of California (Regents).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
//     * Neither the name of The Regents or University of California nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Please contact the author of this library if you have any questions.
// Author: Chris Sweeney (cmsweeney@cs.ucsb.edu)

#ifndef VISION_MATCHING_DISTANCE_HAMMING_H_
#define VISION_MATCHING_DISTANCE_HAMMING_H_

#include <bitset>
#ifdef THEIA_USE_SSE
#include <emmintrin.h>
#include <tmmintrin.h>
#endif 
namespace theia {
namespace {
#ifdef __GNUC__
static const char __attribute__((aligned(16))) MASK_4bit[16] = {0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf};
static const uint8_t __attribute__((aligned(16))) POPCOUNT_4bit[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
static const __m128i shiftval = _mm_set_epi32 (0,0,0,4);
#endif
#ifdef _MSC_VER
__declspec(align(16)) static const char MASK_4bit[16] = {0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf};
__declspec(align(16)) static const uint8_t POPCOUNT_4bit[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
static const __m128i shiftval = _mm_set_epi32 (0,0,0,4);
#endif

inline int ssse3_popcntofXORed(const __m128i* signature1,
                               const __m128i* signature2,
                               const int numberOf128BitWords) {

  int result = 0;

  register __m128i xmm0;
  register __m128i xmm1;
  register __m128i xmm2;
  register __m128i xmm3;
  register __m128i xmm4;
  register __m128i xmm5;
  register __m128i xmm6;
  register __m128i xmm7;

  //__asm__ volatile ("movdqa (%0), %%xmm7" : : "a" (POPCOUNT_4bit) : "xmm7");
  xmm7 = _mm_load_si128 ((__m128i *)POPCOUNT_4bit);
  //__asm__ volatile ("movdqa (%0), %%xmm6" : : "a" (MASK_4bit) : "xmm6");
  xmm6 = _mm_load_si128 ((__m128i *)MASK_4bit);
  //__asm__ volatile ("pxor    %%xmm5, %%xmm5" : : : "xmm5"); // xmm5 -- global accumulator
  xmm5 = _mm_setzero_si128();

  const size_t end=(size_t)(signature1+numberOf128BitWords);

  //__asm__ volatile ("movdqa %xmm5, %xmm4"); // xmm4 -- local accumulator
  xmm4 = xmm5;//_mm_load_si128(&xmm5);

  //for (n=0; n < numberOf128BitWords; n++) {
  do{
    //__asm__ volatile ("movdqa (%0), %%xmm0" : : "a" (signature1++) : "xmm0"); //slynen load data for XOR
    //          __asm__ volatile(
    //                          "movdqa   (%0), %%xmm0  \n"
    //"pxor      (%0), %%xmm0   \n" //slynen do XOR
    xmm0 = _mm_xor_si128 ( (__m128i)*signature1++, (__m128i)*signature2++); //slynen load data for XOR and do XOR
    //                          "movdqu    %%xmm0, %%xmm1       \n"
    xmm1 = xmm0;//_mm_loadu_si128(&xmm0);
    //                          "psrlw         $4, %%xmm1       \n"
    xmm1 = _mm_srl_epi16 (xmm1, shiftval);
    //                          "pand      %%xmm6, %%xmm0       \n"     // xmm0 := lower nibbles
    xmm0 = _mm_and_si128 (xmm0, xmm6);
    //                          "pand      %%xmm6, %%xmm1       \n"     // xmm1 := higher nibbles
    xmm1 = _mm_and_si128 (xmm1, xmm6);
    //                          "movdqu    %%xmm7, %%xmm2       \n"
    xmm2 = xmm7;//_mm_loadu_si128(&xmm7);
    //                          "movdqu    %%xmm7, %%xmm3       \n"     // get popcount
    xmm3 = xmm7;//_mm_loadu_si128(&xmm7);
    //                          "pshufb    %%xmm0, %%xmm2       \n"     // for all nibbles
    xmm2 = _mm_shuffle_epi8(xmm2, xmm0);
    //                          "pshufb    %%xmm1, %%xmm3       \n"     // using PSHUFB
    xmm3 = _mm_shuffle_epi8(xmm3, xmm1);
    //                          "paddb     %%xmm2, %%xmm4       \n"     // update local
    xmm4 = _mm_add_epi8(xmm4, xmm2);
    //                          "paddb     %%xmm3, %%xmm4       \n"     // accumulator
    xmm4 = _mm_add_epi8(xmm4, xmm3);
    //                          :
    //                          : "a" (buffer++)
    //                          : "xmm0","xmm1","xmm2","xmm3","xmm4"
    //          );
  }while((size_t)signature1<end);
  // update global accumulator (two 32-bits counters)
  //    __asm__ volatile (
  //                    /*"pxor %xmm0, %xmm0            \n"*/
  //                    "psadbw %%xmm5, %%xmm4          \n"
  xmm4 = _mm_sad_epu8(xmm4, xmm5);
  //                    "paddd  %%xmm4, %%xmm5          \n"
  xmm5 = _mm_add_epi32(xmm5, xmm4);
  //                    :
  //                    :
  //                    : "xmm4","xmm5"
  //    );
  // finally add together 32-bits counters stored in global accumulator
  //    __asm__ volatile (
  //                    "movhlps   %%xmm5, %%xmm0       \n"
  xmm0 = _mm_cvtps_epi32(_mm_movehl_ps(_mm_cvtepi32_ps(xmm0),
                                       _mm_cvtepi32_ps(xmm5))); //TODO fix with appropriate intrinsic
  //                    "paddd     %%xmm5, %%xmm0       \n"
  xmm0 = _mm_add_epi32(xmm0, xmm5);
  //                    "movd      %%xmm0, %%eax        \n"
  result = _mm_cvtsi128_si32 (xmm0);
  //                    : "=a" (result) : : "xmm5","xmm0"
  //    );
  return result;
}

}
struct Hamming {
  typedef bool ElementType;
  typedef int ResultType;

  template <typename Iterator1, typename Iterator2>
  ResultType operator()(const Iterator1 a, const Iterator2 b,
                        const size_t size) const {
    // a^b = XOR op, then perform a popcnt on it. This should be optimized
    // rather heavily, and is architecture independent since it is part of the
    // c++ standard. TODO(cmsweeney): compare this vs brisk hamming distance
    // function to see which is faster.
    /*
#ifndef THEIA_USE_SSE
    return (a^b).count();
#else
    return ssse3_popcntofXORed((const __m128i*)&a,
                               (const __m128i*)&b,
                               size/16);

#endif  // THEIA_USE_SSE
*/
    int dist1 = (a^b).count();

    unsigned char temp_char[5];
    std::bitset<5*sizeof(unsigned char)>* temp_bit =
        reinterpret_cast<std::bitset<5*sizeof(unsigned char)>* >(temp_char);
    temp_bit->reset();
    VLOG(0) << "temp bit = " << temp_bit->to_string();
    int dist2 = ssse3_popcntofXORed((const __m128i*)(&a),
                                    (const __m128i*)(&b),
                                    size/16);
    CHECK_EQ(dist1, dist2) << "distances are not equivalent!";
    
  }
};

// max_early_dist is the maximum value of the distance of the first num_bits
// bits in order to continue testing the distance.
/*
  template<int max_early_dist = 0, size_t num_early_bits = 128>
  struct HammingEarlyOut {
  typedef bool ElementType;
  typedef int ResultType;

  template <typename Iterator1, typename Iterator2>
  ResultType operator()(Iterator1 a, Iterator2 b, size_t size) const {
  // Test the distance of the first
  std::bitset<size> ones;
  ones.set();
  // Do some bitshifting so that only the first num_early_bits are left as 1's.
  std::bitset<size> first_n_test =
  (ones >> (size - num_early_bits)) << (size - num_early_bits);
  VLOG(0) << "first n: " << fist_n_test.to_string();
  std::bitset<size> rest_test = (ones << num_early_bits) >> num_early_bits;
  VLOG(0) << "rest n: " << rest_test.to_string();
  // Calculate the hamming distance of the first num_early_bits.
  int fist_dist = ((a&first_n_test)^(b&first_n_test)).count();
  if (max_early_dist > 0 && max_early_dist < num_early_bits &&
  fist_dist > max_early_dist) {
  return size_t;
  } else {
  // Otherwise, return the normal distance.
  int rest_dist = ((a&rest_test)^(b&rest_test)).count();
  return fist_dist + rest_dist;
  // return (a^b).count();
  }
  }
  };
*/
}  // namespace theia
#endif  // VISION_MATCHING_DISTANCE_HAMMING_H_
