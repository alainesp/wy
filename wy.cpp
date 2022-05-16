/////////////////////////////////////////////////////////////////////////////////
// This file is a C++ wrapper around wyhash: 
// https://github.com/wangyi-fudan/wyhash
// 
// Copyright (c) 2022 by Alain Espinosa.
/////////////////////////////////////////////////////////////////////////////////

#include "wy.hpp"

// Define 'byteswap64(uint64_t)' needed for 'rand::generate_stream(size_t)'
#if !WYHASH_LITTLE_ENDIAN
	#include <version>
	#ifdef __cpp_lib_byteswap
		#include <bit>
		#define byteswap64(v) std::byteswap(x)
	#elif defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
		#define byteswap64(v) __builtin_bswap64(v)
	#elif defined(_MSC_VER)
		#define byteswap64(v) _byteswap_uint64(v)
	#else
		static inline uint64_t byteswap64(uint64_t v) noexcept
		{
			v = ((v & 0x00000000FFFFFFFFull) << 32) | ((v & 0xFFFFFFFF00000000ull) >> 32);
			v = ((v & 0x0000FFFF0000FFFFull) << 16) | ((v & 0xFFFF0000FFFF0000ull) >> 16);
			v = ((v & 0x00FF00FF00FF00FFull) <<  8) | ((v & 0xFF00FF00FF00FF00ull) >>  8);
			return v;
		}
	#endif
#endif

namespace wy {

	void rand::generate_stream(std::vector<uint8_t>& vec, size_t size) noexcept
	{
		size_t sizeOf64 = (size + sizeof(uint64_t) - 1) / sizeof(uint64_t); // The number of 64-bits numbers to generate

		// Create the memory on the vector
		vec.resize(sizeOf64 * sizeof(uint64_t), 0);
		uint8_t* dataPtr = vec.data();

		// Generate random values
		for (size_t i = 0; i < sizeOf64; i++)
		{
#if WYHASH_LITTLE_ENDIAN
			uint64_t val = operator()();
#else
			uint64_t val = byteswap64(operator()());
#endif
			memcpy(dataPtr + i * sizeof(uint64_t), &val, sizeof(uint64_t));
		}

		// Final size
		vec.resize(size);
	}
};
