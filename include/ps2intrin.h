#pragma once

/*
*
*
*
*/


#if defined(_MSC_VER)
#define FORCEINLINE __forceinline
#elif defined(_GNU)
#define FORCEINLINE __attribute__ ((always_inline))
#else
#warning "Could not determine 'always_inline' analog for current compiler"
#define FORCEINLINE
#endif

#ifdef __cplusplus
extern "C" {
#endif


	FORCEINLINE void div1_i32_start(int dividend, int divisor)
	{

	}

	FORCEINLINE int div1_i32_finish(int* remainder)
	{

	}


	/// @brief DIV1 : Divide Word Pipeline 1
	/// @param dividend number do divide
	/// @param divisor number to divide by
	/// @param remainder where to store the reminder, may not be null
	/// @return division result
	/// INT_MIN divided by -1 returns INT_MIN and stores 0 to reminder
	/// divide by 0 is undefined
	FORCEINLINE int div1_i32(int dividend, int divisor, int* remainder)
	{
		div1_i32_start(dividend, divisor);
	}

	/// @brief DIVU1 : Divide Unsigned Word Pipeline 1
	/// @param dividend number do divide
	/// @param divisor number to divide by
	/// @param remainder where to store the reminder, may not be null
	/// @return division result
	/// divide by 0 is undefined
	FORCEINLINE unsigned div1(unsigned dividend, unsigned divisor, unsigned* remainder);

	//LQ : Load Quadword
	//	why would someone call this? just load a 128-bit sized and aligned data type



#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#endif
