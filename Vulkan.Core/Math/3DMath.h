#pragma once

#include "Common.h"
//#include <math.h>



namespace Math
{



#define IEEE_FLT_MANTISSA_BITS		23

const float	PI = 3.14159265358979323846f;
const float	M_DEG2RAD = PI / 180.0f;
const float	M_RAD2DEG = 180.0f / PI;
	
	

#define DEG2RAD(a)				( (a) * Math::M_DEG2RAD )
#define RAD2DEG(a)				( (a) * Math::M_RAD2DEG )


#define C_FLOAT_TO_INT( x )		(int)(x)

	const int SMALLEST_NON_DENORMAL = 1 << IEEE_FLT_MANTISSA_BITS;
	const int NAN_VALUE = 0x7f800000;

	const float FLT_SMALLEST_NON_DENORMAL = *reinterpret_cast<const float *>(&SMALLEST_NON_DENORMAL);	// 1.1754944e-038f


	INLINE unsigned char Ftob(float f) {
#ifdef ID_WIN_X86_SSE_INTRIN
		// If a converted result is negative the value (0) is returned and if the
		// converted result is larger than the maximum byte the value (255) is returned.
		__m128 x = _mm_load_ss(&f);
		x = _mm_max_ss(x, SIMD_SP_zero);
		x = _mm_min_ss(x, SIMD_SP_255);
		return static_cast<byte>(_mm_cvttss_si32(x));
#else
		// The converted result is clamped to the range [0,255].
		int i = C_FLOAT_TO_INT(f);
		if (i < 0) {
			return 0;
		}
		else if (i > 255) {
			return 255;
		}
		return static_cast<unsigned char>(i);
#endif
	}


	INLINE float InvSqrt(float x) {
#ifdef ID_WIN_X86_SSE_INTRIN

		return (x > FLT_SMALLEST_NON_DENORMAL) ? sqrtf(1.0f / x) : INFINITY;

#else

		return (x > FLT_SMALLEST_NON_DENORMAL) ? sqrtf(1.0f / x) : INFINITY;

#endif
	}


  INLINE int Ftoi(float f) {
#ifdef ID_WIN_X86_SSE_INTRIN
		// If a converted result is larger than the maximum signed doubleword integer,
		// the floating-point invalid exception is raised, and if this exception is masked,
		// the indefinite integer value (80000000H) is returned.
		__m128 x = _mm_load_ss(&f);
		return _mm_cvttss_si32(x);
#elif 0 // round chop (C/C++ standard)
		int i, s, e, m, shift;
		i = *reinterpret_cast<int *>(&f);
		s = i >> IEEE_FLT_SIGN_BIT;
		e = ((i >> IEEE_FLT_MANTISSA_BITS) & ((1 << IEEE_FLT_EXPONENT_BITS) - 1)) - IEEE_FLT_EXPONENT_BIAS;
		m = (i & ((1 << IEEE_FLT_MANTISSA_BITS) - 1)) | (1 << IEEE_FLT_MANTISSA_BITS);
		shift = e - IEEE_FLT_MANTISSA_BITS;
		return ((((m >> -shift) | (m << shift)) & ~(e >> INT32_SIGN_BIT)) ^ s) - s;
#else
		// If a converted result is larger than the maximum signed doubleword integer the result is undefined.
		return C_FLOAT_TO_INT(f);
#endif
	}

  /*
  ========================
  idMath::Fabs
  ========================
  */
  INLINE float Fabs(float f) {
#if 1
	  return fabsf(f);
#else
	  int tmp = *reinterpret_cast<int *>(&f);
	  tmp &= 0x7FFFFFFF;
	  return *reinterpret_cast<float *>(&tmp);
#endif
  }

  /*
  ========================
  idMath::Rint
  ========================
  */
  INLINE float Rint(float f) {
	  return floorf(f + 0.5f);
  }


}
