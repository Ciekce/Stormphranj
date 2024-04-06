/*
 * Stormphranj, a UCI shatranj engine
 * Copyright (C) 2024 Ciekce
 *
 * Stormphranj is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stormphranj is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stormphranj. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "types.h"

#if defined(SPJ_NATIVE)
	// cannot expand a macro to defined()
	#if __BMI2__ && defined(SPJ_FAST_PEXT)
		#define SPJ_HAS_BMI2 1
	#else
		#define SPJ_HAS_BMI2 0
	#endif
	#define SPJ_HAS_AVX512 __AVX512F__
	#define SPJ_HAS_AVX2 __AVX2__
	#define SPJ_HAS_NEON __neon__
	#define SPJ_HAS_BMI1 __BMI__
	#define SPJ_HAS_POPCNT __POPCNT__
	#define SPJ_HAS_SSE41 __SSE4_1__
#elif defined(SPJ_AVX512)
	#define SPJ_HAS_BMI2 1
	#define SPJ_HAS_AVX512 1
	#define SPJ_HAS_AVX2 1
	#define SPJ_HAS_NEON 0
	#define SPJ_HAS_BMI1 1
	#define SPJ_HAS_POPCNT 1
	#define SPJ_HAS_SSE41 1
#elif defined(SPJ_AVX2_BMI2)
	#define SPJ_HAS_BMI2 1
	#define SPJ_HAS_AVX512 0
	#define SPJ_HAS_AVX2 1
	#define SPJ_HAS_NEON 0
	#define SPJ_HAS_BMI1 1
	#define SPJ_HAS_POPCNT 1
	#define SPJ_HAS_SSE41 1
#elif defined(SPJ_AVX2)
	#define SPJ_HAS_BMI2 0
	#define SPJ_HAS_AVX512 0
	#define SPJ_HAS_AVX2 1
	#define SPJ_HAS_NEON 0
	#define SPJ_HAS_BMI1 1
	#define SPJ_HAS_POPCNT 1
	#define SPJ_HAS_SSE41 1
#elif defined(SPJ_SSE41_POPCNT)
	#define SPJ_HAS_BMI2 0
	#define SPJ_HAS_AVX512 0
	#define SPJ_HAS_AVX2 0
	#define SPJ_HAS_NEON 0
	#define SPJ_HAS_BMI1 0
	#define SPJ_HAS_POPCNT 1
	#define SPJ_HAS_SSE41 1
#else
#error no arch specified
#endif

// should be good on all(tm) architectures
#define SPJ_CACHE_LINE_SIZE (64)
