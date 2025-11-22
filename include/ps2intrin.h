#pragma once

/*
*	<*intrin.h>-like wrapper functions for pipeline 1, multiply-add and multimedia instructions.
*	Functions taking an immediate value as argument have macro versions exposing this
*	functionality. C++ convenience wrappers are provided as well.
*
*	These functions are declared 'always_inline'. You should not take their addresses or call
*	them using function pointers.
* 
*	The '__m128i' analogue are split up by integer type. There are plenty of no-op conversion
*	functions. This is done to make integer type mismatches easier to notice.
*	
*	A note for developers looking to use these functions:
*	The EE Core Multimedia Instructions have most support for these types: 'int16_t', 'int32_t'
*	and 'uint32_t'. If you plan on using these functions you should make sure your data is in
*	this format. The variants operating on 64-bit values are mostly just 32-bit values plus 32
*	bits of sign extension, not real 64-bit integers.
*/

#ifndef _EE
#error "This header only deals with EE-Core intrinsics and should not be used for the IOP or other architectures."
#endif

#include <stdint.h>
#include <string.h>		//	for memcpy

// This is mostly here to make working with IntelliSense easier.
#if defined(_MSC_VER)
#define FORCEINLINE __forceinline inline
#define CONST __declspec(noalias)
#define UNSEQUENCED __declspec(noalias)
#define PURE __declspec(noalias)
#define REPRODUCIBLE __declspec(noalias)
#define ALIGNAS(x) __declspec(align(x))
#define ALIGNVECTOR(x) __declspec(align(x))
#define MODE_TI
#elif defined(__GNUC__)
// Always inline this function into the caller. Must be both
// '(always_inline)' and 'inline' to guarantee inlining, as per gcc warning.
#define FORCEINLINE __attribute__ ((always_inline)) inline
// This function only reads arguments and returns a value.
#define CONST __attribute__ ((const))
// This function only reads arguments, returns a value and reads or writes memory through pointers
// in function arguments.
#define UNSEQUENCED __attribute__ ((unsequenced))
// This function only reads arguments, global state and returns a value.
#define PURE __attribute__ ((pure))
// This function only reads arguments, global state, returns a value and reads or writes memory
// through pointers in function arguments.
#define REPRODUCIBLE __attribute__ ((reproducible))
// Alignment specifier used for vector types. I'd use C++ 'alignas' specifier for this, but this
// header also needs to work in C.
#define ALIGNAS(x) __attribute__ ((aligned(x)))
#define ALIGNVECTOR(x) __attribute__ ((vector_size(x), aligned(x)))
// Integer size of 128 bits specifier
#define MODE_TI __attribute__((mode(TI)))
#else
#warning "Could not determine 'always_inline' analog for current compiler"
#define FORCEINLINE
#define CONST
#define UNSEQUENCED
#define PURE
#define REPRODUCIBLE
#define ALIGNAS(x)
#define ALIGNVECTOR(x)
#define MODE_TI
#endif

#ifdef __cplusplus
extern "C" {
#endif

	// Types

	typedef int int128_t MODE_TI;
	typedef unsigned uint128_t MODE_TI;

	/// @brief Result type of signed 32-bit multiplication
	typedef struct
	{
		int32_t lo;
		int32_t hi;
	} mul_i32_result_t;

	/// @brief Result type of unsigned 32-bit multiplication
	typedef struct
	{
		uint32_t lo;
		uint32_t hi;
	} mul_u32_result_t;

	/// @brief Result type of signed 32-bit division with remainder
	typedef struct
	{
		int32_t quotient;
		int32_t remainder;
	} divrem_i32_result_t;

	/// @brief Result type of unsigned 32-bit division with remainder
	typedef struct
	{
		uint32_t quotient;
		uint32_t remainder;
	} divrem_u32_result_t;

	/// @brief A 128-bit value containing 16 signed 8-bit values.
	/// Loads and stores of this type must be aligned 16 to bytes. Otherwise a different memory
	/// address is fetched/written.
	typedef struct
	{
		int8_t v[16];
	} ALIGNAS(16) m128i8;

	/// @brief A 128-bit value containing 16 unsigned 8-bit values.
	/// Loads and stores of this type must be aligned 16 to bytes. Otherwise a different memory
	/// address is fetched/written.
	typedef struct
	{
		uint8_t v[16];
	} ALIGNAS(16) m128u8;

	/// @brief A 128-bit value containing 8 signed 16-bit values.
	/// Loads and stores of this type must be aligned 16 to bytes. Otherwise a different memory
	/// address is fetched/written.
	typedef struct
	{
		int16_t v[8];
	} ALIGNAS(16) m128i16;

	/// @brief A 128-bit value containing 8 unsigned 16-bit values.
	/// Loads and stores of this type must be aligned 16 to bytes. Otherwise a different memory
	/// address is fetched/written.
	typedef struct
	{
		uint16_t v[8];
	} ALIGNAS(16) m128u16;

	/// @brief A 128-bit value containing 4 signed 32-bit values.
	/// Loads and stores of this type must be aligned 16 to bytes. Otherwise a different memory
	/// address is fetched/written.
	typedef struct
	{
		int32_t v[4];
	} ALIGNAS(16) m128i32;

	/// @brief A 128-bit value containing 4 unsigned 32-bit values.
	/// Loads and stores of this type must be aligned 16 to bytes. Otherwise a different memory
	/// address is fetched/written.
	typedef struct
	{
		uint32_t v[4];
	} ALIGNAS(16) m128u32;

	/// @brief A 128-bit value containing 2 signed 64-bit values.
	/// Loads and stores of this type must be aligned 16 to bytes. Otherwise a different memory
	/// address is fetched/written.
	/// 
	/// Note: Some functions operate on these values. However, often the EE Core hardware
	/// actually operates on 32-bit values with the upper 32 bits being sign-extension. This is
	/// also noted on the specific functions this applies to.
	typedef struct
	{
		int64_t v[2];
	} ALIGNAS(16) m128i64;

	/// @brief A 128-bit value containing 2 unsigned 64-bit values.
	/// Loads and stores of this type must be aligned 16 to bytes. Otherwise a different memory
	/// address is fetched/written.
	/// 
	/// Note: Some functions operate on these values. However, often the EE Core hardware
	/// actually operates on 32-bit values with the upper 32 bits being sign-extension. This is
	/// also noted on the specific functions this applies to.
	typedef struct
	{
		uint64_t v[2];
	} ALIGNAS(16) m128u64;

	/// @brief A 128-bit value containing 1 signed 128-bit value.
	/// Loads and stores of this type must be aligned 16 to bytes. Otherwise a different memory
	/// address is fetched/written.
	/// 
	/// Note that 'int128_t' can be assigned using a 'qword_t' from <tamtypes.h>. Use this if you
	/// already have a 'qword_t' you want to use, plus a cast.
	typedef struct
	{
		int128_t v[1];
	} ALIGNAS(16) m128i128;

	/// @brief A 128-bit value containing 1 unsigned 128-bit value.
	/// Loads and stores of this type must be aligned 16 to bytes. Otherwise a different memory
	/// address is fetched/written.
	/// 
	/// Note that 'uint128_t' can be assigned using a 'qword_t' from <tamtypes.h>. Use this if
	/// you already have a 'qword_t' you want to use, plus a cast.
	typedef struct
	{
		uint128_t v[1];
	} ALIGNAS(16) m128u128;


	// General Purpose

	/// @brief BREAK : BREAKpoint
	/// 
	/// Issue a breakpoint exception.
	/// 
	/// Has a 20-bit field for custom code, but can't be used here without compiler magic.
	/// (It would need to be encoded in the instruction itself, not an argument).
	FORCEINLINE void breakpoint()
	{
		// implicitly volatile
		asm("break");
	}

	/// @brief PREF : PREFetch
	/// 
	/// Prefetch data from memory to the cache.
	/// 
	/// Can set specific hint value. Only 0 is supported as this value, other values are
	/// reserved. Possible values: [0, 32). Prefer using the function 'prefetch' instead.
	/// 
	/// Does nothing on uncached memory locations.
	/// 
	/// Not allowed on memory locations not present in the TLB. Not recently used pages may not
	/// be present in the TLB, reducing effectiveness. Prefetch may not happen when the memory
	/// bus is used.
	/// @param address The address to prefetch. Must be a variable convertible to 'const void*'.
	/// @param hint The hint value to use. Must be an integer literal.
#define PREF(address, hint)																			\
	asm(												/* implicitly volatile	*/					\
		"pref %c[Hint],%a[Address]"																	\
		:												/* output operands	*/						\
		: [Hint] "n" (hint), [Address] "ZD" (address)	/* input operands	Per GCC documentation,	\
														ZD refers to an address suitable for the	\
														'prefetch' instruction	*/					\
	)

	/// @brief PREF : PREFetch
	/// 
	/// Prefetch data from memory to the cache.
	/// 
	/// Uses hint value 0.
	/// 
	/// Does nothing on uncached memory locations.
	/// 
	/// Not allowed on memory locations not present in the TLB. Not recently used pages may not
	/// be present in the TLB, reducing effectiveness. Prefetch may not happen when the memory
	/// bus is used.
	/// @param address The address to prefetch
	FORCEINLINE void prefetch(const void* address)
	{
		PREF(address, 0);
	}


	// LO/HI registers

	/// @brief MTLO : Move To LO register
	/// 
	/// Set LO0 to 0.
	/// 
	/// This function writes to global state (LO0).
	FORCEINLINE void setzero_lo0()
	{
		asm("mtlo $0");
	}

	/// @brief MTHI : Move To HI register
	/// 
	/// Set HI0 to 0.
	/// 
	/// This function writes to global state (HI0).
	FORCEINLINE void setzero_hi0()
	{
		asm("mthi $0");
	}

	/// @brief MTLO1 : Move To LO1 register
	/// 
	/// Set LO1 to 0.
	/// 
	/// This function writes to global state (LO1).
	FORCEINLINE void setzero_lo1()
	{
		asm("mtlo1 $0");
	}

	/// @brief MTHI1 : Move To HI1 register
	/// 
	/// Set HI1 to 0.
	/// 
	/// This function writes to global state (HI1).
	FORCEINLINE void setzero_hi1()
	{
		asm("mthi1 $0");
	}

	/// @brief MTLO : Move To LO register
	/// MTHI : Move To HI register
	/// 
	/// Set LO0 and HI0 to 0.
	/// 
	/// This function writes to global state (LO0/HI0).
	FORCEINLINE void setzero_lohi0()
	{
		setzero_lo0();
		setzero_hi0();
	}

	/// @brief MTLO1 : Move To LO1 register
	/// MTHI1 : Move To HI1 register
	/// 
	/// Set LO1 and HI1 to 0.
	/// 
	/// This function writes to global state (LO1/HI1).
	FORCEINLINE void setzero_lohi1()
	{
		setzero_lo1();
		setzero_hi1();
	}


	/// @brief MFLO : Move From LO register
	/// 
	/// Get the value of the LO0 register, treated as a sign-extended 32-bit value.
	/// 
	/// This produces a stall if the value is being written to by another instruction and it has
	/// not finished yet. See functions implementing 'MULT', 'MADD' and 'DIV'.
	/// 
	/// This function reads global state (LO0)
	/// @return The value stored in LO0 (LO bits 0..63)
	FORCEINLINE PURE int32_t load_lo0_32()
	{
		int32_t result;

		// I'd like to tell gcc that this asm reads from the LO register but using a variable bound
		// to the "l" constraint sets LO first and then inserts this asm after. As it stands, this
		// needs to be declared volatile to ensure gcc doesn't think the result is always the same.
		asm volatile(
			"mflo %[Result]"
			: [Result] "=r" (result)		// output operands
		);

		return result;
	}

	/// @brief MFLO : Move From LO register
	/// 
	/// Get the value of the LO0 register, treated as a 64-bit value.
	/// 
	/// This produces a stall if the value is being written to by another instruction and it has
	/// not finished yet. See functions implementing 'MULT', 'MADD' and 'DIV'.
	/// 
	/// This function reads global state (LO0)
	/// @return The value stored in LO0 (LO bits 0..63)
	FORCEINLINE PURE int64_t load_lo0_64()
	{
		int64_t result;

		// I'd like to tell gcc that this asm reads from the LO register but using a variable bound
		// to the "l" constraint sets LO first and then inserts this asm after. As it stands, this
		// needs to be declared volatile to ensure gcc doesn't think the result is always the same.
		asm volatile(
			"mflo %[Result]"
			: [Result] "=r" (result)		// output operands
			);

		return result;
	}

	/// @brief MFHI : Move From HI register
	/// 
	/// Get the value of the HI0 register, treated as a sign-extended 32-bit value.
	/// 
	/// This produces a stall if the value is being written to by another instruction and it has
	/// not finished yet. See functions implementing 'MULT', 'MADD' and 'DIV'.
	/// 
	/// This function reads global state (HI0)
	/// @return The value stored in HI0 (HI bits 0..63)
	FORCEINLINE PURE int32_t load_hi0_32()
	{
		int32_t result;

		// I'd like to tell gcc that this asm reads from the HI register but using a variable bound
		// to the "h" constraint sets HI first and then inserts this asm after. As it stands, this
		// needs to be declared volatile to ensure gcc doesn't think the result is always the same.
		asm volatile(
			"mfhi %[Result]"
			: [Result] "=r" (result)		// output operands
		);

		return result;
	}

	/// @brief MFHI : Move From HI register
	/// 
	/// Get the value of the HI0 register, treated as a 64-bit value.
	/// 
	/// This produces a stall if the value is being written to by another instruction and it has
	/// not finished yet. See functions implementing 'MULT', 'MADD' and 'DIV'.
	/// 
	/// This function reads global state (HI0)
	/// @return The value stored in HI0 (HI bits 0..63)
	FORCEINLINE PURE int64_t load_hi0_64()
	{
		int64_t result;

		// I'd like to tell gcc that this asm reads from the HI register but using a variable bound
		// to the "h" constraint sets HI first and then inserts this asm after. As it stands, this
		// needs to be declared volatile to ensure gcc doesn't think the result is always the same.
		asm volatile(
			"mfhi %[Result]"
			: [Result] "=r" (result)		// output operands
			);

		return result;
	}

	/// @brief MFLO1 : Move From LO1 register
	/// 
	/// Get the value of the LO1 register, treated as a sign-extended 32-bit value.
	/// 
	/// This produces a stall if the value is being written to by another instruction and it has
	/// not finished yet. See functions implementing 'MULT', 'MADD' and 'DIV'.
	/// 
	/// This function reads global state (LO1)
	/// @return The value stored in LO1 (LO bits 64..127)
	FORCEINLINE PURE int32_t load_lo1_32()
	{
		int32_t result;

		// I'd like to tell gcc that this asm reads from the LO register but using a variable bound
		// to the "l" constraint sets LO first and then inserts this asm after. As it stands, this
		// needs to be declared volatile to ensure gcc doesn't think the result is always the same.
		asm volatile(
			"mflo1 %[Result]"
			: [Result] "=r" (result)		// output operands
		);

		return result;
	}

	/// @brief MFLO1 : Move From LO1 register
	/// 
	/// Get the value of the LO1 register, treated as a 64-bit value.
	/// 
	/// This produces a stall if the value is being written to by another instruction and it has
	/// not finished yet. See functions implementing 'MULT', 'MADD' and 'DIV'.
	/// 
	/// This function reads global state (LO1)
	/// @return The value stored in LO1 (LO bits 64..127)
	FORCEINLINE PURE int64_t load_lo1_64()
	{
		int64_t result;

		// I'd like to tell gcc that this asm reads from the LO register but using a variable bound
		// to the "l" constraint sets LO first and then inserts this asm after. As it stands, this
		// needs to be declared volatile to ensure gcc doesn't think the result is always the same.
		asm volatile(
			"mflo1 %[Result]"
			: [Result] "=r" (result)		// output operands
			);

		return result;
	}

	/// @brief MFHI1 : Move From HI1 register
	/// 
	/// Get the value of the HI1 register, treated as a sign-extended 32-bit value.
	/// 
	/// This produces a stall if the value is being written to by another instruction and it has
	/// not finished yet. See functions implementing 'MULT', 'MADD' and 'DIV'.
	/// 
	/// This function reads global state (HI1)
	/// @return The value stored in HI1 (HI bits 64..127)
	FORCEINLINE PURE int32_t load_hi1_32()
	{
		int32_t result;

		// I'd like to tell gcc that this asm reads from the HI register but using a variable bound
		// to the "h" constraint sets HI first and then inserts this asm after. As it stands, this
		// needs to be declared volatile to ensure gcc doesn't think the result is always the same.
		asm volatile(
			"mfhi1 %[Result]"
			: [Result] "=r" (result)		// output operands
		);

		return result;
	}

	/// @brief MFHI1 : Move From HI1 register
	/// 
	/// Get the value of the HI1 register, treated as a 64-bit value.
	/// 
	/// This produces a stall if the value is being written to by another instruction and it has
	/// not finished yet. See functions implementing 'MULT', 'MADD' and 'DIV'.
	/// 
	/// This function reads global state (HI1)
	/// @return The value stored in HI1 (HI bits 64..127)
	FORCEINLINE PURE int64_t load_hi1_64()
	{
		int64_t result;

		// I'd like to tell gcc that this asm reads from the HI register but using a variable bound
		// to the "h" constraint sets HI first and then inserts this asm after. As it stands, this
		// needs to be declared volatile to ensure gcc doesn't think the result is always the same.
		asm volatile(
			"mfhi1 %[Result]"
			: [Result] "=r" (result)		// output operands
			);

		return result;
	}

	/// @brief MFLO : Move From LO register
	/// MFHI : Move From HI register
	/// 
	/// Concatenate the values of LO0 and HI0, each interpreted as a 32-bit integer, into a single
	/// 64-bit value.
	/// 
	/// This produces a stall if the value is being written to by another instruction and it has
	/// not finished yet. See functions implementing 'MULT', 'MADD' and 'DIV'.
	/// 
	/// This function reads global state (LO0/HI0)
	/// @return Concatenated LO and HI register values
	FORCEINLINE PURE int64_t load_lohi0_32()
	{
		int64_t result = load_hi0_32();
		result <<= 32;
		result |= load_lo0_32();
		return result;
	}

	/// @brief MFLO1 : Move From LO1 register
	/// MFHI1 : Move From HI1 register
	/// 
	/// Concatenate the values of LO1 and HI1, each interpreted as a 32-bit integer, into a single
	/// 64-bit value.
	/// 
	/// This produces a stall if the value is being written to by another instruction and it has
	/// not finished yet. See functions implementing 'MULT', 'MADD' and 'DIV'.
	/// 
	/// This function reads global state (LO1/HI1)
	/// @return Concatenated LO and HI register values
	FORCEINLINE PURE int64_t load_lohi0_32()
	{
		int64_t result = load_hi0_32();
		result <<= 32;
		result |= load_lo0_32();
		return result;
	}


	/// @brief MTLO : Move To LO register
	/// 
	/// Store a value to the LO0 register.
	/// 
	/// This function writes to global state (LO0).
	/// @param value Value to store in LO0 (LO bits 0..63)
	FORCEINLINE void store_lo0(int64_t value)
	{
		// I'd like to tell gcc that this asm writes to the LO register but using a variable bound
		// to the "l" constraint will not be used, discarding the asm statement. As it stands, this
		// needs to be declared volatile to ensure gcc doesn't think the store is unobserved.
		asm volatile(					// already implicitly volatile due to no output operand
			"mtlo %[Value]"
			:							// output operands
			: [Value] "r" (value)		// input operands
		);
	}

	/// @brief MTHI : Move To HI register
	/// 
	/// Store a value to the HI0 register.
	/// 
	/// This function writes to global state (HI0).
	/// @param value Value to store in HI0 (HI bits 0..63)
	FORCEINLINE void store_hi0(int64_t value)
	{
		// I'd like to tell gcc that this asm writes to the HI register but using a variable bound
		// to the "h" constraint will not be used, discarding the asm statement. As it stands, this
		// needs to be declared volatile to ensure gcc doesn't think the store is unobserved.
		asm volatile(					// already implicitly volatile due to no output operand
			"mthi %[Value]"
			:							// output operands
			: [Value] "r" (value)		// input operands
		);
	}

	/// @brief MTLO1 : Move To LO1 register
	/// 
	/// Store a value to the LO1 register.
	/// 
	/// This function writes to global state (LO1).
	/// @param value Value to store in LO1 (LO bits 64..127)
	FORCEINLINE void store_lo1(int64_t value)
	{
		// I'd like to tell gcc that this asm writes to the LO register but using a variable bound
		// to the "l" constraint will not be used, discarding the asm statement. As it stands, this
		// needs to be declared volatile to ensure gcc doesn't think the store is unobserved.
		asm volatile(					// already implicitly volatile due to no output operand
			"mtlo1 %[Value]"
			:							// output operands
			: [Value] "r" (value)		// input operands
		);
	}

	/// @brief MTHI1 : Move To HI1 register
	/// 
	/// Store a value to the HI1 register.
	/// 
	/// This function writes to global state (HI1).
	/// @param value Value to store in HI1 (HI bits 64..127)
	FORCEINLINE void store_hi1(int64_t value)
	{
		// I'd like to tell gcc that this asm writes to the HI register but using a variable bound
		// to the "h" constraint will not be used, discarding the asm statement. As it stands, this
		// needs to be declared volatile to ensure gcc doesn't think the store is unobserved.
		asm volatile(					// already implicitly volatile due to no output operand
			"mthi1 %[Value]"
			:							// output operands
			: [Value] "r" (value)		// input operands
		);
	}

	
	// Funnel Shift

	/// @brief MFSA : Move From Shift Amount register
	/// 
	/// Get the value of the shift amount register.
	/// 
	/// The value is only used to preserve the register across context switches. It is meaningless
	/// unless written back to the SA register using 'store_sa'. Do not assume the result has any
	/// particular value. Set the shift amount register to a meaningful value by using 'set_sa_8'
	/// or 'set_sa_16'.
	/// 
	/// This function reads global state (SA)
	/// @return The value stored in SA (SA bits 0..63)
	FORCEINLINE PURE uint64_t load_sa()
	{
		uint64_t result;

		// I'd like to tell gcc that this asm reads from the SA register but I don't know of any
		// constraint that fixes a variable to that register, much less how to use it here (see
		// other load_lo/hi functions. As it stands, this needs to be declared volatile to ensure
		// gcc doesn't think the result is always the same.
		asm volatile(
			"mfsa %[Result]"
			: [Result] "=r" (result)		// output operands
			);

		return result;
	}

	/// @brief MTSA : Move To Shift Amount register
	/// 
	/// Store a value to the shift amount register.
	/// 
	/// The value is only used to preserve the register across context switches. Don't use a value
	/// unless read from the SA register using 'load_sa'. Do not assume the result has any
	/// particular value. Set the shift amount register to a meaningful value by using 'set_sa_8'
	/// or 'set_sa_16'.
	/// 
	/// This function writes to global state (SA).
	/// @param value Value to store in SA
	FORCEINLINE void store_sa(uint64_t value)
	{
		// I'd like to tell gcc that this asm writes to the SA register but I don't know of any
		// constraint that fixes a variable to that register, much less how to use it here (see
		// 'load_sa'). As it stands, this needs to be declared volatile to ensure gcc doesn't think
		// the write is unobserved.
		asm volatile(
			"mtsa %[Value]"
			:							// output operands	need to figure out which constraint means SA register, i hope S works...
			: [Value] "r" (value)		// input operands
		);
	}

	/// @brief MTSAB : Move byte count To Shift Amount register (Byte)
	/// 
	/// Set a byte shift count in the shift amount register.
	/// 
	/// The values of 'variable' and 'immediate' are XOR'ed together. The resulting value is the
	/// amount of bytes 'byte_shift_logical_right' will shift by.
	/// 
	/// Allowable values for 'variable' and 'immediate' are [0,15]. Only the lower 4 bits are used,
	/// others are ignored.
	/// 
	/// Use this macro if you require both the variable and constant values usable by the 'mtsab'
	/// instruction. Otherwise, prefer 'MTSAB_IMMEDIATE' if you only have a constant value and
	/// 'set_sa_8' if you only have a variable shift amount.
	/// 
	/// This macro writes to global state (SA).
	/// @param variable The variable byte-amount to set up the shift amount register for. Should be
	/// the name of a variable of type 'unsigned'.
	/// @param immediate The compile time constant byte-amount to set up the shift amount register
	/// for. Should be an integer literal.
#define MTSAB_BOTH(variable, immediate)																\
		asm volatile(									/*	volatile, same reason as 'store_sa'	*/	\
			"mtsab %[Variable],%c[Immediate]"														\
			:															/*	output parameters	*/	\
			: [Variable] "r" (variable), [Immediate] "n" (immediate)	/*	input parameters	*/	\
		)

	/// @brief MTSAB : Move byte count To Shift Amount register (Byte)
	/// 
	/// Set a byte shift count in the shift amount register.
	/// 
	/// The 'immediate' value is the amount of bytes 'byte_shift_logical_right' will shift by.
	/// 
	/// Allowable values for 'immediate' are [0,15]. Only the lower 4 bits are used, others are
	/// ignored.
	/// 
	/// Use this macro if your shift amount is known at compile time. Using this macro does not use
	/// a general purpose register. Otherwise, prefer 'MTSAB_BOTH' or 'set_sa_8'.
	/// 
	/// This macro writes to global state (SA).
	/// @param immediate The compile time constant byte-amount to set up the shift amount register
	/// for. Should be an integer literal.
#define MTSAB_IMMEDIATE(immediate)																	\
		asm volatile(									/*	volatile, same reason as 'store_sa'	*/	\
			"mtsab $0,%c[Immediate]"																\
			:															/*	output parameters	*/	\
			: [Immediate] "n" (immediate)								/*	input parameters	*/	\
		)

	/// @brief MTSAB : Move byte count To Shift Amount register (Byte)
	/// 
	/// Set a byte shift count in the shift amount register.
	/// 
	/// Allowable values for 'byte_amount' are [0,15]. Only the lower 4 bits are used, others are
	/// ignored.
	/// 
	/// Use this function if your byte shift amount is calculated by the program. Otherwise, prefer
	/// 'MTSAB_BOTH' or 'MTSAB_IMMEDIATE'.
	/// 
	/// This function writes to global state (SA).
	/// @param byte_amount The byte-amount to set up the shift amount register for.
	FORCEINLINE void set_sa_8(unsigned byte_amount)
	{
		MTSAB_BOTH(byte_amount, 0);
	}

	/// @brief MTSAH : Move halfword count To Shift Amount register (Halfword)
	/// 
	/// Set a halfword shift count in the shift amount register.
	/// 
	/// The values of 'variable' and 'immediate' are XOR'ed together. The resulting value is the
	/// amount of halfwords 'byte_shift_logical_right' will shift by.
	/// 
	/// Allowable values for 'variable' and 'immediate' are [0,7]. Only the lower 3 bits are used,
	/// others are ignored.
	/// 
	/// Use this macro if you require both the variable and constant values usable by the 'mtsah'
	/// instruction. Otherwise, prefer 'MTSAH_IMMEDIATE' if you only have a constant value and
	/// 'set_sa_16' if you only have a variable shift amount.
	/// 
	/// This macro writes to global state (SA).
	/// @param variable The variable halfword-amount to set up the shift amount register for. Should be
	/// the name of a variable of type 'unsigned'.
	/// @param immediate The compile time constant halfword-amount to set up the shift amount register
	/// for. Should be an integer literal.
#define MTSAH_BOTH(variable, immediate)																\
		asm volatile(									/*	volatile, same reason as 'store_sa'	*/	\
			"mtsah %[Variable],%c[Immediate]"														\
			:															/*	output parameters	*/	\
			: [Variable] "r" (variable), [Immediate] "n" (immediate)	/*	input parameters	*/	\
		)

	/// @brief MTSAH : Move halfword count To Shift Amount register (Halfword)
	/// 
	/// Set a halfword shift count in the shift amount register.
	/// 
	/// The 'immediate' value is the amount of halfwords 'byte_shift_logical_right' will shift by.
	/// 
	/// Allowable values for 'immediate' are [0,7]. Only the lower 3 bits are used, others are
	/// ignored.
	/// 
	/// Use this macro if your shift amount is known at compile time. Using this macro does not use
	/// a general purpose register. Otherwise, prefer 'MTSAH_BOTH' or 'set_sa_16'.
	/// 
	/// This macro writes to global state (SA).
	/// @param immediate The compile time constant halfword-amount to set up the shift amount register
	/// for. Should be an integer literal.
#define MTSAH_IMMEDIATE(immediate)																	\
		asm volatile(									/*	volatile, same reason as 'store_sa'	*/	\
			"mtsah $0,%c[Immediate]"																\
			:															/*	output parameters	*/	\
			: [Immediate] "n" (immediate)								/*	input parameters	*/	\
		)

	/// @brief MTSAH : Move halfword count To Shift Amount register (Halfword)
	/// 
	/// Set a halfword shift count in the shift amount register.
	/// 
	/// Allowable values for 'halfword_amount' are [0,7]. Only the lower 3 bits are used, others are
	/// ignored.
	/// 
	/// Use this function if your byte shift amount is calculated by the program. Otherwise, prefer
	/// 'MTSAH_BOTH' or 'MTSAH_IMMEDIATE'.
	/// 
	/// This function writes to global state (SA).
	/// @param halfword_amount The halfword-amount to set up the shift amount register for.
	FORCEINLINE void set_sa_16(unsigned halfword_amount)
	{
		MTSAH_BOTH(halfword_amount, 0);
	}

	/// @brief QFSRV : Quadword Funnel Shift Right Variable
	/// 
	/// Concatenate ('upper' | 'lower') into a 256-bit value. Then shift the result right by an
	/// amount specified in the Shift Amount Register (SA). Since that register can only be set to
	/// multiples of bytes or halfwords, only bytes can be shifted at a time.
	/// 
	/// Note: If you can decode the value of the SA register you might be able to set specific bit
	/// amounts after all (using 'load_sa' and 'store_sa'). This is undocumented.
	/// 
	/// With specific values in the SA register a left shift and rotations can be achieved. For a
	/// left shift 'x' bytes use 'mtsab' with '16-x' as the argument, then use qfsvr with your
	/// data in the 'upper' position and '0' for 'lower'.
	/// 
	/// This function reads global state (SA).
	/// @param upper The upper 128 bits of the temporary value getting shifted right.
	/// @param lower The lower 128 bits of the temporary value getting shifted right.
	/// @return The lower 128 bits of the temporary value after shifting.
	FORCEINLINE PURE uint128_t byte_shift_logical_right(uint128_t upper, uint128_t lower)
	{
		uint128_t result;

		asm(
			"qfsrv %[Result],%[Upper],%[Lower]"
			: [Result] "=r" (result)					// output operands
			: [Upper] "r" (upper), [Lower] "r" (lower)	// input operands
		);

		return result;
	}

	// maybe put byte shift and rotate helper macros/functions using qfsrv here


	// Asynchronous and Pipeline 1 instructions

	/// @brief MULT : MULTiply word
	/// 
	/// Multiply 32-bit signed integers.
	/// 
	/// This uses the 3-operand version of the instruction, returning the low 32 bits of the 64 bit
	/// result in a register. Both the LO and HI register are still written to. Use this version if
	/// you need the low 32 bits of the 64-bit multiplication result.
	/// 
	/// Integer multiplication happens asynchonously. Reading the result before it is finished will
	/// stall the EE Core. This applies to the LO and HI registers as well as the return value of
	/// this function.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param a First Multiplicand
	/// @param b Second Multiplicand
	/// @return Low 32 bits of the 64 bit multiplication result
	FORCEINLINE int32_t mullo0_i32_start(int32_t a, int32_t b)
	{
		int32_t lo;

		asm volatile(
			"mult %[Lo],%[A],%[B]"
			: [Lo] "=r" (lo)
			: [A] "%r" (a), [B] "r" (b)
		);

		return lo;
	}

	/// @brief MULT : MULTiply word
	/// 
	/// Multiply 32-bit signed integers.
	/// 
	/// This uses the 2-operand version of the instruction, writing only to the LO0 and HI0
	/// register. Use this version if you are only interested in the high 32 bits of the 64-bit
	/// multiplication result.
	/// 
	/// Integer multiplication happens asynchonously. Reading the result before it is finished will
	/// stall the EE Core.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param a First Multiplicand
	/// @param b Second Multiplicand
	FORCEINLINE void mulhi0_i32_start(int32_t a, int32_t b)
	{
		asm volatile(
			"mult %[A],%[B]"
			: 
			: [A] "%r" (a), [B] "r" (b)
		);
	}

	/// @brief MULT : MULTiply word
	/// 
	/// Finish an asynchronous multiplication by reading both low and high 32 bits of the 64-bit
	/// result. Use this function if you started a multiplication using 'mulhi0_i32_start' but do
	/// want the low 32 bit result after all.
	/// 
	/// This function reads global state (LO0/HI0).
	/// @return Struct containing low and high 32 bits of 64-bit multiplication result
	FORCEINLINE PURE mul_i32_result_t mul0_i32_finish()
	{
		mul_i32_result_t result;

		result.lo = load_lo0_32();
		result.hi = load_hi0_32();

		return result;
	}

	/// @brief MULT : MULTiply word
	/// 
	/// Finish an asynchronous multiplication by reading the high 32 bits of the 64-bit result,
	/// reusing the low 32 bits from 'mullo0_i32_start'. Prefer this function if you need both
	/// parts as a struct.
	/// 
	/// This function reads global state (HI0).
	/// @param lo Low 32 bits of multiplication result obtained from 'mullo0_i32_start'
	/// @return Struct containing low and high 32 bits of 64-bit multiplication result
	FORCEINLINE PURE mul_i32_result_t mul0_i32_finish_lo(int32_t lo)
	{
		mul_i32_result_t result;

		result.lo = lo;
		result.hi = load_hi0_32();

		return result;
	}

	/// @brief MULT : MULTiply word
	/// 
	/// Finish an asynchronous multiplication by reading the high 32 bits of the 64-bit result.
	/// Prefer this function if you need both parts of the result separately or just the high part.
	/// 
	/// This function reads global state (HI0).
	/// @return High 32 bits of 64-bit multiplication result
	FORCEINLINE PURE int32_t mulhi0_i32_finish()
	{
		return load_hi0_32();
	}

	/// @brief MULT : MULTiply word
	/// 
	/// Multiply 32-bit signed integers.
	/// 
	/// Helper function to both start and finish a multiplication. This function is known to not
	/// be optimal in regards to throughput. Refer to the documentation of 'mullo0_i32_start'.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param a First Multiplicand
	/// @param b Second Multiplicand
	/// @return Struct containing low and high 32 bits of 64-bit multiplication result
	FORCEINLINE mul_i32_result_t mul0_i32(int32_t a, int32_t b)
	{
		return mul0_i32_finish_lo(mullo0_i32_start(a, b));
	}

	/// @brief MULTU : MULTiply Unsigned word
	/// 
	/// Multiply 32-bit unsigned integers.
	/// 
	/// This uses the 3-operand version of the instruction, returning the low 32 bits of the 64 bit
	/// result in a register. Both the LO and HI register are still written to. Use this version if
	/// you need the low 32 bits of the 64-bit multiplication result.
	/// 
	/// Integer multiplication happens asynchonously. Reading the result before it is finished will
	/// stall the EE Core. This applies to the LO and HI registers as well as the return value of
	/// this function.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param a First Multiplicand
	/// @param b Second Multiplicand
	/// @return Low 32 bits of the 64 bit multiplication result
	FORCEINLINE uint32_t mullo0_u32_start(uint32_t a, uint32_t b)
	{
		uint32_t lo;

		asm volatile(
			"multu %[Lo],%[A],%[B]"
			: [Lo] "=r" (lo)
			: [A] "%r" (a), [B] "r" (b)
		);

		return lo;
	}

	/// @brief MULTU : MULTiply Unsigned word
	/// 
	/// Multiply 32-bit unsigned integers.
	/// 
	/// This uses the 2-operand version of the instruction, writing only to the LO0 and HI0
	/// register. Use this version if you are only interested in the high 32 bits of the 64-bit
	/// multiplication result.
	/// 
	/// Integer multiplication happens asynchonously. Reading the result before it is finished will
	/// stall the EE Core.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param a First Multiplicand
	/// @param b Second Multiplicand
	FORCEINLINE void mulhi0_u32_start(uint32_t a, uint32_t b)
	{
		asm volatile(
			"multu %[A],%[B]"
			:
			: [A] "%r" (a), [B] "r" (b)
		);
	}

	/// @brief MULTU : MULTiply Unsigned word
	/// 
	/// Finish an asynchronous multiplication by reading both low and high 32 bits of the 64-bit
	/// result. Use this function if you started a multiplication using 'mulhi0_u32_start' but do
	/// want the low 32 bit result after all.
	/// 
	/// This function reads global state (LO0/HI0).
	/// @return Struct containing low and high 32 bits of 64-bit multiplication result
	FORCEINLINE PURE mul_u32_result_t mul0_u32_finish()
	{
		mul_u32_result_t result;

		result.lo = load_lo0_32();
		result.hi = load_hi0_32();

		return result;
	}

	/// @brief MULTU : MULTiply Unsigned word
	/// 
	/// Finish an asynchronous multiplication by reading the high 32 bits of the 64-bit result,
	/// reusing the low 32 bits from 'mullo0_u32_start'. Prefer this function if you need both
	/// parts as a struct.
	/// 
	/// This function reads global state (HI0).
	/// @param lo Low 32 bits of multiplication result obtained from 'mullo0_u32_start'
	/// @return Struct containing low and high 32 bits of 64-bit multiplication result
	FORCEINLINE PURE mul_u32_result_t mul0_u32_finish_lo(uint32_t lo)
	{
		mul_u32_result_t result;

		result.lo = lo;
		result.hi = load_hi0_32();

		return result;
	}

	/// @brief MULTU : MULTiply Unsigned word
	/// 
	/// Finish an asynchronous multiplication by reading the high 32 bits of the 64-bit result.
	/// Prefer this function if you need both parts of the result separately or just the high part.
	/// 
	/// This function reads global state (HI0).
	/// @return High 32 bits of 64-bit multiplication result
	FORCEINLINE PURE uint32_t mulhi0_u32_finish()
	{
		return load_hi0_32();
	}

	/// @brief MULTU : MULTiply Unsigned word
	/// 
	/// Multiply 32-bit unsigned integers.
	/// 
	/// Helper function to both start and finish a multiplication. This function is known to not
	/// be optimal in regards to throughput. Refer to the documentation of 'mullo0_u32_start'.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param a First Multiplicand
	/// @param b Second Multiplicand
	/// @return Struct containing low and high 32 bits of 64-bit multiplication result
	FORCEINLINE mul_u32_result_t mul0_u32(uint32_t a, uint32_t b)
	{
		return mul0_u32_finish_lo(mullo0_u32_start(a, b));
	}

	/// @brief MULT1 : MULTiply word pipeline 1
	/// 
	/// Multiply 32-bit signed integers.
	/// 
	/// This uses the 3-operand version of the instruction, returning the low 32 bits of the 64 bit
	/// result in a register. Both the LO and HI register are still written to. Use this version if
	/// you need the low 32 bits of the 64-bit multiplication result.
	/// 
	/// Integer multiplication happens asynchonously. Reading the result before it is finished will
	/// stall the EE Core. This applies to the LO and HI registers as well as the return value of
	/// this function.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param a First Multiplicand
	/// @param b Second Multiplicand
	/// @return Low 32 bits of the 64 bit multiplication result
	FORCEINLINE int32_t mullo1_i32_start(int32_t a, int32_t b)
	{
		int32_t lo;

		asm volatile(
			"mult1 %[Lo],%[A],%[B]"
			: [Lo] "=r" (lo)
			: [A] "%r" (a), [B] "r" (b)
		);

		return lo;
	}

	/// @brief MULT1 : MULTiply word pipeline 1
	/// 
	/// Multiply 32-bit signed integers.
	/// 
	/// This uses the 2-operand version of the instruction, writing only to the LO0 and HI0
	/// register. Use this version if you are only interested in the high 32 bits of the 64-bit
	/// multiplication result.
	/// 
	/// Integer multiplication happens asynchonously. Reading the result before it is finished will
	/// stall the EE Core.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param a First Multiplicand
	/// @param b Second Multiplicand
	FORCEINLINE void mulhi1_i32_start(int32_t a, int32_t b)
	{
		asm volatile(
			"mult1 %[A],%[B]"
			:
			: [A] "%r" (a), [B] "r" (b)
		);
	}

	/// @brief MULT : MULTiply word word pipeline 1
	/// 
	/// Finish an asynchronous multiplication by reading both low and high 32 bits of the 64-bit
	/// result. Use this function if you started a multiplication using 'mulhi0_i32_start' but do
	/// want the low 32 bit result after all.
	/// 
	/// This function reads global state (LO1/HI1).
	/// @return Struct containing low and high 32 bits of 64-bit multiplication result
	FORCEINLINE PURE mul_i32_result_t mul1_i32_finish()
	{
		mul_i32_result_t result;

		result.lo = load_lo1_32();
		result.hi = load_hi1_32();

		return result;
	}

	/// @brief MULT1 : MULTiply word pipeline 1
	/// 
	/// Finish an asynchronous multiplication by reading the high 32 bits of the 64-bit result,
	/// reusing the low 32 bits from 'mullo1_i32_start'. Prefer this function if you need both
	/// parts as a struct.
	/// 
	/// This function reads global state (HI1).
	/// @param lo Low 32 bits of multiplication result obtained from 'mullo1_i32_start'
	/// @return Struct containing low and high 32 bits of 64-bit multiplication result
	FORCEINLINE PURE mul_i32_result_t mul1_i32_finish_lo(int32_t lo)
	{
		mul_i32_result_t result;

		result.lo = lo;
		result.hi = load_hi1_32();

		return result;
	}

	/// @brief MULT1 : MULTiply word pipeline 1
	/// 
	/// Finish an asynchronous multiplication by reading the high 32 bits of the 64-bit result.
	/// Prefer this function if you need both parts of the result separately or just the high part.
	/// 
	/// This function reads global state (HI1).
	/// @return High 32 bits of 64-bit multiplication result
	FORCEINLINE PURE int32_t mulhi1_i32_finish()
	{
		return load_hi1_32();
	}

	/// @brief MULT1 : MULTiply word pipeline 1
	/// 
	/// Multiply 32-bit signed integers.
	/// 
	/// Helper function to both start and finish a multiplication. This function is known to not
	/// be optimal in regards to throughput. Refer to the documentation of 'mullo1_i32_start'.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param a First Multiplicand
	/// @param b Second Multiplicand
	/// @return Struct containing low and high 32 bits of 64-bit multiplication result
	FORCEINLINE mul_i32_result_t mul1_i32(int32_t a, int32_t b)
	{
		return mul1_i32_finish_lo(mullo1_i32_start(a, b));
	}

	/// @brief MULTU1 : MULTiply Unsigned word pipeline 1
	/// 
	/// Multiply 32-bit unsigned integers.
	/// 
	/// This uses the 3-operand version of the instruction, returning the low 32 bits of the 64 bit
	/// result in a register. Both the LO and HI register are still written to. Use this version if
	/// you need the low 32 bits of the 64-bit multiplication result.
	/// 
	/// Integer multiplication happens asynchonously. Reading the result before it is finished will
	/// stall the EE Core. This applies to the LO and HI registers as well as the return value of
	/// this function.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param a First Multiplicand
	/// @param b Second Multiplicand
	/// @return Low 32 bits of the 64 bit multiplication result
	FORCEINLINE uint32_t mullo1_u32_start(uint32_t a, uint32_t b)
	{
		uint32_t lo;

		asm volatile(
			"multu1 %[Lo],%[A],%[B]"
			: [Lo] "=r" (lo)
			: [A] "%r" (a), [B] "r" (b)
		);

		return lo;
	}

	/// @brief MULTU1 : MULTiply Unsigned word pipeline 1
	/// 
	/// Multiply 32-bit unsigned integers.
	/// 
	/// This uses the 2-operand version of the instruction, writing only to the LO0 and HI0
	/// register. Use this version if you are only interested in the high 32 bits of the 64-bit
	/// multiplication result.
	/// 
	/// Integer multiplication happens asynchonously. Reading the result before it is finished will
	/// stall the EE Core.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param a First Multiplicand
	/// @param b Second Multiplicand
	FORCEINLINE void mulhi1_u32_start(uint32_t a, uint32_t b)
	{
		asm volatile(
			"multu1 %[A],%[B]"
			:
			: [A] "%r" (a), [B] "r" (b)
		);
	}

	/// @brief MULTU1 : MULTiply Unsigned word pipeline 1
	/// 
	/// Finish an asynchronous multiplication by reading both low and high 32 bits of the 64-bit
	/// result. Use this function if you started a multiplication using 'mulhi1_u32_start' but do
	/// want the low 32 bit result after all.
	/// 
	/// This function reads global state (LO1/HI1).
	/// @return Struct containing low and high 32 bits of 64-bit multiplication result
	FORCEINLINE PURE mul_u32_result_t mul1_u32_finish()
	{
		mul_u32_result_t result;

		result.lo = load_lo1_32();
		result.hi = load_hi1_32();

		return result;
	}

	/// @brief MULTU1 : MULTiply Unsigned word pipeline 1
	/// 
	/// Finish an asynchronous multiplication by reading the high 32 bits of the 64-bit result,
	/// reusing the low 32 bits from 'mullo1_u32_start'. Prefer this function if you need both
	/// parts as a struct.
	/// 
	/// This function reads global state (HI1).
	/// @param lo Low 32 bits of multiplication result obtained from 'mullo1_u32_start'
	/// @return Struct containing low and high 32 bits of 64-bit multiplication result
	FORCEINLINE PURE mul_u32_result_t mul1_u32_finish_lo(uint32_t lo)
	{
		mul_u32_result_t result;

		result.lo = lo;
		result.hi = load_hi1_32();

		return result;
	}

	/// @brief MULTU1 : MULTiply Unsigned word pipeline 1
	/// 
	/// Finish an asynchronous multiplication by reading the high 32 bits of the 64-bit result.
	/// Prefer this function if you need both parts of the result separately or just the high part.
	/// 
	/// This function reads global state (HI1).
	/// @return High 32 bits of 64-bit multiplication result
	FORCEINLINE PURE uint32_t mulhi1_u32_finish()
	{
		return load_hi1_32();
	}

	/// @brief MULTU1 : MULTiply Unsigned word pipeline 1
	/// 
	/// Multiply 32-bit unsigned integers.
	/// 
	/// Helper function to both start and finish a multiplication. This function is known to not
	/// be optimal in regards to throughput. Refer to the documentation of 'mullo1_u32_start'.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param a First Multiplicand
	/// @param b Second Multiplicand
	/// @return Struct containing low and high 32 bits of 64-bit multiplication result
	FORCEINLINE mul_u32_result_t mul1_u32(uint32_t a, uint32_t b)
	{
		return mul1_u32_finish_lo(mullo1_u32_start(a, b));
	}


	/// @brief MADD : Multiply-ADD word
	/// 
	/// Multiply signed 32-bit values and accumulate into the LO/HI registers.
	/// 
	/// This uses the 3 operand form of the instruction. Use this if you only need the low 32 bits
	/// of the 64-bit accumulator. If you do not need that value use 'fma0_i32' instead.
	/// 
	/// Integer fused-multiply-add is processed asynchronously. Attempting to read from LO0, HI0 or
	/// the return value will cause a stall if this instruction is not finished.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param a First multiplicand
	/// @param b Second multiplicand
	/// @return Low 32 bits (LO0) of the accumulator after the fused-multiply-add operation
	FORCEINLINE int32_t fma0_i32_lo(int32_t a, int32_t b)
	{
		int32_t lo;

		asm volatile(
			"madd %[Lo],%[A],%[B]"
			: [Lo] "=r" (lo)
			: [A] "%r" (a), [B] "r" (b)
		);

		return lo;
	}

	/// @brief MADD : Multiply-ADD word
	/// 
	/// Multiply signed 32-bit values and accumulate into the LO/HI registers.
	/// 
	/// This uses the 2 operand form of the instruction. Use this version if you are doing many
	/// fused-multiply-add with the same accumulator in a loop.
	/// 
	/// Integer fused-multiply-add is processed asynchronously. Attempting to read from LO0 or HI0
	/// will cause a stall if this instruction is not finished.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param a First multiplicand
	/// @param b Second multiplicand
	FORCEINLINE void fma0_i32(int32_t a, int32_t b)
	{
		asm volatile(
			"madd %[A],%[B]"
			: 
			: [A] "%r" (a), [B] "r" (b)
		);
	}

	/// @brief MADD : Multiply-ADD word
	/// 
	/// Multiply signed 32-bit values and accumulate into the LO/HI registers.
	/// 
	/// This uses the 3 operand form of the instruction. The upper part of the accumulator is
	/// loaded and combined with the low part into a 64-bit result and returned.
	/// 
	/// This function is known to be suboptimal in regards to throughput. See documentation of
	/// 'fma0_i32' for details.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param a First multiplicand
	/// @param b Second multiplicand
	/// @return Struct containing bot the low and high 32 bits of the accumulator after the
	/// fused-multiply-add operation.
	FORCEINLINE mul_i32_result_t fma0_i32_finish(int32_t a, int32_t b)
	{
		mul_i32_result_t result;

		asm volatile(
			"madd %[Lo],%[A],%[B]\n\t"
			"mfhi %[Hi]"
			: [Lo] "=r" (result.lo), [Hi] "=r" (result.hi)
			: [A] "%r" (a), [B] "r" (b)
		);

		return result;
	}

	/// @brief MADDU : Multiply-ADD Unsigned word
	/// 
	/// Multiply unsigned 32-bit values and accumulate into the LO/HI registers.
	/// 
	/// This uses the 3 operand form of the instruction. Use this if you only need the low 32 bits
	/// of the 64-bit accumulator. If you do not need that value use 'fma0_u32' instead.
	/// 
	/// Integer fused-multiply-add is processed asynchronously. Attempting to read from LO0, HI0 or
	/// the return value will cause a stall if this instruction is not finished.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param a First multiplicand
	/// @param b Second multiplicand
	/// @return Low 32 bits (LO0) of the accumulator after the fused-multiply-add operation
	FORCEINLINE uint32_t fma0_u32_lo(uint32_t a, uint32_t b)
	{
		uint32_t lo;

		asm volatile(
			"maddu %[Lo],%[A],%[B]"
			: [Lo] "=r" (lo)
			: [A] "%r" (a), [B] "r" (b)
			);

		return lo;
	}

	/// @brief MADDU : Multiply-ADD Unsigned word
	/// 
	/// Multiply unsigned 32-bit values and accumulate into the LO/HI registers.
	/// 
	/// This uses the 2 operand form of the instruction. Use this version if you are doing many
	/// fused-multiply-add with the same accumulator in a loop.
	/// 
	/// Integer fused-multiply-add is processed asynchronously. Attempting to read from LO0 or HI0
	/// will cause a stall if this instruction is not finished.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param a First multiplicand
	/// @param b Second multiplicand
	FORCEINLINE void fma0_u32(uint32_t a, uint32_t b)
	{
		asm volatile(
			"maddu %[A],%[B]"
			:
			: [A] "%r" (a), [B] "r" (b)
		);
	}

	/// @brief MADDU : Multiply-ADD Unsigned word
	/// 
	/// Multiply unsigned 32-bit values and accumulate into the LO/HI registers.
	/// 
	/// This uses the 3 operand form of the instruction. The upper part of the accumulator is
	/// loaded and combined with the low part into a 64-bit result and returned.
	/// 
	/// This function is known to be suboptimal in regards to throughput. See documentation of
	/// 'fma0_u32' for details.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param a First multiplicand
	/// @param b Second multiplicand
	/// @return Struct containing bot the low and high 32 bits of the accumulator after the
	/// fused-multiply-add operation.
	FORCEINLINE mul_u32_result_t fma0_u32_finish(uint32_t a, uint32_t b)
	{
		mul_u32_result_t result;

		asm volatile(
			"maddu %[Lo],%[A],%[B]\n\t"
			"mfhi %[Hi]"
			: [Lo] "=r" (result.lo), [Hi] "=r" (result.hi)
			: [A] "%r" (a), [B] "r" (b)
		);

		return result;
	}

	/// @brief MADD1 : Multiply-ADD word pipeline 1
	/// 
	/// Multiply signed 32-bit values and accumulate into the LO/HI registers.
	/// 
	/// This uses the 3 operand form of the instruction. Use this if you only need the low 32 bits
	/// of the 64-bit accumulator. If you do not need that value use 'fma1_i32' instead.
	/// 
	/// Integer fused-multiply-add is processed asynchronously. Attempting to read from LO1, HI1 or
	/// the return value will cause a stall if this instruction is not finished.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param a First multiplicand
	/// @param b Second multiplicand
	/// @return Low 32 bits (LO1) of the accumulator after the fused-multiply-add operation
	FORCEINLINE int32_t fma1_i32_lo(int32_t a, int32_t b)
	{
		int32_t lo;

		asm volatile(
			"madd1 %[Lo],%[A],%[B]"
			: [Lo] "=r" (lo)
			: [A] "%r" (a), [B] "r" (b)
		);

		return lo;
	}

	/// @brief MADD1 : Multiply-ADD word pipeline 1
	/// 
	/// Multiply signed 32-bit values and accumulate into the LO/HI registers.
	/// 
	/// This uses the 2 operand form of the instruction. Use this version if you are doing many
	/// fused-multiply-add with the same accumulator in a loop.
	/// 
	/// Integer fused-multiply-add is processed asynchronously. Attempting to read from LO1 or HI1
	/// will cause a stall if this instruction is not finished.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param a First multiplicand
	/// @param b Second multiplicand
	FORCEINLINE void fma1_i32(int32_t a, int32_t b)
	{
		asm volatile(
			"madd1 %[A],%[B]"
			:
			: [A] "%r" (a), [B] "r" (b)
		);
	}

	/// @brief MADD1 : Multiply-ADD word pipeline 1
	/// 
	/// Multiply signed 32-bit values and accumulate into the LO/HI registers.
	/// 
	/// This uses the 3 operand form of the instruction. The upper part of the accumulator is
	/// loaded and combined with the low part into a 64-bit result and returned.
	/// 
	/// This function is known to be suboptimal in regards to throughput. See documentation of
	/// 'fma1_i32' for details.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param a First multiplicand
	/// @param b Second multiplicand
	/// @return Struct containing bot the low and high 32 bits of the accumulator after the
	/// fused-multiply-add operation.
	FORCEINLINE mul_i32_result_t fma1_i32_finish(int32_t a, int32_t b)
	{
		mul_i32_result_t result;

		asm volatile(
			"madd1 %[Lo],%[A],%[B]\n\t"
			"mfhi1 %[Hi]"
			: [Lo] "=r" (result.lo), [Hi] "=r" (result.hi)
			: [A] "%r" (a), [B] "r" (b)
			);

		return result;
	}

	/// @brief MADDU1 : Multiply-ADD Unsigned word pipeline 1
	/// 
	/// Multiply unsigned 32-bit values and accumulate into the LO/HI registers.
	/// 
	/// This uses the 3 operand form of the instruction. Use this if you only need the low 32 bits
	/// of the 64-bit accumulator. If you do not need that value use 'fma1_u32' instead.
	/// 
	/// Integer fused-multiply-add is processed asynchronously. Attempting to read from LO1, HI1 or
	/// the return value will cause a stall if this instruction is not finished.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param a First multiplicand
	/// @param b Second multiplicand
	/// @return Low 32 bits (LO1) of the accumulator after the fused-multiply-add operation
	FORCEINLINE uint32_t fma1_u32_lo(uint32_t a, uint32_t b)
	{
		uint32_t lo;

		asm volatile(
			"maddu1 %[Lo],%[A],%[B]"
			: [Lo] "=r" (lo)
			: [A] "%r" (a), [B] "r" (b)
		);

		return lo;
	}

	/// @brief MADDU1 : Multiply-ADD Unsigned word pipeline 1
	/// 
	/// Multiply unsigned 32-bit values and accumulate into the LO/HI registers.
	/// 
	/// This uses the 2 operand form of the instruction. Use this version if you are doing many
	/// fused-multiply-add with the same accumulator in a loop.
	/// 
	/// Integer fused-multiply-add is processed asynchronously. Attempting to read from LO1 or HI1
	/// will cause a stall if this instruction is not finished.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param a First multiplicand
	/// @param b Second multiplicand
	FORCEINLINE void fma1_u32(uint32_t a, uint32_t b)
	{
		asm volatile(
			"maddu1 %[A],%[B]"
			:
			: [A] "%r" (a), [B] "r" (b)
		);
	}

	/// @brief MADDU1 : Multiply-ADD Unsigned word pipeline 1
	/// 
	/// Multiply unsigned 32-bit values and accumulate into the LO/HI registers.
	/// 
	/// This uses the 3 operand form of the instruction. The upper part of the accumulator is
	/// loaded and combined with the low part into a 64-bit result and returned.
	/// 
	/// This function is known to be suboptimal in regards to throughput. See documentation of
	/// 'fma1_u32' for details.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param a First multiplicand
	/// @param b Second multiplicand
	/// @return Struct containing bot the low and high 32 bits of the accumulator after the
	/// fused-multiply-add operation.
	FORCEINLINE mul_u32_result_t fma1_u32_finish(uint32_t a, uint32_t b)
	{
		mul_u32_result_t result;

		asm volatile(
			"maddu1 %[Lo],%[A],%[B]\n\t"
			"mfhi1 %[Hi]"
			: [Lo] "=r" (result.lo), [Hi] "=r" (result.hi)
			: [A] "%r" (a), [B] "r" (b)
		);

		return result;
	}


	/// @brief DIV : DIVide word
	/// 
	/// Start a division. The results will be stored in the LO0/HI0 registers.
	/// 
	/// Division by 0 results in undefined values.
	/// No exception occurs.
	/// 
	/// Dividing INT_MIN by -1 results in INT_MIN as the quotient and 0 as the remainder.
	/// No overflow exception will occur.
	/// 
	/// Result signedness table:
	///		Dividend	|	Divisor	|	Quotient (LO0)	|	Remainder (HI0)
	///		+			|	+		|	+				|	+
	///		+			|	-		|	-				|	+
	///		-			|	+		|	-				|	-
	///		-			|	-		|	+				|	-
	/// 
	/// As integer division is done asynchronously on the EE Core you should issue the division
	/// instruction before checking for erroneous inputs like div-by-0 to improve throughput.
	/// Reading from the result registers (LO0/HI0) will cause a stall if the result is not
	/// ready at that point in time.
	/// The Undefined Behaviour definition of C/C++ does not allow a div-by-0 check to happen
	/// in regular code. The code snippet:
	/// 
	///		int foo(int x, int y) {
	///			int z = x / y;
	///			if (y == 0) {
	///				return 0;
	///			}
	///			return z;
	///		}
	/// 
	/// would be reduced to 'return x / y;' in all cases. This is because div-by-0 is
	/// Undefined Behaviour, so it can be assummed to not occur. This means y is never 0 at the
	/// point of the division and therefor can never be 0 after, which is when it is checked.
	/// Use this function to get around this limitation. Query the result of the operation using
	/// 'divrem0_i32_finish'.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param dividend Number to divide
	/// @param divisor Number to divide by
	FORCEINLINE void divrem0_i32_start(int32_t dividend, int32_t divisor)
	{
		asm volatile(
			"div %[Dividend],%[Divisor]"							// 
			:														// output operands
			: [Dividend] "r" (dividend), [Divisor] "r" (divisor)	// input operands
		);
	}

	/// @brief DIV : DIVide word
	/// 
	/// Finish the division. Results will be read from LO0/HI0 registers.
	/// 
	/// See decumentation of 'divrem0_i32_start'.
	/// 
	/// This function reads global state (LO0/HI0).
	/// @return struct containing quotient and remainder of preceding division.
	FORCEINLINE PURE divrem_i32_result_t divrem0_i32_finish()
	{
		divrem_i32_result_t result;

		result.quotient = load_lo0_32();
		result.remainder = load_hi0_32();

		return result;
	}

	/// @brief DIV : DIVide word
	/// 
	/// Convenience function doing both starting and finishing a division.
	/// 
	/// Division by 0 results in undefined results.
	/// No exception occurs.
	/// 
	/// Dividing INT_MIN by -1 results in INT_MIN as the quotient and 0 as the remainder.
	/// No overflow exception will occur.
	/// 
	/// Result signedness table:
	///		Dividend	|	Divisor	|	Quotient (LO0)	|	Remainder (HI0)
	///		+			|	+		|	+				|	+
	///		+			|	-		|	-				|	+
	///		-			|	+		|	-				|	-
	///		-			|	-		|	+				|	-
	/// 
	/// This function is not optimal in regards to throughput. Refer to the documentation of
	/// 'divrem0_i32_start'.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param dividend Number to divide
	/// @param divisor Number to divide by
	/// @return struct containing quotient and remainder of division.
	FORCEINLINE divrem_i32_result_t divrem0_i32(int32_t dividend, int32_t divisor)
	{
		divrem0_i32_start(dividend, divisor);
		return divrem0_i32_finish();
	}

	/// @brief DIVU : DIVide Unsigned word
	/// 
	/// Start a division. The results will be stored in the LO0/HI0 registers.
	/// 
	/// Division by 0 results in undefined values.
	/// No exception occurs.
	/// 
	/// As integer division is done asynchronously on the EE Core you should issue the division
	/// instruction before checking for erroneous inputs like div-by-0 to improve throughput.
	/// Reading from the result registers (LO0/HI0) will cause a stall if the result is not
	/// ready at that point in time.
	/// The Undefined Behaviour definition of C/C++ does not allow a div-by-0 check to happen
	/// in regular code. The code snippet:
	/// 
	///		unsigned foo(unsigned x, unsigned y) {
	///			unsigned z = x / y;
	///			if (y == 0) {
	///				return 0;
	///			}
	///			return z;
	///		}
	/// 
	/// would be reduced to 'return x / y;' in all cases. This is because div-by-0 is
	/// Undefined Behaviour, so it can be assummed to not occur. This means y is never 0 at the
	/// point of the division and therefor can never be 0 after, which is when it is checked.
	/// Use this function to get around this limitation. Query the result of the operation using
	/// 'divrem0_u32_finish'.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param dividend Number to divide
	/// @param divisor Number to divide by
	FORCEINLINE void divrem0_u32_start(uint32_t dividend, uint32_t divisor)
	{
		asm volatile(
			"divu %[Dividend],%[Divisor]"							// 
			:														// output operands
			: [Dividend] "r" (dividend), [Divisor] "r" (divisor)	// input operands
		);
	}

	/// @brief DIVU : DIVide Unsigned word
	/// 
	/// Finish the division. Results will be read from LO0/HI0 registers.
	/// 
	/// See decumentation of 'divrem0_u32_start'.
	/// 
	/// This function reads global state (LO0/HI0).
	/// @return struct containing quotient and remainder of preceding division.
	FORCEINLINE PURE divrem_u32_result_t divrem0_u32_finish()
	{
		divrem_u32_result_t result;

		result.quotient = load_lo0_32();
		result.remainder = load_hi0_32();

		return result;
	}

	/// @brief DIVU : DIVide Unsigned word
	/// 
	/// Convenience function doing both starting and finishing a division.
	/// 
	/// Division by 0 results in undefined results.
	/// No exception occurs.
	/// 
	/// This function is not optimal in regards to throughput. Refer to the documentation of
	/// 'divrem0_u32_start'.
	/// 
	/// This function writes to global state (LO0/HI0).
	/// @param dividend Number to divide
	/// @param divisor Number to divide by
	/// @return struct containing quotient and remainder of division.
	FORCEINLINE divrem_u32_result_t divrem0_u32(uint32_t dividend, uint32_t divisor)
	{
		divrem0_u32_start(dividend, divisor);
		return divrem0_u32_finish();
	}

	/// @brief DIV1 : Divide Word Pipeline 1
	/// 
	/// Start a division. The results will be stored in the LO1/HI1 registers.
	/// 
	/// Division by 0 results in undefined results.
	/// No exception occurs.
	/// 
	/// Dividing INT_MIN by -1 results in INT_MIN as the quotient and 0 as the remainder.
	/// No overflow exception will occur.
	/// 
	/// Result signedness table:
	///		Dividend	|	Divisor	|	Quotient (LO1)	|	Remainder (HI1)
	///		+			|	+		|	+				|	+
	///		+			|	-		|	-				|	+
	///		-			|	+		|	-				|	-
	///		-			|	-		|	+				|	-
	/// 
	/// As integer division is done asynchronously on the EE Core you should issue the division
	/// instruction before checking for erroneous inputs like div-by-0 to improve throughput.
	/// Reading from the result registers (LO1/HI1) will cause a stall if the result is not
	/// ready at that point in time.
	/// The Undefined Behaviour definition of C/C++ does not allow a div-by-0 check to happen
	/// in regular code. The code snippet:
	/// 
	///		int foo(int x, int y) {
	///			int z = x / y;
	///			if (y == 0) {
	///				return 0;
	///			}
	///			return z;
	///		}
	/// 
	/// would be reduced to 'return x / y;' in all cases. This is because div-by-0 is
	/// Undefined Behaviour, so it can be assummed to not occur. This means y is never 0 at the
	/// point of the division and therefor can never be 0 after, which is when it is checked.
	/// Use this function to get around this limitation. Query the result of the operation using
	/// 'divrem1_i32_finish'.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param dividend Number to divide
	/// @param divisor Number to divide by
	FORCEINLINE void divrem1_i32_start(int32_t dividend, int32_t divisor)
	{
		asm volatile(
			"div1 %[Dividend],%[Divisor]"							// 
			:														// output operands
			: [Dividend] "r" (dividend), [Divisor] "r" (divisor)	// input operands
		);
	}

	/// @brief DIV1 : Divide Word Pipeline 1
	/// 
	/// Finish the division. Results will be read from LO1/HI1 registers.
	/// 
	/// See decumentation of 'divrem1_i32_start'.
	/// 
	/// This function reads global state (LO1/HI1).
	/// @return struct containing quotient and remainder of preceding division.
	FORCEINLINE PURE divrem_i32_result_t divrem1_i32_finish()
	{
		divrem_i32_result_t result;

		result.quotient = load_lo1_32();
		result.remainder = load_hi1_32();

		return result;
	}

	/// @brief DIV1 : Divide Word Pipeline 1
	/// 
	/// Convenience function doing both starting and finishing a pipeline 1 division.
	/// 
	/// Division by 0 results in undefined results.
	/// No exception occurs.
	/// 
	/// Dividing INT_MIN by -1 results in INT_MIN as the quotient and 0 as the remainder.
	/// No overflow exception will occur.
	/// 
	/// Result signedness table:
	///		Dividend	|	Divisor	|	Quotient (LO1)	|	Remainder (HI1)
	///		+			|	+		|	+				|	+
	///		+			|	-		|	-				|	+
	///		-			|	+		|	-				|	-
	///		-			|	-		|	+				|	-
	/// 
	/// This function is not optimal in regards to throughput. Refer to the documentation of
	/// 'divrem1_i32_start'.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param dividend Number to divide
	/// @param divisor Number to divide by
	/// @return struct containing quotient and remainder of division.
	FORCEINLINE divrem_i32_result_t div1_i32(int32_t dividend, int32_t divisor)
	{
		divrem1_i32_start(dividend, divisor);
		return divrem1_i32_finish();
	}

	/// @brief DIVU1 : DIVide Unsigned word pipeline 1
	/// 
	/// Start a division. The results will be stored in the LO1/HI1 registers.
	/// 
	/// Division by 0 results in undefined results.
	/// No exception occurs.
	/// 
	/// As integer division is done asynchronously on the EE Core you should issue the division
	/// instruction before checking for erroneous inputs like div-by-0 to improve throughput.
	/// Reading from the result registers (LO1/HI1) will cause a stall if the result is not
	/// ready at that point in time.
	/// The Undefined Behaviour definition of C/C++ does not allow a div-by-0 check to happen
	/// in regular code. The code snippet:
	/// 
	///		unsigned foo(unsigned x, unsigned y) {
	///			unsigned z = x / y;
	///			if (y == 0) {
	///				return 0;
	///			}
	///			return z;
	///		}
	/// 
	/// would be reduced to 'return x / y;' in all cases. This is because div-by-0 is
	/// Undefined Behaviour, so it can be assummed to not occur. This means y is never 0 at the
	/// point of the division and therefor can never be 0 after, which is when it is checked.
	/// Use this function to get around this limitation. Query the result of the operation using
	/// 'divrem1_i32_finish'.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param dividend Number to divide
	/// @param divisor Number to divide by
	FORCEINLINE void divrem1_u32_start(uint32_t dividend, uint32_t divisor)
	{
		asm volatile(
			"divu1 %[Dividend],%[Divisor]"							// 
			:														// output operands
			: [Dividend] "r" (dividend), [Divisor] "r" (divisor)	// input operands
		);
	}

	/// @brief DIVU1 : DIVide Unsigned word pipeline 1
	/// 
	/// Finish the division. Results will be read from LO1/HI1 registers.
	/// 
	/// See decumentation of 'divrem1_i32_start'.
	/// 
	/// This function reads global state (LO1/HI1).
	/// @return struct containing quotient and remainder of preceding division.
	FORCEINLINE PURE divrem_u32_result_t divrem1_u32_finish()
	{
		divrem_u32_result_t result;

		result.quotient = load_lo1_32();
		result.remainder = load_hi1_32();

		return result;
	}

	/// @brief DIVU1 : DIVide Unsigned word pipeline 1
	/// 
	/// Convenience function doing both starting and finishing a pipeline 1 division.
	/// 
	/// Division by 0 results in undefined results.
	/// No exception occurs.
	/// 
	/// This function is not optimal in regards to throughput. Refer to the documentation of
	/// 'divrem1_u32_start'.
	/// 
	/// This function writes to global state (LO1/HI1).
	/// @param dividend Number to divide
	/// @param divisor Number to divide by
	/// @return struct containing quotient and remainder of division.
	FORCEINLINE divrem_u32_result_t div1_u32(uint32_t dividend, uint32_t divisor)
	{
		divrem1_u32_start(dividend, divisor);
		return divrem1_u32_finish();
	}


	// Multimedia instructions

	/// @brief Create a 128-bit packed integer of all 0.
	/// @return 128-bit packed integer containing 16 8-bit signed integers
	FORCEINLINE CONST m128i8 mm_setzero_epi8()
	{
		m128i8 result;

		asm(
			"move %[Result],$0"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief Create a 128-bit packed integer of all 0.
	/// @return 128-bit packed integer containing 16 8-bit unsigned integers
	FORCEINLINE CONST m128u8 mm_setzero_epu8()
	{
		m128u8 result;

		asm(
			"move %[Result],$0"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief Create a 128-bit packed integer of all 0.
	/// @return 128-bit packed integer containing 8 16-bit signed integers
	FORCEINLINE CONST m128i16 mm_setzero_epi16()
	{
		m128i16 result;

		asm(
			"move %[Result],$0"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief Create a 128-bit packed integer of all 0.
	/// @return 128-bit packed integer containing 8 16-bit unsigned integers
	FORCEINLINE CONST m128u16 mm_setzero_epu16()
	{
		m128u16 result;

		asm(
			"move %[Result],$0"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief Create a 128-bit packed integer of all 0.
	/// @return 128-bit packed integer containing 4 32-bit signed integers
	FORCEINLINE CONST m128i32 mm_setzero_epi32()
	{
		m128i32 result;

		asm(
			"move %[Result],$0"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief Create a 128-bit packed integer of all 0.
	/// @return 128-bit packed integer containing 4 32-bit unsigned integers
	FORCEINLINE CONST m128u32 mm_setzero_epu32()
	{
		m128u32 result;

		asm(
			"move %[Result],$0"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief Create a 128-bit packed integer of all 0.
	/// @return 128-bit packed integer containing 2 64-bit signed integers
	FORCEINLINE CONST m128i64 mm_setzero_epi64()
	{
		m128i64 result;

		asm(
			"move %[Result],$0"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief Create a 128-bit packed integer of all 0.
	/// @return 128-bit packed integer containing 2 64-bit unsigned integers
	FORCEINLINE CONST m128u64 mm_setzero_epu64()
	{
		m128u64 result;

		asm(
			"move %[Result],$0"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief Create a 128-bit packed integer of all 0.
	/// @return 128-bit packed integer containing 1 128-bit signed integers
	FORCEINLINE CONST m128i128 mm_setzero_epi128()
	{
		m128i128 result;

		asm(
			"move %[Result],$0"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief Create a 128-bit packed integer of all 0.
	/// @return 128-bit packed integer containing 1 128-bit unsigned integers
	FORCEINLINE CONST m128u128 mm_setzero_epu128()
	{
		m128u128 result;

		asm(
			"move %[Result],$0"
			: [Result] "=r" (result)
		);

		return result;
	}


	/// @brief LQ : Load Quadword
	/// 
	/// Load a 128-bit value from memory.
	/// 
	/// This is a helper macro. Prefer using the specific functions instead.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, loading unintended values.
	/// 
	/// This macro reads global state (*'address')
	/// @param result Variable to store result to. Should be the name of a 128-bit variable.
	/// @param address Address to load from. Must be a variable convertible to 'const void*'.
#define LQ(result, address)																			\
	asm(																							\
		"lq %[Result],%[Address]"																	\
		: [Result] "=r" (result)						/*	output operands	*/						\
		: [Address] "o" (*(const char (*)[16]) address)	/*	input operands	Tell GCC that this is	\
																reading 16 bytes from *address	*/	\
	)

	/// @brief LQ : Load Quadword
	/// 
	/// Load 16 packed signed 8-bit integers from memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, loading unintended values.
	/// 
	/// This function reads global state (*'p')
	/// @param p Memory location to load integer data from.
	/// @return Packed integer data loaded from given memory location.
	FORCEINLINE UNSEQUENCED m128i8 mm_load_epi8(const m128i8* p)
	{
		m128i8 result;

		LQ(result, p);

		return result;
	}

	/// @brief LQ : Load Quadword
	/// 
	/// Load 16 packed unsigned 8-bit integers from memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, loading unintended values.
	/// 
	/// This function reads global state (*'p')
	/// @param p Memory location to load integer data from.
	/// @return Packed integer data loaded from given memory location.
	FORCEINLINE UNSEQUENCED m128u8 mm_load_epu8(const m128u8* p)
	{
		m128u8 result;

		LQ(result, p);

		return result;
	}

	/// @brief LQ : Load Quadword
	/// 
	/// Load 8 packed signed 16-bit integers from memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, loading unintended values.
	/// 
	/// This function reads global state (*'p')
	/// @param p Memory location to load integer data from.
	/// @return Packed integer data loaded from given memory location.
	FORCEINLINE UNSEQUENCED m128i16 mm_load_epi16(const m128i16* p)
	{
		m128i16 result;

		LQ(result, p);

		return result;
	}

	/// @brief LQ : Load Quadword
	/// 
	/// Load 8 packed unsigned 16-bit integers from memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, loading unintended values.
	/// 
	/// This function reads global state (*'p')
	/// @param p Memory location to load integer data from.
	/// @return Packed integer data loaded from given memory location.
	FORCEINLINE UNSEQUENCED m128u16 mm_load_epu16(const m128u16* p)
	{
		m128u16 result;

		LQ(result, p);

		return result;
	}

	/// @brief LQ : Load Quadword
	/// 
	/// Load 4 packed signed 32-bit integers from memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, loading unintended values.
	/// 
	/// This function reads global state (*'p')
	/// @param p Memory location to load integer data from.
	/// @return Packed integer data loaded from given memory location.
	FORCEINLINE UNSEQUENCED m128i32 mm_load_epi32(const m128i32* p)
	{
		m128i32 result;

		LQ(result, p);

		return result;
	}

	/// @brief LQ : Load Quadword
	/// 
	/// Load 4 packed unsigned 32-bit integers from memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, loading unintended values.
	/// 
	/// This function reads global state (*'p')
	/// @param p Memory location to load integer data from.
	/// @return Packed integer data loaded from given memory location.
	FORCEINLINE UNSEQUENCED m128u32 mm_load_epu32(const m128u32* p)
	{
		m128u32 result;

		LQ(result, p);

		return result;
	}

	/// @brief LQ : Load Quadword
	/// 
	/// Load 2 packed signed 64-bit integers from memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, loading unintended values.
	/// 
	/// This function reads global state (*'p')
	/// @param p Memory location to load integer data from.
	/// @return Packed integer data loaded from given memory location.
	FORCEINLINE UNSEQUENCED m128i64 mm_load_epi64(const m128i64* p)
	{
		m128i64 result;

		LQ(result, p);

		return result;
	}

	/// @brief LQ : Load Quadword
	/// 
	/// Load 2 packed unsigned 64-bit integers from memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, loading unintended values.
	/// 
	/// This function reads global state (*'p')
	/// @param p Memory location to load integer data from.
	/// @return Packed integer data loaded from given memory location.
	FORCEINLINE UNSEQUENCED m128u64 mm_load_epu64(const m128u64* p)
	{
		m128u64 result;

		LQ(result, p);

		return result;
	}

	/// @brief LQ : Load Quadword
	/// 
	/// Load 1 signed 128-bit integer from memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, loading unintended values.
	/// 
	/// This function reads global state (*'p')
	/// @param p Memory location to load integer data from.
	/// @return Packed integer data loaded from given memory location.
	FORCEINLINE UNSEQUENCED m128i128 mm_load_epi128(const m128i128* p)
	{
		m128i128 result;

		LQ(result, p);

		return result;
	}

	/// @brief LQ : Load Quadword
	/// 
	/// Load 1 unsigned 128-bit integer from memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, loading unintended values.
	/// 
	/// This function reads global state (*'p')
	/// @param p Memory location to load integer data from.
	/// @return Packed integer data loaded from given memory location.
	FORCEINLINE UNSEQUENCED m128u128 mm_load_epu128(const m128u128* p)
	{
		m128u128 result;

		LQ(result, p);

		return result;
	}


	/// @brief SQ : Store Quadword
	/// 
	/// Store a 128-bit value to memory.
	/// 
	/// This is a helper macro. Prefer using the specific functions instead.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, storing to an unintended address.
	/// 
	/// This macro writes to global state (*'address')
	/// @param address Address to store to. Must be a variable convertible to 'void*'.
	/// @param value Variable to write. Should be the name of a 128-bit variable.
#define SQ(address, value)																			\
	asm(																							\
		"sq %[Value],%[Address]"																	\
		: [Address] "=o" (*(char (*)[16]) address)		/*	output operands	Tell GCC that this is	\
															writing 16 bytes to *address	*/		\
		: [Value] "r" (value)							/*	input operands	*/						\
	)

	/// @brief SQ : Store Quadword
	/// 
	/// Store 16 packed signed 8-bit integers to memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, storing to an unintended address.
	/// 
	/// This function writes to global state (*'address')
	/// @param p Address to store to.
	/// @return Packed integer data to store to the memory location.
	FORCEINLINE UNSEQUENCED void mm_store_epi8(m128i8* p, m128i8 value)
	{
		SQ(p, value);
	}

	/// @brief SQ : Store Quadword
	/// 
	/// Store 16 packed unsigned 8-bit integers to memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, storing to an unintended address.
	/// 
	/// This function writes to global state (*'address')
	/// @param p Address to store to.
	/// @return Packed integer data to store to the memory location.
	FORCEINLINE UNSEQUENCED void mm_store_epu8(m128u8* p, m128u8 value)
	{
		SQ(p, value);
	}

	/// @brief SQ : Store Quadword
	/// 
	/// Store 8 packed signed 16-bit integers to memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, storing to an unintended address.
	/// 
	/// This function writes to global state (*'address')
	/// @param p Address to store to.
	/// @return Packed integer data to store to the memory location.
	FORCEINLINE UNSEQUENCED void mm_store_epi16(m128i16* p, m128i16 value)
	{
		SQ(p, value);
	}

	/// @brief SQ : Store Quadword
	/// 
	/// Store 8 packed unsigned 16-bit integers to memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, storing to an unintended address.
	/// 
	/// This function writes to global state (*'address')
	/// @param p Address to store to.
	/// @return Packed integer data to store to the memory location.
	FORCEINLINE UNSEQUENCED void mm_store_epu16(m128u16* p, m128u16 value)
	{
		SQ(p, value);
	}

	/// @brief SQ : Store Quadword
	/// 
	/// Store 4 packed signed 32-bit integers to memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, storing to an unintended address.
	/// 
	/// This function writes to global state (*'address')
	/// @param p Address to store to.
	/// @return Packed integer data to store to the memory location.
	FORCEINLINE UNSEQUENCED void mm_store_epi32(m128i8* p, m128i32 value)
	{
		SQ(p, value);
	}

	/// @brief SQ : Store Quadword
	/// 
	/// Store 4 packed unsigned 32-bit integers to memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, storing to an unintended address.
	/// 
	/// This function writes to global state (*'address')
	/// @param p Address to store to.
	/// @return Packed integer data to store to the memory location.
	FORCEINLINE UNSEQUENCED void mm_store_epu32(m128u32* p, m128u32 value)
	{
		SQ(p, value);
	}

	/// @brief SQ : Store Quadword
	/// 
	/// Store 2 packed signed 64-bit integers to memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, storing to an unintended address.
	/// 
	/// This function writes to global state (*'address')
	/// @param p Address to store to.
	/// @return Packed integer data to store to the memory location.
	FORCEINLINE UNSEQUENCED void mm_store_epi64(m128i64* p, m128i64 value)
	{
		SQ(p, value);
	}

	/// @brief SQ : Store Quadword
	/// 
	/// Store 2 packed unsigned 64-bit integers to memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, storing to an unintended address.
	/// 
	/// This function writes to global state (*'address')
	/// @param p Address to store to.
	/// @return Packed integer data to store to the memory location.
	FORCEINLINE UNSEQUENCED void mm_store_epu64(m128u64* p, m128u64 value)
	{
		SQ(p, value);
	}

	/// @brief SQ : Store Quadword
	/// 
	/// Store 1 packed signed 128-bit integers to memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, storing to an unintended address.
	/// 
	/// This function writes to global state (*'address')
	/// @param p Address to store to.
	/// @return Packed integer data to store to the memory location.
	FORCEINLINE UNSEQUENCED void mm_store_epi128(m128i128* p, m128i128 value)
	{
		SQ(p, value);
	}

	/// @brief SQ : Store Quadword
	/// 
	/// Store 1 packed unsigned 128-bit integers to memory.
	/// 
	/// The memory location must aligned on a 16-byte boundary. Otherwise the next 16-byte boundary
	/// below the given memory location is used instead, storing to an unintended address.
	/// 
	/// This function writes to global state (*'address')
	/// @param p Address to store to.
	/// @return Packed integer data to store to the memory location.
	FORCEINLINE UNSEQUENCED void mm_store_epu128(m128u128* p, m128u128 value)
	{
		SQ(p, value);
	}


	/// @brief Set a 128-bit packed integer type with given values.
	/// 
	/// This is a convenience function and corresponds to a multitude of instructions.
	/// 
	/// Ordering of the arguments is where they would be as if loaded from memory in order of their
	/// name. So the code:
	/// 
	///		int8_t arr[16] = { r0, r1, r2, ... , r15 };
	///		result = mm_load_epi8(arr);
	/// 
	/// would result in the same initialized packed integer type. (Omitting alignment/casting).
	/// @param r15 Signed 8-bit value to put in the highest position of the result
	/// @param r14 
	/// @param r13 
	/// @param r12 
	/// @param r11 
	/// @param r10 
	/// @param r9 
	/// @param r8 
	/// @param r7 
	/// @param r6 
	/// @param r5 
	/// @param r4 
	/// @param r3 
	/// @param r2 
	/// @param r1 
	/// @param r0 Signed 8-bit value to put in the lowest position of the result
	/// @return Packed integer type initialized to the given values
	FORCEINLINE CONST m128i8 mm_set_epi8(int8_t r15, int8_t r14, int8_t r13, int8_t r12, int8_t r11, int8_t r10, int8_t r9, int8_t r8,
										 int8_t r7, int8_t r6, int8_t r5, int8_t r4, int8_t r3, int8_t r2, int8_t r1, int8_t r0)
	{
		m128i8 result;

		result.v[0] = r15;
		result.v[1] = r14;
		result.v[2] = r13;
		result.v[3] = r12;
		result.v[4] = r11;
		result.v[5] = r10;
		result.v[6] = r9;
		result.v[7] = r8;
		result.v[8] = r7;
		result.v[9] = r6;
		result.v[10] = r5;
		result.v[11] = r4;
		result.v[12] = r3;
		result.v[13] = r2;
		result.v[14] = r1;
		result.v[15] = r0;

		return result;
	}

	/// @brief Set a 128-bit packed integer type with given values.
	/// 
	/// This is a convenience function and corresponds to a multitude of instructions.
	/// 
	/// Ordering of the arguments is where they would be as if loaded from memory in order of their
	/// name. So the code:
	/// 
	///		uint8_t arr[16] = { r0, r1, r2, ... , r15 };
	///		result = mm_load_epu8(arr);
	/// 
	/// would result in the same initialized packed integer type. (Omitting alignment/casting).
	/// @param r15 Unsigned 8-bit value to put in the highest position of the result
	/// @param r14 
	/// @param r13 
	/// @param r12 
	/// @param r11 
	/// @param r10 
	/// @param r9 
	/// @param r8 
	/// @param r7 
	/// @param r6 
	/// @param r5 
	/// @param r4 
	/// @param r3 
	/// @param r2 
	/// @param r1 
	/// @param r0 Unsigned 8-bit value to put in the lowest position of the result
	/// @return Packed integer type initialized to the given values
	FORCEINLINE CONST m128u8 mm_set_epu8(uint8_t r15, uint8_t r14, uint8_t r13, uint8_t r12, uint8_t r11, uint8_t r10, uint8_t r9, uint8_t r8,
										 uint8_t r7, uint8_t r6, uint8_t r5, uint8_t r4, uint8_t r3, uint8_t r2, uint8_t r1, uint8_t r0)
	{
		m128u8 result;

		result.v[0] = r15;
		result.v[1] = r14;
		result.v[2] = r13;
		result.v[3] = r12;
		result.v[4] = r11;
		result.v[5] = r10;
		result.v[6] = r9;
		result.v[7] = r8;
		result.v[8] = r7;
		result.v[9] = r6;
		result.v[10] = r5;
		result.v[11] = r4;
		result.v[12] = r3;
		result.v[13] = r2;
		result.v[14] = r1;
		result.v[15] = r0;

		return result;
	}

	/// @brief Set a 128-bit packed integer type with given values.
	/// 
	/// This is a convenience function and corresponds to a multitude of instructions.
	/// 
	/// Ordering of the arguments is where they would be as if loaded from memory in order of their
	/// name. So the code:
	/// 
	///		int16_t arr[8] = { r0, r1, r2, ... , r7 };
	///		result = mm_load_epi16(arr);
	/// 
	/// would result in the same initialized packed integer type. (Omitting alignment/casting).
	/// @param r7 Signed 16-bit value to put in the highest position of the result
	/// @param r6 
	/// @param r5 
	/// @param r4 
	/// @param r3 
	/// @param r2 
	/// @param r1 
	/// @param r0 Signed 16-bit value to put in the lowest position of the result
	/// @return Packed integer type initialized to the given values
	FORCEINLINE CONST m128i16 mm_set_epi16(int16_t r7, int16_t r6, int16_t r5, int16_t r4, int16_t r3, int16_t r2, int16_t r1, int16_t r0)
	{
		m128i16 result;

		result.v[0] = r7;
		result.v[1] = r6;
		result.v[2] = r5;
		result.v[3] = r4;
		result.v[4] = r3;
		result.v[5] = r2;
		result.v[6] = r1;
		result.v[7] = r0;

		return result;
	}

	/// @brief Set a 128-bit packed integer type with given values.
	/// 
	/// This is a convenience function and corresponds to a multitude of instructions.
	/// 
	/// Ordering of the arguments is where they would be as if loaded from memory in order of their
	/// name. So the code:
	/// 
	///		uint16_t arr[8] = { r0, r1, r2, ... , r7 };
	///		result = mm_load_epu16(arr);
	/// 
	/// would result in the same initialized packed integer type. (Omitting alignment/casting).
	/// @param r7 Unsigned 16-bit value to put in the highest position of the result
	/// @param r6 
	/// @param r5 
	/// @param r4 
	/// @param r3 
	/// @param r2 
	/// @param r1 
	/// @param r0 Unsigned 16-bit value to put in the lowest position of the result
	/// @return Packed integer type initialized to the given values
	FORCEINLINE CONST m128u16 mm_set_epu16(uint16_t r7, uint16_t r6, uint16_t r5, uint16_t r4, uint16_t r3, uint16_t r2, uint16_t r1, uint16_t r0)
	{
		m128u16 result;

		result.v[0] = r7;
		result.v[1] = r6;
		result.v[2] = r5;
		result.v[3] = r4;
		result.v[4] = r3;
		result.v[5] = r2;
		result.v[6] = r1;
		result.v[7] = r0;

		return result;
	}

	/// @brief Set a 128-bit packed integer type with given values.
	/// 
	/// This is a convenience function and corresponds to a multitude of instructions.
	/// 
	/// Ordering of the arguments is where they would be as if loaded from memory in order of their
	/// name. So the code:
	/// 
	///		int32_t arr[16] = { r0, r1, r2, r3 };
	///		result = mm_load_epi32(arr);
	/// 
	/// would result in the same initialized packed integer type. (Omitting alignment/casting).
	/// @param r3 Signed 32-bit value to put in the highest position of the result
	/// @param r2 
	/// @param r1 
	/// @param r0 Signed 32-bit value to put in the lowest position of the result
	/// @return Packed integer type initialized to the given values
	FORCEINLINE CONST m128i32 mm_set_epi32(int32_t r3, int32_t r2, int32_t r1, int32_t r0)
	{
		m128i32 result;

		result.v[0] = r3;
		result.v[1] = r2;
		result.v[2] = r1;
		result.v[3] = r0;

		return result;
	}

	/// @brief Set a 128-bit packed integer type with given values.
	/// 
	/// This is a convenience function and corresponds to a multitude of instructions.
	/// 
	/// Ordering of the arguments is where they would be as if loaded from memory in order of their
	/// name. So the code:
	/// 
	///		uint32_t arr[4] = { r0, r1, r2, r3 };
	///		result = mm_load_epu32(arr);
	/// 
	/// would result in the same initialized packed integer type. (Omitting alignment/casting).
	/// @param r3 Unsigned 32-bit value to put in the highest position of the result
	/// @param r2 
	/// @param r1 
	/// @param r0 Unsigned 32-bit value to put in the lowest position of the result
	/// @return Packed integer type initialized to the given values
	FORCEINLINE CONST m128u32 mm_set_epu32(uint32_t r3, uint32_t r2, uint32_t r1, uint32_t r0)
	{
		m128u32 result;

		result.v[0] = r3;
		result.v[1] = r2;
		result.v[2] = r1;
		result.v[3] = r0;

		return result;
	}

	/// @brief Set a 128-bit packed integer type with given values.
	/// 
	/// This is a convenience function and corresponds to a multitude of instructions.
	/// 
	/// Ordering of the arguments is where they would be as if loaded from memory in order of their
	/// name. So the code:
	/// 
	///		int64_t arr[2] = { r0, r1 };
	///		result = mm_load_epi64(arr);
	/// 
	/// would result in the same initialized packed integer type. (Omitting alignment/casting).
	/// @param r1 Signed 64-bit value to put in the highest position of the result
	/// @param r0 Signed 64-bit value to put in the lowest position of the result
	/// @return Packed integer type initialized to the given values
	FORCEINLINE CONST m128i64 mm_set_epi64(int64_t r1, int64_t r0)
	{
		m128i64 result;

		result.v[0] = r1;
		result.v[1] = r0;

		return result;
	}

	/// @brief Set a 128-bit packed integer type with given values.
	/// 
	/// This is a convenience function and corresponds to a multitude of instructions.
	/// 
	/// Ordering of the arguments is where they would be as if loaded from memory in order of their
	/// name. So the code:
	/// 
	///		uint64_t arr[2] = { r0, r1 };
	///		result = mm_load_epu64(arr);
	/// 
	/// would result in the same initialized packed integer type. (Omitting alignment/casting).
	/// @param r1 Unsigned 64-bit value to put in the highest position of the result
	/// @param r0 Unsigned 64-bit value to put in the lowest position of the result
	/// @return Packed integer type initialized to the given values
	FORCEINLINE CONST m128u64 mm_set_epu64(uint64_t r1, uint64_t r0)
	{
		m128u64 result;

		result.v[0] = r1;
		result.v[1] = r0;

		return result;
	}

	/// @brief Set a 128-bit packed integer type with given values.
	/// 
	/// This is a convenience function. Note that qword_t is convertible to int128_t.
	/// @param r0 Signed 128-bit value to put in the result
	/// @return Packed integer type initialized to the given values
	FORCEINLINE CONST m128i128 mm_set_epi128(int128_t r0)
	{
		m128i128 result = { r0 };
		return result;
	}

	/// @brief Set a 128-bit packed integer type with given values.
	/// 
	/// This is a convenience function. Note that qword_t is convertible to uint128_t.
	/// @param r0 Unsigned 128-bit value to put in the result
	/// @return Packed integer type initialized to the given values
	FORCEINLINE CONST m128u128 mm_set_epu128(uint128_t r0)
	{
		m128u128 result = { r0 };
		return result;
	}
	

	/// Conversion functions
	///
	/// All of these functions are no-ops and follow the naming convention 'mm_cast<To>_<From>'
#define CAST(NameTo, NameFrom) FORCEINLINE CONST m128 ## NameTo mm_castep ## NameTo ## _ep ## NameFrom (m128 ## NameFrom v) { \
		m128 ## NameTo result;	\
		memcpy(&result, &v, sizeof(v));	\
		return result;	\
	}

	CAST(u8, i8);
	CAST(i16, i8);
	CAST(u16, i8);
	CAST(i32, i8);
	CAST(u32, i8);
	CAST(i64, i8);
	CAST(u64, i8);
	CAST(i128, i8);
	CAST(u128, i8);

	CAST(i8, u8);
	CAST(i16, u8);
	CAST(u16, u8);
	CAST(i32, u8);
	CAST(u32, u8);
	CAST(i64, u8);
	CAST(u64, u8);
	CAST(i128, u8);
	CAST(u128, u8);

	CAST(i8, i16);
	CAST(u8, i16);
	CAST(u16, i16);
	CAST(i32, i16);
	CAST(u32, i16);
	CAST(i64, i16);
	CAST(u64, i16);
	CAST(i128, i16);
	CAST(u128, i16);

	CAST(i8, u16);
	CAST(u8, u16);
	CAST(i16, u16);
	CAST(i32, u16);
	CAST(u32, u16);
	CAST(i64, u16);
	CAST(u64, u16);
	CAST(i128, u16);
	CAST(u128, u16);

	CAST(i8, i32);
	CAST(u8, i32);
	CAST(i16, i32);
	CAST(u16, i32);
	CAST(u32, i32);
	CAST(i64, i32);
	CAST(u64, i32);
	CAST(i128, i32);
	CAST(u128, i32);

	CAST(i8, u32);
	CAST(u8, u32);
	CAST(i16, u32);
	CAST(u16, u32);
	CAST(i32, u32);
	CAST(i64, u32);
	CAST(u64, u32);
	CAST(i128, u32);
	CAST(u128, u32);

	CAST(i8, i64);
	CAST(u8, i64);
	CAST(i16, i64);
	CAST(u16, i64);
	CAST(i32, i64);
	CAST(u32, i64);
	CAST(u64, i64);
	CAST(i128, i64);
	CAST(u128, i64);

	CAST(i8, u64);
	CAST(u8, u64);
	CAST(i16, u64);
	CAST(u16, u64);
	CAST(i32, u64);
	CAST(u32, u64);
	CAST(i64, u64);
	CAST(i128, u64);
	CAST(u128, u64);

	CAST(i8, i128);
	CAST(u8, i128);
	CAST(i16, i128);
	CAST(u16, i128);
	CAST(i32, i128);
	CAST(u32, i128);
	CAST(i64, i128);
	CAST(u64, i128);
	CAST(u128, i128);

	CAST(i8, u128);
	CAST(u8, u128);
	CAST(i16, u128);
	CAST(u16, u128);
	CAST(i32, u128);
	CAST(u32, u128);
	CAST(i64, u128);
	CAST(u64, u128);
	CAST(i128, u128);

#undef CAST

	/// @brief PMFLO : Parallel Move From LO register
	/// 
	/// Read the entire LO register and interpret its contents as signed 16-bit integers.
	/// 
	/// This function reads global state (LO).
	/// @return The current LO register
	FORCEINLINE PURE m128i16 mm_loadlo_epi16()
	{
		m128i16 result;

		///	volatile for same reason as 'load_lo*'
		asm volatile(
			"pmflo %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFLO : Parallel Move From LO register
	/// 
	/// Read the entire LO register and interpret its contents as unsigned 16-bit integers.
	/// 
	/// This function reads global state (LO).
	/// @return The current LO register
	FORCEINLINE PURE m128u16 mm_loadlo_epu16()
	{
		m128u16 result;

		///	volatile for same reason as 'load_lo*'
		asm volatile(
			"pmflo %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFLO : Parallel Move From LO register
	/// 
	/// Read the entire LO register and interpret its contents as unsigned 16-bit integers.
	/// 
	/// This function reads global state (LO).
	/// @return The current LO register
	FORCEINLINE PURE m128i32 mm_loadlo_epi32()
	{
		m128i32 result;

		///	volatile for same reason as 'load_lo*'
		asm volatile(
			"pmflo %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFLO : Parallel Move From LO register
	/// 
	/// Read the entire LO register and interpret its contents as unsigned 16-bit integers.
	/// 
	/// This function reads global state (LO).
	/// @return The current LO register
	FORCEINLINE PURE m128u32 mm_loadlo_epu32()
	{
		m128u32 result;

		///	volatile for same reason as 'load_lo*'
		asm volatile(
			"pmflo %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFHI : Parallel Move From HI register
	/// 
	/// Read the entire HI register and interpret its contents as unsigned 16-bit integers.
	/// 
	/// This function reads global state (HI).
	/// @return The current HI register
	FORCEINLINE PURE m128i16 mm_loadhi_epi16()
	{
		m128i16 result;

		///	volatile for same reason as 'load_hi*'
		asm volatile(
			"pmfhi %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFHI : Parallel Move From HI register
	/// 
	/// Read the entire HI register and interpret its contents as unsigned 16-bit integers.
	/// 
	/// This function reads global state (HI).
	/// @return The current HI register
	FORCEINLINE PURE m128u16 mm_loadhi_epu16()
	{
		m128u16 result;

		///	volatile for same reason as 'load_hi*'
		asm volatile(
			"pmfhi %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFHI : Parallel Move From HI register
	/// 
	/// Read the entire HI register and interpret its contents as unsigned 16-bit integers.
	/// 
	/// This function reads global state (HI).
	/// @return The current HI register
	FORCEINLINE PURE m128i32 mm_loadhi_epi32()
	{
		m128i32 result;

		///	volatile for same reason as 'load_hi*'
		asm volatile(
			"pmfhi %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFHI : Parallel Move From HI register
	/// 
	/// Read the entire HI register and interpret its contents as unsigned 16-bit integers.
	/// 
	/// This function reads global state (HI).
	/// @return The current HI register
	FORCEINLINE PURE m128u32 mm_loadhi_epu32()
	{
		m128u32 result;

		///	volatile for same reason as 'load_hi*'
		asm volatile(
			"pmfhi %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFHL.LH : Parallel Move From Hi/Lo register; Lower Halfwords
	/// 
	/// Copy contents of both LO and HI registers. Assume LO and HI contain 8 16-bit values
	/// each, of which only the even positions are used. Store the 4 values from LO to positions
	/// 0, 1, 4 and 5 of the result. The 4 values from the HI register are put in positions 2, 3,
	/// 6 and 7. Finally, interpret the result as packed signed 16-bit integers.
	/// 
	/// Bitwise reordering:
	///		Result[ 15,   0]	=	LO[ 15,   0]
	///		Result[ 31,  16]	=	LO[ 47,  32]
	///		Result[ 47,  32]	=	HI[ 15,   0]
	///		Result[ 63,  48]	=	HI[ 47,  32]
	///		Result[ 79,  64]	=	LO[ 79,  64]
	///		Result[ 95,  80]	=	LO[111,  96]
	///		Result[111,  96]	=	HI[ 79,  64]
	///		Result[127, 112]	=	HI[111,  96]
	/// 
	/// This function reads global state (LO/HI).
	/// @return Rearranged values from the LO and HI registers
	FORCEINLINE PURE m128i16 mm_loadlohi_lower_epi16()
	{
		m128i16 result;

		///	volatile for same reason as 'load_lo*'
		asm volatile(
			"pmfhl.lh %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFHL.LH : Parallel Move From Hi/Lo register; Lower Halfwords
	/// 
	/// Copy contents of both LO and HI registers. Assume LO and HI contain 8 16-bit values
	/// each, of which only the even positions are used. Store the 4 values from LO to positions
	/// 0, 1, 4 and 5 of the result. The 4 values from the HI register are put in positions 2, 3,
	/// 6 and 7. Finally, interpret the result as packed signed 16-bit integers.
	/// 
	/// Bitwise reordering:
	///		Result[ 15,   0]	=	LO[ 15,   0]
	///		Result[ 31,  16]	=	LO[ 47,  32]
	///		Result[ 47,  32]	=	HI[ 15,   0]
	///		Result[ 63,  48]	=	HI[ 47,  32]
	///		Result[ 79,  64]	=	LO[ 79,  64]
	///		Result[ 95,  80]	=	LO[111,  96]
	///		Result[111,  96]	=	HI[ 79,  64]
	///		Result[127, 112]	=	HI[111,  96]
	/// 
	/// This function reads global state (LO/HI).
	/// @return Rearranged values from the LO and HI registers
	FORCEINLINE PURE m128u16 mm_loadlohi_lower_epu16()
	{
		m128u16 result;

		///	volatile for same reason as 'load_lo*'
		asm volatile(
			"pmfhl.lh %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFHL.SH : Parallel Move From Hi/Lo register; Saturate lower Halfwords
	/// 
	/// Copy contents of both LO and HI registers. Assume LO and HI contain 4 32-bit values
	/// each, which get saturated to 16-bit signed values. Store the 4 values from LO to
	/// positions 0, 1, 4 and 5 of the result. The 4 values from the HI register are put in
	/// positions 2, 3, 6 and 7. Finally, interpret the result as packed signed 16-bit integers.
	/// 
	/// Bitwise reordering:
	///		Result[ 15,   0]	=	SaturateS16(LO[ 31,   0])
	///		Result[ 31,  16]	=	SaturateS16(LO[ 63,  32])
	///		Result[ 47,  32]	=	SaturateS16(HI[ 31,   0])
	///		Result[ 63,  48]	=	SaturateS16(HI[ 63,  32])
	///		Result[ 79,  64]	=	SaturateS16(LO[ 95,  64])
	///		Result[ 95,  80]	=	SaturateS16(LO[127,  96])
	///		Result[111,  96]	=	SaturateS16(HI[ 95,  64])
	///		Result[127, 112]	=	SaturateS16(HI[127,  96])
	/// 
	/// This function reads global state (LO/HI).
	/// @return Rearranged values from the LO and HI registers
	FORCEINLINE PURE m128i16 mm_loadslohi_lower_epi16()
	{
		m128i16 result;

		///	volatile for same reason as 'load_lo*'
		asm volatile(
			"pmfhl.sh %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFHL.LW : Parallel Move From Hi/Lo register; Lower Words
	/// 
	/// Copy contents of both LO and HI registers. Assume LO and HI contain 4 32-bit values
	/// each, of which only the even positions are used. Store the 2 values from LO to positions
	/// 0 and 2 of the result. The 2 values from the HI register are put in positions 1 and 3.
	/// Finally, interpret the result as packed signed 32-bit integers.
	/// 
	/// Bitwise reordering:
	///		Result[ 31,   0]	=	LO[ 31,   0]
	///		Result[ 63,  32]	=	HI[ 31,   0]
	///		Result[ 95,  64]	=	LO[ 95,  64]
	///		Result[127,  96]	=	HI[ 95,  64]
	/// 
	/// This function reads global state (LO/HI).
	/// @return Rearranged values from the LO and HI registers
	FORCEINLINE PURE m128i32 mm_loadlohi_lower_epi32()
	{
		m128i32 result;

		///	volatile for same reason as 'load_lo*'
		asm volatile(
			"pmfhl.lw %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFHL.LW : Parallel Move From Hi/Lo register; Lower Words
	/// 
	/// Copy contents of both LO and HI registers. Assume LO and HI contain 4 32-bit values
	/// each, of which only the even positions are used. Store the 2 values from LO to positions
	/// 0 and 2 of the result. The 2 values from the HI register are put in positions 1 and 3.
	/// Finally, interpret the result as packed unsigned 32-bit integers.
	/// 
	/// Bitwise reordering:
	///		Result[ 31,   0]	=	LO[ 31,   0]
	///		Result[ 63,  32]	=	HI[ 31,   0]
	///		Result[ 95,  64]	=	LO[ 95,  64]
	///		Result[127,  96]	=	HI[ 95,  64]
	/// 
	/// This function reads global state (LO/HI).
	/// @return Rearranged values from the LO and HI registers
	FORCEINLINE PURE m128u32 mm_loadlohi_lower_epu32()
	{
		m128u32 result;

		///	volatile for same reason as 'load_lo*'
		asm volatile(
			"pmfhl.lw %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFHL.SLW : Parallel Move From Hi/Lo register; Saturate lower Words
	/// 
	/// Copy contents of both LO and HI registers. Assume LO and HI contain 4 32-bit values
	/// each, of which only the even positions are used. Concatenate the 32-bit value in HI with
	/// the corresponding element in LO to 2 temporary 64-bit values. Saturate these values to
	/// signed 32-bit values, then store them with sign extension as 64-bit signed numbers in the
	/// result.
	/// 
	/// Bitwise reordering: (where '|' means concatenation)
	///		Result[ 63,   0]	=	SaturateS32(HI[31,  0] | LO[31,  0])
	///		Result[127,  64]	=	SaturateS32(HI[95, 64] | LO[95, 64])
	/// 
	/// This function reads global state (LO/HI).
	/// @return Rearranged values from the LO and HI registers
	FORCEINLINE PURE m128i64 mm_loadslohi_lower_epi64()
	{
		m128i64 result;

		///	volatile for same reason as 'load_lo*'
		asm volatile(
			"pmfhl.slw %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFHL.UW : Parallel Move From Hi/Lo register; Upper Words
	/// 
	/// Copy contents of both LO and HI registers. Assume LO and HI contain 4 32-bit values
	/// each, of which only the odd positions are used. Store the 2 values from LO to positions
	/// 0 and 2 of the result. The 2 values from the HI register are put in positions 1 and 3.
	/// Finally, interpret the result as packed signed 32-bit integers.
	/// 
	/// Bitwise reordering:
	///		Result[ 31,   0]	=	LO[ 63,  32]
	///		Result[ 63,  32]	=	HI[ 63,  32]
	///		Result[ 95,  64]	=	LO[127,  96]
	///		Result[127,  96]	=	HI[127,  96]
	/// 
	/// This function reads global state (LO/HI).
	/// @return Rearranged values from the LO and HI registers
	FORCEINLINE PURE m128i32 mm_loadlohi_upper_epi32()
	{
		m128i32 result;

		///	volatile for same reason as 'load_lo*'
		asm volatile(
			"pmfhl.uw %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMFHL.UW : Parallel Move From Hi/Lo register; Upper Words
	/// 
	/// Copy contents of both LO and HI registers. Assume LO and HI contain 4 32-bit values
	/// each, of which only the odd positions are used. Store the 2 values from LO to positions
	/// 0 and 2 of the result. The 2 values from the HI register are put in positions 1 and 3.
	/// Finally, interpret the result as packed unsigned 32-bit integers.
	/// 
	/// Bitwise reordering:
	///		Result[ 31,   0]	=	LO[ 63,  32]
	///		Result[ 63,  32]	=	HI[ 63,  32]
	///		Result[ 95,  64]	=	LO[127,  96]
	///		Result[127,  96]	=	HI[127,  96]
	/// 
	/// This function reads global state (LO/HI).
	/// @return Rearranged values from the LO and HI registers
	FORCEINLINE PURE m128u32 mm_loadlohi_upper_epu32()
	{
		m128u32 result;

		///	volatile for same reason as 'load_lo*'
		asm volatile(
			"pmfhl.uw %[Result]"
			: [Result] "=r" (result)
		);

		return result;
	}

	/// @brief PMTLO : Parallel Move To LO register
	/// 
	/// Store 8 signed 16-bit values to the LO register.
	/// @param v Value to store to LO
	FORCEINLINE void mm_storelo_epi16(m128i16 v)
	{
		// volatile for same reason as store_lo*
		asm volatile(
			"pmtlo %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTLO : Parallel Move To LO register
	/// 
	/// Store 8 unsigned 16-bit values to the LO register.
	/// @param v Value to store to LO
	FORCEINLINE void mm_storelo_epu16(m128u16 v)
	{
		// volatile for same reason as store_lo*
		asm volatile(
			"pmtlo %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTLO : Parallel Move To LO register
	/// 
	/// Store 4 signed 32-bit values to the LO register.
	/// @param v Value to store to LO
	FORCEINLINE void mm_storelo_epi32(m128i32 v)
	{
		// volatile for same reason as store_lo*
		asm volatile(
			"pmtlo %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTLO : Parallel Move To LO register
	/// 
	/// Store 4 unsigned 32-bit values to the LO register.
	/// @param v Value to store to LO
	FORCEINLINE void mm_storelo_epu32(m128u32 v)
	{
		// volatile for same reason as store_lo*
		asm volatile(
			"pmtlo %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTLO : Parallel Move To LO register
	/// 
	/// Store 2 signed 64-bit values to the LO register.
	/// @param v Value to store to LO
	FORCEINLINE void mm_storelo_epi64(m128i64 v)
	{
		// volatile for same reason as store_lo*
		asm volatile(
			"pmtlo %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTLO : Parallel Move To LO register
	/// 
	/// Store 2 unsigned 64-bit values to the LO register.
	/// @param v Value to store to LO
	FORCEINLINE void mm_storelo_epu64(m128u64 v)
	{
		// volatile for same reason as store_lo*
		asm volatile(
			"pmtlo %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTLO : Parallel Move To LO register
	/// 
	/// Store 1 signed 128-bit value to the LO register.
	/// @param v Value to store to LO
	FORCEINLINE void mm_storelo_epi128(m128i128 v)
	{
		// volatile for same reason as store_lo*
		asm volatile(
			"pmtlo %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTLO : Parallel Move To LO register
	/// 
	/// Store 1 unsigned 128-bit value to the LO register.
	/// @param v Value to store to LO
	FORCEINLINE void mm_storelo_epu64(m128u64 v)
	{
		// volatile for same reason as store_lo*
		asm volatile(
			"pmtlo %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTHI : Parallel Move To HI register
	/// 
	/// Store 8 signed 16-bit values to the HI register.
	/// @param v Value to store to HI
	FORCEINLINE void mm_storehi_epi16(m128i16 v)
	{
		// volatile for same reason as store_hi*
		asm volatile(
			"pmthi %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTHI : Parallel Move To HI register
	/// 
	/// Store 8 unsigned 16-bit values to the HI register.
	/// @param v Value to store to HI
	FORCEINLINE void mm_storehi_epu16(m128u16 v)
	{
		// volatile for same reason as store_hi*
		asm volatile(
			"pmthi %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTHI : Parallel Move To HI register
	/// 
	/// Store 4 signed 32-bit values to the HI register.
	/// @param v Value to store to HI
	FORCEINLINE void mm_storehi_epi32(m128i32 v)
	{
		// volatile for same reason as store_hi*
		asm volatile(
			"pmthi %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTHI : Parallel Move To HI register
	/// 
	/// Store 4 unsigned 32-bit values to the HI register.
	/// @param v Value to store to HI
	FORCEINLINE void mm_storehi_epu32(m128u32 v)
	{
		// volatile for same reason as store_hi*
		asm volatile(
			"pmthi %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTHI : Parallel Move To HI register
	/// 
	/// Store 2 signed 64-bit values to the HI register.
	/// @param v Value to store to HI
	FORCEINLINE void mm_storehi_epi64(m128i64 v)
	{
		// volatile for same reason as store_hi*
		asm volatile(
			"pmthi %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTHI : Parallel Move To HI register
	/// 
	/// Store 2 unsigned 64-bit values to the HI register.
	/// @param v Value to store to HI
	FORCEINLINE void mm_storehi_epu64(m128u64 v)
	{
		// volatile for same reason as store_hi*
		asm volatile(
			"pmthi %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTHI : Parallel Move To HI register
	/// 
	/// Store 1 signed 128-bit value to the HI register.
	/// @param v Value to store to HI
	FORCEINLINE void mm_storehi_epi128(m128i128 v)
	{
		// volatile for same reason as store_hi*
		asm volatile(
			"pmthi %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTHI : Parallel Move To HI register
	/// 
	/// Store 1 unsigned 128-bit value to the HI register.
	/// @param v Value to store to HI
	FORCEINLINE void mm_storehi_epu128(m128u128 v)
	{
		// volatile for same reason as store_hi*
		asm volatile(
			"pmthi %[Value]"
			:
		: [Value] "r" (v)
			);
	}

	/// @brief PMTHL.LW : Parallel Move To Hi/Lo register; Lower Words
	/// 
	/// Move 4 32-bit values to both LO and HI registers. Values in even position are moved to
	/// even positions in the LO register and odd positions are moved to even positions in the HI
	/// register. Odd positions in both LO and HI are not changed.
	/// 
	/// Bitwise reordering:
	///		LO[ 31,   0]	=	Value[ 31,   0]
	///		HI[ 31,   0]	=	Value[ 63,  32]
	///		LO[ 95,  64]	=	Value[ 95,  64]
	///		HI[ 95,  64]	=	Value[127,  96]
	/// @param v Packed values to store to LO and HI registers
	FORCEINLINE void mm_storelohi_epi32(m128i32 v)
	{
		// volatile for same reason as store_lo*
		asm volatile(
			"pmthl.lw %[Value]"
			:
			: [Value] "r" (v)
		);
	}

	/// @brief PMTHL.LW : Parallel Move To Hi/Lo register; Lower Words
	/// 
	/// Move 4 32-bit values to both LO and HI registers. Values in even position are moved to
	/// even positions in the LO register and odd positions are moved to even positions in the HI
	/// register. Odd positions in both LO and HI are not changed.
	/// 
	/// Bitwise reordering:
	///		LO[ 31,   0]	=	Value[ 31,   0]
	///		HI[ 31,   0]	=	Value[ 63,  32]
	///		LO[ 95,  64]	=	Value[ 95,  64]
	///		HI[ 95,  64]	=	Value[127,  96]
	/// @param v Packed values to store to LO and HI registers
	FORCEINLINE void mm_storelohi_epu32(m128u32 v)
	{
		// volatile for same reason as store_lo*
		asm volatile(
			"pmthl.lw %[Value]"
			:
			: [Value] "r" (v)
		);
	}


	/// @brief PAND : Parallel AND
	/// 
	/// Compute bitwise-AND of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-AND of both operands
	FORCEINLINE CONST m128i8 mm_and_epi8(m128i8 l, m128i8 r)
	{
		m128i8 result;

		asm("pand %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PAND : Parallel AND
	/// 
	/// Compute bitwise-AND of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-AND of both operands
	FORCEINLINE CONST m128u8 mm_and_epu8(m128u8 l, m128u8 r)
	{
		m128u8 result;

		asm("pand %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PAND : Parallel AND
	/// 
	/// Compute bitwise-AND of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-AND of both operands
	FORCEINLINE CONST m128i16 mm_and_epi16(m128i16 l, m128i16 r)
	{
		m128i16 result;

		asm("pand %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PAND : Parallel AND
	/// 
	/// Compute bitwise-AND of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-AND of both operands
	FORCEINLINE CONST m128u16 mm_and_epu16(m128u16 l, m128u16 r)
	{
		m128u16 result;

		asm("pand %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PAND : Parallel AND
	/// 
	/// Compute bitwise-AND of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-AND of both operands
	FORCEINLINE CONST m128i32 mm_and_epi32(m128i32 l, m128i32 r)
	{
		m128i32 result;

		asm("pand %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PAND : Parallel AND
	/// 
	/// Compute bitwise-AND of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-AND of both operands
	FORCEINLINE CONST m128u32 mm_and_epu32(m128u32 l, m128u32 r)
	{
		m128u32 result;

		asm("pand %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PAND : Parallel AND
	/// 
	/// Compute bitwise-AND of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-AND of both operands
	FORCEINLINE CONST m128i64 mm_and_epi64(m128i64 l, m128i64 r)
	{
		m128i64 result;

		asm("pand %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PAND : Parallel AND
	/// 
	/// Compute bitwise-AND of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-AND of both operands
	FORCEINLINE CONST m128u64 mm_and_epu64(m128u64 l, m128u64 r)
	{
		m128u64 result;

		asm("pand %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PAND : Parallel AND
	/// 
	/// Compute bitwise-AND of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-AND of both operands
	FORCEINLINE CONST m128i128 mm_and_epi128(m128i128 l, m128i128 r)
	{
		m128i128 result;

		asm("pand %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PAND : Parallel AND
	/// 
	/// Compute bitwise-AND of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-AND of both operands
	FORCEINLINE CONST m128u128 mm_and_epu128(m128u128 l, m128u128 r)
	{
		m128u128 result;

		asm("pand %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}


	/// @brief POR : Parallel OR
	/// 
	/// Compute bitwise-OR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-OR of both operands
	FORCEINLINE CONST m128i8 mm_or_epi8(m128i8 l, m128i8 r)
	{
		m128i8 result;

		asm("por %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief POR : Parallel OR
	/// 
	/// Compute bitwise-OR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-OR of both operands
	FORCEINLINE CONST m128u8 mm_or_epu8(m128u8 l, m128u8 r)
	{
		m128u8 result;

		asm("por %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief POR : Parallel OR
	/// 
	/// Compute bitwise-OR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-OR of both operands
	FORCEINLINE CONST m128i16 mm_or_epi16(m128i16 l, m128i16 r)
	{
		m128i16 result;

		asm("por %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief POR : Parallel OR
	/// 
	/// Compute bitwise-OR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-OR of both operands
	FORCEINLINE CONST m128u16 mm_or_epu16(m128u16 l, m128u16 r)
	{
		m128u16 result;

		asm("por %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief POR : Parallel OR
	/// 
	/// Compute bitwise-OR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-OR of both operands
	FORCEINLINE CONST m128i32 mm_or_epi32(m128i32 l, m128i32 r)
	{
		m128i32 result;

		asm("por %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief POR : Parallel OR
	/// 
	/// Compute bitwise-OR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-OR of both operands
	FORCEINLINE CONST m128u32 mm_or_epu32(m128u32 l, m128u32 r)
	{
		m128u32 result;

		asm("por %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief POR : Parallel OR
	/// 
	/// Compute bitwise-OR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-OR of both operands
	FORCEINLINE CONST m128i64 mm_or_epi64(m128i64 l, m128i64 r)
	{
		m128i64 result;

		asm("por %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief POR : Parallel OR
	/// 
	/// Compute bitwise-OR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-OR of both operands
	FORCEINLINE CONST m128u64 mm_or_epu64(m128u64 l, m128u64 r)
	{
		m128u64 result;

		asm("por %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief POR : Parallel OR
	/// 
	/// Compute bitwise-OR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-OR of both operands
	FORCEINLINE CONST m128i128 mm_or_epi128(m128i128 l, m128i128 r)
	{
		m128i128 result;

		asm("por %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief POR : Parallel OR
	/// 
	/// Compute bitwise-OR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-OR of both operands
	FORCEINLINE CONST m128u128 mm_or_epu128(m128u128 l, m128u128 r)
	{
		m128u128 result;

		asm("por %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}


	/// @brief PXOR : Parallel XOR
	/// 
	/// Compute bitwise-XOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-XOR of both operands
	FORCEINLINE CONST m128i8 mm_xor_epi8(m128i8 l, m128i8 r)
	{
		m128i8 result;

		asm("pxor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PXOR : Parallel XOR
	/// 
	/// Compute bitwise-XOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-XOR of both operands
	FORCEINLINE CONST m128u8 mm_xor_epu8(m128u8 l, m128u8 r)
	{
		m128u8 result;

		asm("pxor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PXOR : Parallel XOR
	/// 
	/// Compute bitwise-XOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-XOR of both operands
	FORCEINLINE CONST m128i16 mm_xor_epi16(m128i16 l, m128i16 r)
	{
		m128i16 result;

		asm("pxor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PXOR : Parallel XOR
	/// 
	/// Compute bitwise-XOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-XOR of both operands
	FORCEINLINE CONST m128u16 mm_xor_epu16(m128u16 l, m128u16 r)
	{
		m128u16 result;

		asm("pxor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PXOR : Parallel XOR
	/// 
	/// Compute bitwise-XOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-XOR of both operands
	FORCEINLINE CONST m128i32 mm_xor_epi32(m128i32 l, m128i32 r)
	{
		m128i32 result;

		asm("pxor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PXOR : Parallel XOR
	/// 
	/// Compute bitwise-XOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-XOR of both operands
	FORCEINLINE CONST m128u32 mm_xor_epu32(m128u32 l, m128u32 r)
	{
		m128u32 result;

		asm("pxor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PXOR : Parallel XOR
	/// 
	/// Compute bitwise-XOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-XOR of both operands
	FORCEINLINE CONST m128i64 mm_xor_epi64(m128i64 l, m128i64 r)
	{
		m128i64 result;

		asm("pxor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PXOR : Parallel XOR
	/// 
	/// Compute bitwise-XOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-XOR of both operands
	FORCEINLINE CONST m128u64 mm_xor_epu64(m128u64 l, m128u64 r)
	{
		m128u64 result;

		asm("pxor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PXOR : Parallel XOR
	/// 
	/// Compute bitwise-XOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-XOR of both operands
	FORCEINLINE CONST m128i128 mm_xor_epi128(m128i128 l, m128i128 r)
	{
		m128i128 result;

		asm("pxor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PXOR : Parallel XOR
	/// 
	/// Compute bitwise-XOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-XOR of both operands
	FORCEINLINE CONST m128u128 mm_xor_epu128(m128u128 l, m128u128 r)
	{
		m128u128 result;

		asm("pxor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}


	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-NOR of both operands
	FORCEINLINE CONST m128i8 mm_nor_epi8(m128i8 l, m128i8 r)
	{
		m128i8 result;

		asm("pnor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-NOR of both operands
	FORCEINLINE CONST m128u8 mm_nor_epu8(m128u8 l, m128u8 r)
	{
		m128u8 result;

		asm("pnor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-NOR of both operands
	FORCEINLINE CONST m128i16 mm_nor_epi16(m128i16 l, m128i16 r)
	{
		m128i16 result;

		asm("pnor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-NOR of both operands
	FORCEINLINE CONST m128u16 mm_nor_epu16(m128u16 l, m128u16 r)
	{
		m128u16 result;

		asm("pnor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-NOR of both operands
	FORCEINLINE CONST m128i32 mm_nor_epi32(m128i32 l, m128i32 r)
	{
		m128i32 result;

		asm("pnor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-NOR of both operands
	FORCEINLINE CONST m128u32 mm_nor_epu32(m128u32 l, m128u32 r)
	{
		m128u32 result;

		asm("pnor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-NOR of both operands
	FORCEINLINE CONST m128i64 mm_nor_epi64(m128i64 l, m128i64 r)
	{
		m128i64 result;

		asm("pnor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-NOR of both operands
	FORCEINLINE CONST m128u64 mm_nor_epu64(m128u64 l, m128u64 r)
	{
		m128u64 result;

		asm("pnor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-NOR of both operands
	FORCEINLINE CONST m128i128 mm_nor_epi128(m128i128 l, m128i128 r)
	{
		m128i128 result;

		asm("pnor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOR of 128-bit values.
	/// @param l First operand
	/// @param r Second operand
	/// @return Bitwise-NOR of both operands
	FORCEINLINE CONST m128u128 mm_nor_epu128(m128u128 l, m128u128 r)
	{
		m128u128 result;

		asm("pnor %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}


	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOT of 128-bit values.
	/// @param v Operand
	/// @return Bitwise-NOT of operand
	FORCEINLINE CONST m128i8 mm_not_epi8(m128i8 v)
	{
		return mm_nor_epi8(v, v);
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOT of 128-bit values.
	/// @param v Operand
	/// @return Bitwise-NOT of operand
	FORCEINLINE CONST m128u8 mm_not_epu8(m128u8 v)
	{
		return mm_nor_epu8(v, v);
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOT of 128-bit values.
	/// @param v Operand
	/// @return Bitwise-NOT of operand
	FORCEINLINE CONST m128i16 mm_not_epi16(m128i16 v)
	{
		return mm_nor_epi16(v, v);
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOT of 128-bit values.
	/// @param v Operand
	/// @return Bitwise-NOT of operand
	FORCEINLINE CONST m128u16 mm_not_epu16(m128u16 v)
	{
		return mm_nor_epu16(v, v);
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOT of 128-bit values.
	/// @param v Operand
	/// @return Bitwise-NOT of operand
	FORCEINLINE CONST m128i32 mm_not_epi32(m128i32 v)
	{
		return mm_nor_epi32(v, v);
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOT of 128-bit values.
	/// @param v Operand
	/// @return Bitwise-NOT of operand
	FORCEINLINE CONST m128u32 mm_not_epu32(m128u32 v)
	{
		return mm_nor_epu32(v, v);
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOT of 128-bit values.
	/// @param v Operand
	/// @return Bitwise-NOT of operand
	FORCEINLINE CONST m128i64 mm_not_epi64(m128i64 v)
	{
		return mm_nor_epi64(v, v);
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOT of 128-bit values.
	/// @param v Operand
	/// @return Bitwise-NOT of operand
	FORCEINLINE CONST m128u64 mm_not_epu64(m128u64 v)
	{
		return mm_nor_epu64(v, v);
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOT of 128-bit values.
	/// @param v Operand
	/// @return Bitwise-NOT of operand
	FORCEINLINE CONST m128i128 mm_not_epi128(m128i128 v)
	{
		return mm_nor_epi128(v, v);
	}

	/// @brief PNOR : Parallel NOR
	/// 
	/// Compute bitwise-NOT of 128-bit values.
	/// @param v Operand
	/// @return Bitwise-NOT of operand
	FORCEINLINE CONST m128u128 mm_not_epu128(m128u128 v)
	{
		return mm_nor_epu128(v, v);
	}


	/// @brief PCEQB : Parallel Compare for EQual Byte
	/// 
	/// Compare 16 8-bit value pairs for equality. On equality, return 0xFF for that value pair,
	/// otherwise 0x00.
	/// @param l First operand
	/// @param r Second operand
	/// @return Equality result mask
	FORCEINLINE CONST m128i8 mm_cmpeq_epi8(m128i8 l, m128i8 r)
	{
		m128i8 result;

		asm(
			"pceqb %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PCEQB : Parallel Compare for EQual Byte
	/// PNOR : Parallel NOR
	/// 
	/// Convenience function. Invert result of 'cmpeq'.
	/// @param l First operand
	/// @param r Second operand
	/// @return Inverted equality result mask
	FORCEINLINE CONST m128i8 mm_cmpneq_epi8(m128i8 l, m128i8 r)
	{
		return mm_not_epi8(mm_cmpeq_epi8(l, r));
	}

	/// @brief PCGTB : Parallel Compare for Greater Than Byte
	/// 
	/// Compare 16 signed 8-bit value pairs for a greater than relationship. Returns 0xFF for
	/// pairs where the value in 'l' is true greater than the value in 'r', and 0x00 otherwise.
	/// 
	/// Unsigned comparisons can be achieved by first adding '0x80' to every element in each
	/// operand.
	/// @param l Left operand
	/// @param r Right operand
	/// @return Comparison result mask
	FORCEINLINE CONST m128i8 mm_cmpgt_epi8(m128i8 l, m128i8 r)
	{
		m128i8 result;

		asm(
			"pcgtb %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PCGTB : Parallel Compare for Greater Than Byte
	/// 
	/// Compare 16 signed 8-bit value pairs for a less than relationship. Returns 0xFF for
	/// pairs where the value in 'l' is true less than the value in 'r', and 0x00 otherwise.
	/// 
	/// Unsigned comparisons can be achieved by first adding '0x80' to every element in each
	/// operand.
	/// @param l Left operand
	/// @param r Right operand
	/// @return Comparison result mask
	FORCEINLINE CONST m128i8 mm_cmplt_epi8(m128i8 l, m128i8 r)
	{
		return mm_cmpgt_epi8(r, l);
	}

	/// @brief PCGTB : Parallel Compare for Greater Than Byte
	/// PNOR : Parallel NOR
	/// 
	/// Compare 16 signed 8-bit value pairs for a less than or equal relationship. Returns 0xFF
	/// for pairs where the value in 'l' is less than or equal the value in 'r', and 0x00
	/// otherwise.
	/// 
	/// Unsigned comparisons can be achieved by first adding '0x80' to every element in each
	/// operand.
	/// @param l Left operand
	/// @param r Right operand
	/// @return Comparison result mask
	FORCEINLINE CONST m128i8 mm_cmple_epi8(m128i8 l, m128i8 r)
	{
		return mm_not_epi8(mm_cmpgt_epi8(l, r));
	}

	/// @brief PCGTB : Parallel Compare for Greater Than Byte
	/// PNOR : Parallel NOR
	/// 
	/// Compare 16 signed 8-bit value pairs for a greater than or equal relationship. Returns 0xFF
	/// for pairs where the value in 'l' is greater than or equal the value in 'r', and 0x00
	/// otherwise.
	/// 
	/// Unsigned comparisons can be achieved by first adding '0x80' to every element in each
	/// operand.
	/// @param l Left operand
	/// @param r Right operand
	/// @return Comparison result mask
	FORCEINLINE CONST m128i8 mm_cmpge_epi8(m128i8 l, m128i8 r)
	{
		return mm_not_epi8(mm_cmpgt_epi8(r, l));
	}

	/// @brief PCEQB : Parallel Compare for EQual Byte
	/// 
	/// Compare 16 8-bit value pairs for equality. On equality, return 0xFF for that value pair,
	/// otherwise 0x00.
	/// @param l First operand
	/// @param r Second operand
	/// @return Equality result mask
	FORCEINLINE CONST m128u8 mm_cmpeq_epu8(m128u8 l, m128u8 r)
	{
		m128u8 result;

		asm(
			"pceqb %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PCEQB : Parallel Compare for EQual Byte
	/// PNOR : Parallel NOR
	/// 
	/// Convenience function. Invert result of 'cmpeq'.
	/// @param l First operand
	/// @param r Second operand
	/// @return Inverted equality result mask
	FORCEINLINE CONST m128u8 mm_cmpneq_epu8(m128u8 l, m128u8 r)
	{
		return mm_not_epu8(mm_cmpeq_epu8(l, r));
	}

	/// @brief PCEQH : Parallel Compare for EQual Halfword
	/// 
	/// Compare 8 16-bit value pairs for equality. On equality, return 0xFFFF for that value pair,
	/// otherwise 0x0000.
	/// @param l First operand
	/// @param r Second operand
	/// @return Equality result mask
	FORCEINLINE CONST m128i16 mm_cmpeq_epi16(m128i16 l, m128i16 r)
	{
		m128i16 result;

		asm(
			"pceqh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PCEQB : Parallel Compare for EQual Byte
	/// PNOR : Parallel NOR
	/// 
	/// Convenience function. Invert result of 'cmpeq'.
	/// @param l First operand
	/// @param r Second operand
	/// @return Inverted equality result mask
	FORCEINLINE CONST m128i16 mm_cmpneq_epi16(m128i16 l, m128i16 r)
	{
		return mm_not_epi16(mm_cmpeq_epi16(l, r));
	}

	/// @brief PCGTH : Parallel Compare for Greater Than Halfword
	/// 
	/// Compare 8 signed 16-bit value pairs for a greater than relationship. Returns 0xFFFF for
	/// pairs where the value in 'l' is true greater than the value in 'r', and 0x0000 otherwise.
	/// 
	/// Unsigned comparisons can be achieved by first adding '0x8000' to every element in each
	/// operand.
	/// @param l Left operand
	/// @param r Right operand
	/// @return Comparison result mask
	FORCEINLINE CONST m128i16 mm_cmpgt_epi16(m128i16 l, m128i16 r)
	{
		m128i16 result;

		asm(
			"pcgth %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PCGTH : Parallel Compare for Greater Than Halfword
	/// 
	/// Compare 8 signed 16-bit value pairs for a less than relationship. Returns 0xFFFF for
	/// pairs where the value in 'l' is true less than the value in 'r', and 0x0000 otherwise.
	/// 
	/// Unsigned comparisons can be achieved by first adding '0x8000' to every element in each
	/// operand.
	/// @param l Left operand
	/// @param r Right operand
	/// @return Comparison result mask
	FORCEINLINE CONST m128i16 mm_cmplt_epi16(m128i16 l, m128i16 r)
	{
		return mm_cmpgt_epi16(r, l);
	}

	/// @brief PCGTH : Parallel Compare for Greater Than Halfword
	/// PNOR : Parallel NOR
	/// 
	/// Compare 8 signed 16-bit value pairs for a less than or equal relationship. Returns 0xFFFF
	/// for pairs where the value in 'l' is less than or equal the value in 'r', and 0x0000
	/// otherwise.
	/// 
	/// Unsigned comparisons can be achieved by first adding '0x8000' to every element in each
	/// operand.
	/// @param l Left operand
	/// @param r Right operand
	/// @return Comparison result mask
	FORCEINLINE CONST m128i16 mm_cmple_epi16(m128i16 l, m128i16 r)
	{
		return mm_not_epi16(mm_cmpgt_epi16(l, r));
	}

	/// @brief PCGTH : Parallel Compare for Greater Than Halfword
	/// PNOR : Parallel NOR
	/// 
	/// Compare 8 signed 16-bit value pairs for a greater than or equal relationship. Returns 0xFFFF
	/// for pairs where the value in 'l' is greater than or equal the value in 'r', and 0x0000
	/// otherwise.
	/// 
	/// Unsigned comparisons can be achieved by first adding '0x8000' to every element in each
	/// operand.
	/// @param l Left operand
	/// @param r Right operand
	/// @return Comparison result mask
	FORCEINLINE CONST m128i16 mm_cmpge_epi16(m128i16 l, m128i16 r)
	{
		return mm_not_epi16(mm_cmpgt_epi16(r, l));
	}

	/// @brief PCEQH : Parallel Compare for EQual Halfword
	/// 
	/// Compare 8 16-bit value pairs for equality. On equality, return 0xFFFF for that value pair,
	/// otherwise 0x0000.
	/// @param l First operand
	/// @param r Second operand
	/// @return Equality result mask
	FORCEINLINE CONST m128u16 mm_cmpeq_epu16(m128u16 l, m128u16 r)
	{
		m128u16 result;

		asm(
			"pceqh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PCEQB : Parallel Compare for EQual Byte
	/// PNOR : Parallel NOR
	/// 
	/// Convenience function. Invert result of 'cmpeq'.
	/// @param l First operand
	/// @param r Second operand
	/// @return Inverted equality result mask
	FORCEINLINE CONST m128u16 mm_cmpneq_epu16(m128u16 l, m128u16 r)
	{
		return mm_not_epu16(mm_cmpeq_epu16(l, r));
	}

	/// @brief PCEQW : Parallel Compare for EQual Word
	/// 
	/// Compare 4 32-bit value pairs for equality. On equality, return 0xFFFFFFFF for that value
	/// pair, otherwise 0x00000000.
	/// @param l First operand
	/// @param r Second operand
	/// @return Equality result mask
	FORCEINLINE CONST m128i32 mm_cmpeq_epi32(m128i32 l, m128i32 r)
	{
		m128i32 result;

		asm(
			"pceqw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PCEQB : Parallel Compare for EQual Byte
	/// PNOR : Parallel NOR
	/// 
	/// Convenience function. Invert result of 'cmpeq'.
	/// @param l First operand
	/// @param r Second operand
	/// @return Inverted equality result mask
	FORCEINLINE CONST m128i32 mm_cmpneq_epi32(m128i32 l, m128i32 r)
	{
		return mm_not_epi32(mm_cmpeq_epi32(l, r));
	}

	/// @brief PCGTW : Parallel Compare for Greater Than Word
	/// 
	/// Compare 4 signed 32-bit value pairs for a greater than relationship. Returns 0xFFFFFFFF
	/// for pairs where the value in 'l' is true greater than the value in 'r', and 0x00000000
	/// otherwise.
	/// 
	/// Unsigned comparisons can be achieved by first adding '0x80000000' to every element in each
	/// operand.
	/// @param l Left operand
	/// @param r Right operand
	/// @return Comparison result mask
	FORCEINLINE CONST m128i32 mm_cmpgt_epi32(m128i32 l, m128i32 r)
	{
		m128i32 result;

		asm(
			"pcgtw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PCGTW : Parallel Compare for Greater Than Word
	/// 
	/// Compare 4 signed 32-bit value pairs for a less than relationship. Returns 0xFFFFFFFF for
	/// pairs where the value in 'l' is true less than the value in 'r', and 0x00000000 otherwise.
	/// 
	/// Unsigned comparisons can be achieved by first adding '0x80000000' to every element in each
	/// operand.
	/// @param l Left operand
	/// @param r Right operand
	/// @return Comparison result mask
	FORCEINLINE CONST m128i32 mm_cmplt_epi32(m128i32 l, m128i32 r)
	{
		return mm_cmpgt_epi32(r, l);
	}

	/// @brief PCGTW : Parallel Compare for Greater Than Word
	/// PNOR : Parallel NOR
	/// 
	/// Compare 4 signed 32-bit value pairs for a less than or equal relationship. Returns
	/// 0xFFFFFFFF for pairs where the value in 'l' is less than or equal the value in 'r', and
	/// 0x00000000 otherwise.
	/// 
	/// Unsigned comparisons can be achieved by first adding '0x80000000' to every element in each
	/// operand.
	/// @param l Left operand
	/// @param r Right operand
	/// @return Comparison result mask
	FORCEINLINE CONST m128i32 mm_cmple_epi32(m128i32 l, m128i32 r)
	{
		return mm_not_epi32(mm_cmpgt_epi32(l, r));
	}

	/// @brief PCGTW : Parallel Compare for Greater Than Word
	/// PNOR : Parallel NOR
	/// 
	/// Compare 4 signed 32-bit value pairs for a greater than or equal relationship. Returns
	/// 0xFFFFFFFF for pairs where the value in 'l' is greater than or equal the value in 'r', and
	/// 0x00000000 otherwise.
	/// 
	/// Unsigned comparisons can be achieved by first adding '0x80000000' to every element in each
	/// operand.
	/// @param l Left operand
	/// @param r Right operand
	/// @return Comparison result mask
	FORCEINLINE CONST m128i32 mm_cmpge_epi32(m128i32 l, m128i32 r)
	{
		return mm_not_epi32(mm_cmpgt_epi32(r, l));
	}

	/// @brief PCEQW : Parallel Compare for EQual Word
	/// 
	/// Compare 4 32-bit value pairs for equality. On equality, return 0xFFFFFFFF for that value
	/// pair, otherwise 0x00000000.
	/// @param l First operand
	/// @param r Second operand
	/// @return Equality result mask
	FORCEINLINE CONST m128u32 mm_cmpeq_epu32(m128u32 l, m128u32 r)
	{
		m128u32 result;

		asm(
			"pceqw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PCEQB : Parallel Compare for EQual Byte
	/// PNOR : Parallel NOR
	/// 
	/// Convenience function. Invert result of 'cmpeq'.
	/// @param l First operand
	/// @param r Second operand
	/// @return Inverted equality result mask
	FORCEINLINE CONST m128u32 mm_cmpneq_epu32(m128u32 l, m128u32 r)
	{
		return mm_not_epu32(mm_cmpeq_epu32(l, r));
	}


	/// @brief PSLLH : Parallel Shift Left Logical Halfword
	/// 
	/// Logically left shift 16-bit values. Shifts in '0's into the lower bits.
	/// @param result Variable identifier to store result to. Must be of type 'm128i16' or
	/// 'm128u16'
	/// @param value Variable identifier to use as source value. Must be of type 'm128i16' or
	/// 'm128u16'
	/// @param shift_value Amount of bits to shift the source value left. Must be in range [0, 15]
#define PSLLH(result, value, shift_amount)															\
		asm(																						\
			"psllh %[Result],%[Value],%c[ShiftAmount]"												\
			: [Result] "=r" (result)																\
			: [Value] "r" (value), [ShiftAmount] "n" (shift_amount)									\
		)

	/// @brief PSLLW : Parallel Shift Left Logical Word
	/// 
	/// Logically left shift 32-bit values. Shifts in '0's into the lower bits.
	/// @param result Variable identifier to store result to. Must be of type 'm128i32' or
	/// 'm128u32'
	/// @param value Variable identifier to use as source value. Must be of type 'm128i32' or
	/// 'm128u32'
	/// @param shift_value Amount of bits to shift the source value left. Must be in range [0, 31]
#define PSLLW(result, value, shift_amount)															\
		asm(																						\
			"psllw %[Result],%[Value],%c[ShiftAmount]"												\
			: [Result] "=r" (result)																\
			: [Value] "r" (value), [ShiftAmount] "n" (shift_amount)									\
		)

	/// @brief PSLLVW : Parallel Shift Left Logical Variable Word
	/// 
	/// Treat 128-bit packed integer data as 2 sign-extended 32-bit values. Shift each value left
	/// while shifting in '0's by an amount equal to the corresponding value in 'shift_amount'.
	/// Only the low 5 bits in each value of 'shift_amount' are used, others are ignored.
	/// @param value Values to shift
	/// @param shift_amount Amount to shift values by
	/// @return Values shifted left by variable amount
	FORCEINLINE CONST m128i64 mm_sllv_epi64(m128i64 value, m128u64 shift_amount)
	{
		m128i64 result;

		asm(
			"psllvw %[Result],%[Value],%[Amount]"
			: [Result] "=r" (result)
			: [Value] "r" (value), [Amount] "r" (shift_amount)
		);

		return result;
	}

	/// @brief PSRAH : Parallel Shift Right Arithmetic Halfword
	/// 
	/// Arithmetically right shift 16-bit values. Shifts in sign bits into the upper bits.
	/// @param result Variable identifier to store result to. Must be of type 'm128i16'
	/// @param value Variable identifier to use as source value. Must be of type 'm128i16'
	/// @param shift_value Amount of bits to shift the source value right. Must be in range [0, 15]
#define PSRAH(result, value, shift_amount)															\
		asm(																						\
			"psrah %[Result],%[Value],%c[ShiftAmount]"												\
			: [Result] "=r" (result)																\
			: [Value] "r" (value), [ShiftAmount] "n" (shift_amount)									\
		)

	/// @brief PSRAW : Parallel Shift Right Arithmetic Word
	/// 
	/// Arithmetically right shift 32-bit values. Shifts in sign bits into the upper bits.
	/// @param result Variable identifier to store result to. Must be of type 'm128i32'
	/// @param value Variable identifier to use as source value. Must be of type 'm128i32'
	/// @param shift_value Amount of bits to shift the source value left. Must be in range [0, 31]
#define PSRAW(result, value, shift_amount)															\
		asm(																						\
			"psraw %[Result],%[Value],%c[ShiftAmount]"												\
			: [Result] "=r" (result)																\
			: [Value] "r" (value), [ShiftAmount] "n" (shift_amount)									\
		)

	/// @brief PSRAVW : Parallel Shift Right Arithmetic Variable Word
	/// 
	/// Treat 128-bit packed integer data as 2 sign-extended 32-bit values. Shift each value
	/// right while shifting in sign bits by an amount equal to the corresponding value in
	/// 'shift_amount'. Only the low 5 bits in each value of 'shift_amount' are used, others
	/// are ignored.
	/// @param value Values to shift
	/// @param shift_amount Amount to shift values by
	/// @return Values shifted right by variable amount
	FORCEINLINE CONST m128i64 mm_srav_epi64(m128i64 value, m128u64 shift_amount)
	{
		m128i64 result;

		asm(
			"psravw %[Result],%[Value],%[Amount]"
			: [Result] "=r" (result)
			: [Value] "r" (value), [Amount] "r" (shift_amount)
		);

		return result;
	}

	/// @brief PSRLH : Parallel Shift Right Logical Halfword
	/// 
	/// Logically right shift 16-bit values. Shifts in '0's into the upper bits.
	/// @param result Variable identifier to store result to. Must be of type 'm128u16'
	/// @param value Variable identifier to use as source value. Must be of type 'm128u16'
	/// @param shift_value Amount of bits to shift the source value right. Must be in range [0, 15]
#define PSRLH(result, value, shift_amount)															\
		asm(																						\
			"psrlh %[Result],%[Value],%c[ShiftAmount]"												\
			: [Result] "=r" (result)																\
			: [Value] "r" (value), [ShiftAmount] "n" (shift_amount)									\
		)

	/// @brief PSRLW : Parallel Shift Right Logical Word
	/// 
	/// Logically right shift 32-bit values. Shifts in '0's into the upper bits.
	/// @param result Variable identifier to store result to. Must be of type 'm128u32'
	/// @param value Variable identifier to use as source value. Must be of type 'm128u32'
	/// @param shift_value Amount of bits to shift the source value left. Must be in range [0, 31]
#define PSRAW(result, value, shift_amount)															\
		asm(																						\
			"psrlw %[Result],%[Value],%c[ShiftAmount]"												\
			: [Result] "=r" (result)																\
			: [Value] "r" (value), [ShiftAmount] "n" (shift_amount)									\
		)

	/// @brief PSRAVW : Parallel Shift Right Logical Variable Word
	/// 
	/// Treat 128-bit packed integer data as 2 sign-extended 32-bit values. Shift each value
	/// right while shifting in '0's by an amount equal to the corresponding value in
	/// 'shift_amount'. Only the low 5 bits in each value of 'shift_amount' are used, others
	/// are ignored.
	/// @param value Values to shift
	/// @param shift_amount Amount to shift values by
	/// @return Values shifted right by variable amount
	FORCEINLINE CONST m128u64 mm_srlv_epu64(m128u64 value, m128u64 shift_amount)
	{
		m128u64 result;

		asm(
			"psrlvw %[Result],%[Value],%[Amount]"
			: [Result] "=r" (result)
			: [Value] "r" (value), [Amount] "r" (shift_amount)
		);

		return result;
	}


	/// @brief PABSH : Parallel ABSolute Halfword
	/// 
	/// Calculate the absolute value of 8 16-bit signed values.
	/// 
	/// If any of the values is '0x8000' then the result will be truncated to '0x7FFF', or one
	/// less than the accurate absolute value.
	/// @param v Values to calculate the absolute value of
	/// @return Absolute values of argument with truncation
	FORCEINLINE CONST m128i16 mm_abs_epi16(m128i16 v)
	{
		m128i16 result;

		asm(
			"pabsh %[Result],%[Value]"
			: [Result] "=r" (result)
			: [Value] "r" (v)
		);

		return result;
	}

	/// @brief PABSW : Parallel ABSolute Word
	/// 
	/// Calculate the absolute value of 4 32-bit signed values.
	/// 
	/// If any of the values is '0x80000000' then the result will be truncated to '0x7FFFFFFF', or
	/// one less than the accurate absolute value.
	/// @param v Values to calculate the absolute value of
	/// @return Absolute values of argument with truncation
	FORCEINLINE CONST m128i32 mm_abs_epi32(m128i32 v)
	{
		m128i32 result;

		asm(
			"pabsw %[Result],%[Value]"
			: [Result] "=r" (result)
			: [Value] "r" (v)
		);

		return result;
	}

	/// @brief PMAXH : Parallel MAXimum Halfword
	/// 
	/// Calculate the maximum value of 8 16-bit signed value pairs.
	/// 
	/// Unsigned maximum can be achieved by first adding '0x8000' to both operands and then
	/// subtracting it again after.
	/// @param l First operand
	/// @param r Second operand
	/// @return Maximum value of each value pair
	FORCEINLINE CONST m128i16 mm_max_epi16(m128i16 l, m128i16 r)
	{
		m128i16 result;

		asm(
			"pmaxh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PMAXW : Parallel MAXimum Word
	/// 
	/// Calculate the maximum value of 4 32-bit signed value pairs.
	/// 
	/// Unsigned maximum can be achieved by first adding '0x80000000' to both operands and then
	/// subtracting it again after.
	/// @param l First operand
	/// @param r Second operand
	/// @return Maximum value of each value pair
	FORCEINLINE CONST m128i32 mm_max_epi32(m128i32 l, m128i32 r)
	{
		m128i32 result;

		asm(
			"pmaxw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PMINH : Parallel MINimum Halfword
	/// 
	/// Calculate the minimum value of 8 16-bit signed value pairs.
	/// 
	/// Unsigned minimum can be achieved by first adding '0x8000' to both operands and then
	/// subtracting it again after.
	/// @param l First operand
	/// @param r Second operand
	/// @return Minimum value of each value pair
	FORCEINLINE CONST m128i16 mm_min_epi16(m128i16 l, m128i16 r)
	{
		m128i16 result;

		asm(
			"pminh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PMINW : Parallel MINimum Word
	/// 
	/// Calculate the minimum value of 4 32-bit signed value pairs.
	/// 
	/// Unsigned minimum can be achieved by first adding '0x80000000' to both operands and then
	/// subtracting it again after.
	/// @param l First operand
	/// @param r Second operand
	/// @return Minimum value of each value pair
	FORCEINLINE CONST m128i32 mm_min_epi32(m128i32 l, m128i32 r)
	{
		m128i32 result;

		asm(
			"pminw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}


	/// @brief PADDB : Parallel ADD Byte
	/// 
	/// Add 16 8-bit value pairs.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of addition of both operands
	FORCEINLINE CONST m128i8 mm_add_epi8(m128i8 l, m128i8 r)
	{
		m128i8 result;

		asm(
			"paddb %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PADDB : Parallel ADD Byte
	/// 
	/// Add 16 8-bit value pairs.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of addition of both operands
	FORCEINLINE CONST m128u8 mm_add_epu8(m128u8 l, m128u8 r)
	{
		m128u8 result;

		asm(
			"paddb %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}
	
	/// @brief PADDH : Parallel ADD Halfword
	/// 
	/// Add 8 16-bit value pairs.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of addition of both operands
	FORCEINLINE CONST m128i16 mm_add_epi16(m128i16 l, m128i16 r)
	{
		m128i16 result;

		asm(
			"paddh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PADDH : Parallel ADD Halfword
	/// 
	/// Add 8 16-bit value pairs.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of addition of both operands
	FORCEINLINE CONST m128u16 mm_add_epu16(m128u16 l, m128u16 r)
	{
		m128u16 result;

		asm(
			"paddh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PADDW : Parallel ADD Word
	/// 
	/// Add 4 32-bit value pairs.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of addition of both operands
	FORCEINLINE CONST m128i32 mm_add_epi32(m128i32 l, m128i32 r)
	{
		m128i32 result;

		asm(
			"paddw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PADDW : Parallel ADD Word
	/// 
	/// Add 4 32-bit value pairs.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of addition of both operands
	FORCEINLINE CONST m128u32 mm_add_epu32(m128u32 l, m128u32 r)
	{
		m128u32 result;

		asm(
			"paddw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PADDSB : Parallel ADD Signed saturation Byte
	/// 
	/// Add 16 8-bit value pairs. Saturate the result to the extreme values of int8_t instead of
	/// wraparound.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of signed saturated addition of both operands
	FORCEINLINE CONST m128i8 mm_adds_epi8(m128i8 l, m128i8 r)
	{
		m128i8 result;

		asm(
			"paddsb %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PADDUB : Parallel ADD Unsigned saturation Byte
	/// 
	/// Add 16 8-bit value pairs. Saturate the result to the extreme values of uint8_t instead of
	/// wraparound.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of unsigned saturated addition of both operands
	FORCEINLINE CONST m128u8 mm_adds_epu8(m128u8 l, m128u8 r)
	{
		m128u8 result;

		asm(
			"paddub %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PADDSH : Parallel ADD Signed saturation Halfword
	/// 
	/// Add 8 16-bit value pairs. Saturate the result to the extreme values of int16_t instead of
	/// wraparound.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of signed saturated addition of both operands
	FORCEINLINE CONST m128i16 mm_adds_epi16(m128i16 l, m128i16 r)
	{
		m128i16 result;

		asm(
			"paddsh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PADDUH : Parallel ADD Unsigned saturation Halfword
	/// 
	/// Add 8 16-bit value pairs. Saturate the result to the extreme values of uint16_t instead of
	/// wraparound.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of unsigned saturated addition of both operands
	FORCEINLINE CONST m128u16 mm_adds_epu16(m128u16 l, m128u16 r)
	{
		m128u16 result;

		asm(
			"padduh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PADDSW : Parallel ADD Signed saturation Word
	/// 
	/// Add 4 32-bit value pairs. Saturate the result to the extreme values of int32_t instead of
	/// wraparound.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of signed saturated addition of both operands
	FORCEINLINE CONST m128i32 mm_adds_epi32(m128i32 l, m128i32 r)
	{
		m128i32 result;

		asm(
			"paddsw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PADDUW : Parallel ADD Unsigned saturation Word
	/// 
	/// Add 4 32-bit value pairs. Saturate the result to the extreme values of uint32_t instead of
	/// wraparound.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of unsigned saturated addition of both operands
	FORCEINLINE CONST m128u32 mm_adds_epu32(m128u32 l, m128u32 r)
	{
		m128u32 result;

		asm(
			"padduw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}


	/// @brief PSUBB : Parallel SUBtract Byte
	/// 
	/// Subtract 16 8-bit value pairs.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of subtraction of both operands
	FORCEINLINE CONST m128i8 mm_sub_epi8(m128i8 l, m128i8 r)
	{
		m128i8 result;

		asm(
			"psubb %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PSUBB : Parallel SUBtract Byte
	/// 
	/// Subtract 16 8-bit value pairs.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of subtraction of both operands
	FORCEINLINE CONST m128u8 mm_sub_epu8(m128u8 l, m128u8 r)
	{
		m128u8 result;

		asm(
			"psubb %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PSUBH : Parallel SUBtract Halfword
	/// 
	/// Subtract 8 16-bit value pairs.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of subtraction of both operands
	FORCEINLINE CONST m128i16 mm_sub_epi16(m128i16 l, m128i16 r)
	{
		m128i16 result;

		asm(
			"psubh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PSUBH : Parallel SUBtract Halfword
	/// 
	/// Subtract 8 16-bit value pairs.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of subtraction of both operands
	FORCEINLINE CONST m128u16 mm_sub_epu16(m128u16 l, m128u16 r)
	{
		m128u16 result;

		asm(
			"psubh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PSUBW : Parallel SUBtract Word
	/// 
	/// Subtract 4 32-bit value pairs.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of subtraction of both operands
	FORCEINLINE CONST m128i32 mm_sub_epi32(m128i32 l, m128i32 r)
	{
		m128i32 result;

		asm(
			"psubw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PSUBW : Parallel SUBtract Word
	/// 
	/// Subtract 4 32-bit value pairs.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of subtraction of both operands
	FORCEINLINE CONST m128u32 mm_sub_epu32(m128u32 l, m128u32 r)
	{
		m128u32 result;

		asm(
			"psubw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PSUBSB : Parallel SUBtract Signed saturation Byte
	/// 
	/// Subtract 16 8-bit value pairs. Saturate the result to the extreme values of int8_t instead
	/// of wraparound.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of signed saturated subtraction of both operands
	FORCEINLINE CONST m128i8 mm_subs_epi8(m128i8 l, m128i8 r)
	{
		m128i8 result;

		asm(
			"psubsb %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PSUBUB : Parallel SUBtract Unsigned saturation Byte
	/// 
	/// Subtract 16 8-bit value pairs. Saturate the result to the extreme values of uint8_t
	/// instead of wraparound.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of unsigned saturated subtraction of both operands
	FORCEINLINE CONST m128u8 mm_subs_epu8(m128u8 l, m128u8 r)
	{
		m128u8 result;

		asm(
			"psubub %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PSUBSH : Parallel SUBtract Signed saturation Halfword
	/// 
	/// Subtract 8 16-bit value pairs. Saturate the result to the extreme values of int16_t
	/// instead of wraparound.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of signed saturated subtraction of both operands
	FORCEINLINE CONST m128i16 mm_subs_epi16(m128i16 l, m128i16 r)
	{
		m128i16 result;

		asm(
			"psubsh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PSUBUH : Parallel SUBtract Unsigned saturation Halfword
	/// 
	/// Subtract 8 16-bit value pairs. Saturate the result to the extreme values of uint16_t
	/// instead of wraparound.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of unsigned saturated addition of both operands
	FORCEINLINE CONST m128u16 mm_subs_epu16(m128u16 l, m128u16 r)
	{
		m128u16 result;

		asm(
			"psubuh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PSUBSW : Parallel SUBtract Signed saturation Word
	/// 
	/// Subtract 4 32-bit value pairs. Saturate the result to the extreme values of int32_t
	/// instead of wraparound.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of signed saturated subtraction of both operands
	FORCEINLINE CONST m128i32 mm_subs_epi32(m128i32 l, m128i32 r)
	{
		m128i32 result;

		asm(
			"psubsw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PSUBUW : Parallel SUBtract Unsigned saturation Word
	/// 
	/// Subtract 4 32-bit value pairs. Saturate the result to the extreme values of uint32_t
	/// instead of wraparound.
	/// @param l First operand
	/// @param r Second operand
	/// @return Result of unsigned saturated subtraction of both operands
	FORCEINLINE CONST m128u32 mm_subs_epu32(m128u32 l, m128u32 r)
	{
		m128u32 result;

		asm(
			"psubuw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "r" (l), [Right] "r" (r)
		);

		return result;
	}


	/// @brief PMULTH : Parallel MULTiply Halfword
	/// 
	/// Multiply 8 pairs of 16-bit signed values.
	/// 
	/// Let each pair have a signed 32-bit product named p0..p7. Store p0, p2, p4 and p6 into the
	/// return value. Store p0, p1, p4 and p5 into LO and p2, p3, p6 and p7 into HI.
	/// 
	/// Bitwise result:
	///		Return value:	[ 31,   0] = p0
	///						[ 63,  32] = p2
	///						[ 95,  64] = p4
	///						[127,  96] = p6
	///		LO:				[ 31,   0] = p0
	///						[ 63,  32] = p1
	///						[ 95,  64] = p4
	///						[127,  96] = p5
	///		HI:				[ 31,   0] = p2
	///						[ 63,  32] = p3
	///						[ 95,  64] = p6
	///						[127,  96] = p7
	/// 
	/// Multiplication happens asynchronously. Reading from the return value, the LO or the HI
	/// register will stall the EE Core until the result is ready.
	/// 
	/// You can extract only the products not in the return value using 'mm_loadlohi_upper_epi32'.
	/// 
	/// This function writes to global state (LO/HI).
	/// @param l First operand
	/// @param r Second operand
	/// @return Multiplication result of value pairs in even positions
	FORCEINLINE m128i32 mm_mul_epi16(m128i16 l, m128i16 r)
	{
		m128i32 result;

		// volatile because writes to LO/HI
		asm volatile(
			"pmulth %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PMULTW : Parallel MULTiply Word
	/// 
	/// Multiply 2 pairs of signed 32-bit values. The input operands are treated as sign-extended
	/// 32-bit values. The 64-bit products are written to the return value. The low 32 bits of
	/// the products are written to even positions of the LO register and sign-extended. The high
	/// 32 bits are written to even positions of the HI register and sign extended.
	/// 
	/// Multiplication happens asynchronously. Reading from the return value, the LO or the HI
	/// register will stall the EE Core until the result is ready.
	/// 
	/// This function writes to global state (LO/HI).
	/// @param l First operand
	/// @param r Second operand
	/// @return Multiplication result containing full 64-bit signed values
	FORCEINLINE m128i64 mm_mul_epi64(m128i64 l, m128i64 r)
	{
		m128i64 result;

		// volatile because writes to LO/HI
		asm volatile(
			"pmultw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PMULTUW : Parallel MULTiply Unsigned Word
	/// 
	/// Multiply 2 pairs of unsigned 32-bit values. The input operands are treated as sign-
	/// extended 32-bit values. The 64-bit products are written to the return value. The low 32
	/// bits of the products are written to even positions of the LO register and sign-extended.
	/// The high 32 bits are written to even positions of the HI register and sign extended.
	/// 
	/// Multiplication happens asynchronously. Reading from the return value, the LO or the HI
	/// register will stall the EE Core until the result is ready.
	/// 
	/// This function writes to global state (LO/HI).
	/// @param l First operand
	/// @param r Second operand
	/// @return Multiplication result containing full 64-bit unsigned values
	FORCEINLINE m128u64 mm_mul_epu64(m128u64 l, m128u64 r)
	{
		m128u64 result;

		// volatile because writes to LO/HI
		asm volatile(
			"pmultuw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PMADDH : Parallel Multiply-ADD Halfword
	/// 
	/// Multiply 8 pairs of 16-bit signed values and accumulate.
	/// 
	/// Let each pair have a signed 32-bit product named p0..p7. Assign each product a 32-bit
	/// accumulator a0..a7 according to the output locations in LO/HI of 'mm_mul_epi16'. Add
	/// the product to the accumulator and store a0, a2, a4 and a6 into the return value. Store
	/// a0, a1, a4 and a5 into LO and a2, a3, a6 and a7 into HI.
	/// 
	/// Bitwise accumulation:
	///		LO:				[ 31,   0] = a0
	///						[ 63,  32] = a1
	///						[ 95,  64] = a4
	///						[127,  96] = a5
	///		HI:				[ 31,   0] = a2
	///						[ 63,  32] = a3
	///						[ 95,  64] = a6
	///						[127,  96] = a7
	///		Return value:	[ 31,   0] = LO[ 31,   0]
	///						[ 63,  32] = HI[ 31,   0]
	///						[ 95,  64] = LO[ 95,  64]
	///						[127,  96] = HI[ 95,  64]
	/// 
	/// Multiplication happens asynchronously. Reading from the return value, the LO or the HI
	/// register will stall the EE Core until the result is ready.
	/// 
	/// You can extract only the accumulators not in the return value using
	/// 'mm_loadlohi_upper_epi32'.
	/// 
	/// This function writes to global state (LO/HI).
	/// @param l First operand
	/// @param r Second operand
	/// @return Multipy-accumulate result of value pairs in even positions
	FORCEINLINE m128i32 mm_fma_epi16(m128i16 l, m128i16 r)
	{
		m128i32 result;

		// volatile because writes to LO/HI
		asm volatile(
			"pmaddh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PMADDW : Parallel Multiply-ADD Word
	/// 
	/// Multiply 2 pairs of signed 32-bit values and accumulate. The input operands are treated
	/// as sign-extended 32-bit values. Assign each product a 64-bit accumulator a0..a1 in LO/HI
	/// according to the output locations of 'mm_mul_epi64'. The products are added to the
	/// accumulator and written to the return value. The low 32 bits of the accumulators are
	/// written to even positions of the LO register and sign-extended. The high 32 bits are
	/// written to even positions of the HI register and sign extended.
	/// 
	/// Multiplication happens asynchronously. Reading from the return value, the LO or the HI
	/// register will stall the EE Core until the result is ready.
	/// 
	/// This function writes to global state (LO/HI).
	/// @param l First operand
	/// @param r Second operand
	/// @return Multiply-accumulate result containing full 64-bit signed values
	FORCEINLINE m128i64 mm_fma_epi64(m128i64 l, m128i64 r)
	{
		m128i64 result;

		// volatile because writes to LO/HI
		asm volatile(
			"pmaddw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
			);

		return result;
	}

	/// @brief PMADDUW : Parallel Multiply-ADD Unsigned Word
	/// 
	/// Multiply 2 pairs of unsigned 32-bit values and accumulate. The input operands are treated
	/// as sign-extended 32-bit values. Assign each product a 64-bit accumulator a0..a1 in LO/HI
	/// according to the output locations of 'mm_mul_epu64'. The products are added to the
	/// accumulator and written to the return value. The low 32 bits of the accumulators are
	/// written to even positions of the LO register and sign-extended. The high 32 bits are
	/// written to even positions of the HI register and sign extended.
	/// 
	/// Multiplication happens asynchronously. Reading from the return value, the LO or the HI
	/// register will stall the EE Core until the result is ready.
	/// 
	/// This function writes to global state (LO/HI).
	/// @param l First operand
	/// @param r Second operand
	/// @return Multiply-accumulate result containing full 64-bit unsigned values
	FORCEINLINE m128u64 mm_fma_epu64(m128u64 l, m128u64 r)
	{
		m128u64 result;

		// volatile because writes to LO/HI
		asm volatile(
			"pmadduw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
			);

		return result;
	}

	/// @brief PMSUBH : Parallel Multiply-SUBtract Halfword
	/// 
	/// Works exactly as 'mm_fma_epi16' but subtracts the product from the accumulators instead.
	/// 
	/// Multiplication happens asynchronously. Reading from the return value, the LO or the HI
	/// register will stall the EE Core until the result is ready.
	/// 
	/// You can extract only the accumulators not in the return value using
	/// 'mm_loadlohi_upper_epi32'.
	/// 
	/// This function writes to global state (LO/HI).
	/// @param l First operand
	/// @param r Second operand
	/// @return Multipy-accumulate result of value pairs in even positions
	FORCEINLINE m128i32 mm_fms_epi16(m128i16 l, m128i16 r)
	{
		m128i32 result;

		// volatile because writes to LO/HI
		asm volatile(
			"pmsubh %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PMSUBW : Parallel Multiply-SUBtract Word
	/// 
	/// Works exactly as 'mm_fma_epi64' but subtracts the product from the accumulators instead.
	/// 
	/// Multiplication happens asynchronously. Reading from the return value, the LO or the HI
	/// register will stall the EE Core until the result is ready.
	/// 
	/// This function writes to global state (LO/HI).
	/// @param l First operand
	/// @param r Second operand
	/// @return Multiply-accumulate result containing full 64-bit signed values
	FORCEINLINE m128i64 mm_fms_epi64(m128i64 l, m128i64 r)
	{
		m128i64 result;

		// volatile because writes to LO/HI
		asm volatile(
			"pmsubw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}

	/// @brief PMADDUW : Parallel Multiply-ADD Unsigned Word
	/// 
	/// Works exactly as 'mm_fma_epu64' but subtracts the product from the accumulators instead.
	/// 
	/// Multiplication happens asynchronously. Reading from the return value, the LO or the HI
	/// register will stall the EE Core until the result is ready.
	/// 
	/// This function writes to global state (LO/HI).
	/// @param l First operand
	/// @param r Second operand
	/// @return Multiply-accumulate result containing full 64-bit unsigned values
	FORCEINLINE m128u64 mm_fms_epu64(m128u64 l, m128u64 r)
	{
		m128u64 result;

		// volatile because writes to LO/HI
		asm volatile(
			"pmsubuw %[Result],%[Left],%[Right]"
			: [Result] "=r" (result)
			: [Left] "%r" (l), [Right] "r" (r)
		);

		return result;
	}


	/// @brief PDIVW : Parallel DIVide Word
	/// 
	/// Treat each operand as containing 2 32-bit sign-extended values. Divide each pair and store
	/// the quotients in the LO register and the remainder in the HI register as sign-extended 32-
	/// bit values. See 'divrem0_i32_start' for further information regarding result signedness
	/// and overflow/div-by-0 behavior.
	/// 
	/// As integer division is done asynchronously on the EE Core you should issue the division
	/// instruction before checking for erroneous inputs like div-by-0 to improve throughput.
	/// Reading from the result registers (LO/HI) will cause a stall if the result is not ready at
	/// that point in time.
	/// 
	/// This function writes to global state (LO/HI).
	/// @param dividend Values to divide
	/// @param divisor Values to divide by
	FORCEINLINE void mm_divrem_epi64(m128i64 dividend, m128i64 divisor)
	{
		// volatile because writes to LO/HI
		asm volatile(
			"pdivw %[Dividend],%[Divisor]"
			:
			: [Dividend] "r" (dividend), [Divisor] "r" (divisor)
		);
	}

	/// @brief PDIVUW : Parallel DIVide Unsigned Word
	/// 
	/// Treat each operand as containing 2 32-bit sign-extended values. Divide each pair and store
	/// the quotients in the LO register and the remainder in the HI register as sign-extended 32-
	/// bit values. See 'divrem0_u32_start' for further information regarding overflow/div-by-0
	/// behavior.
	/// 
	/// As integer division is done asynchronously on the EE Core you should issue the division
	/// instruction before checking for erroneous inputs like div-by-0 to improve throughput.
	/// Reading from the result registers (LO/HI) will cause a stall if the result is not ready at
	/// that point in time.
	/// 
	/// This function writes to global state (LO/HI).
	/// @param dividend Values to divide
	/// @param divisor Values to divide by
	FORCEINLINE void mm_divrem_epu64(m128u64 dividend, m128u64 divisor)
	{
		// volatile because writes to LO/HI
		asm volatile(
			"pdivuw %[Dividend],%[Divisor]"
			:
			: [Dividend] "r" (dividend), [Divisor] "r" (divisor)
		);
	}

	/// @brief PDIVBW : Parallel DIVide Broadcast Word
	/// 
	/// Divide 4 32-bit signed values by a single signed 16-bit value. Store the four quotients
	/// to the LO register and the remainders to the HI register.
	/// 
	/// As integer division is done asynchronously on the EE Core you should issue the division
	/// instruction before checking for erroneous inputs like div-by-0 to improve throughput.
	/// Reading from the result registers (LO/HI) will cause a stall if the result is not ready at
	/// that point in time.
	/// 
	/// This function writes to global state (LO/HI).
	/// @param dividend Values to divide
	/// @param divisor Value to divide by
	FORCEINLINE void mm_divremb_epi32(m128i32 dividend, int16_t divisor)
	{
		// volatile because writes to LO/HI
		asm volatile(
			"pdivbw %[Dividend],%[Divisor]"
			:
			: [Dividend] "r" (dividend), [Divisor] "r" (divisor)
		);
	}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#endif

#undef FORCEINLINE
#undef CONST
#undef UNSEQUENCED
#undef PURE
#undef REPRODUCIBLE
#undef ALIGNAS
#undef ALIGNVECTOR
#undef MODE_TI
