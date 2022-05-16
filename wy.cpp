/////////////////////////////////////////////////////////////////////////////////
// This file is a C++ wrapper around wyhash: 
// https://github.com/wangyi-fudan/wyhash
// 
// Copyright (c) 2022 by Alain Espinosa.
/////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <random>
#include <memory>
#include "wyhash.h"
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

	rand::rand(uint64_t seed) noexcept : state(seed)
	{}
	rand::rand() noexcept
	{
		// Seed with a real random value, if available
		std::random_device rd;
		state = static_cast<uint64_t>(rd()) | static_cast<uint64_t>(rd()) << 32;
	}
	uint64_t rand::operator()() noexcept
	{
		return wyrand(&state);
	}
	double rand::uniform_dist() noexcept
	{
		return wy2u01(operator()());
	}
	double rand::uniform_dist(double min_value, double max_value) noexcept
	{
		assert(max_value > min_value);

		return uniform_dist() * (max_value - min_value) + min_value;
	}
	uint64_t rand::uniform_dist(uint64_t max_value) noexcept
	{
		return wy2u0k(operator()(), max_value);
	}
	double rand::gaussian_dist() noexcept
	{
		return wy2gau(operator()());
	}
	double rand::gaussian_dist(double mean, double std) noexcept
	{
		assert(std > 0);

		return gaussian_dist() * std + mean;
	}
	std::vector<uint8_t> rand::generate_stream(size_t size) noexcept
	{
		size_t sizeOf64 = (size + sizeof(uint64_t) - 1) / sizeof(uint64_t); // The number of 64-bits numbers to generate
		std::unique_ptr<uint64_t[]> buffer = std::make_unique<uint64_t[]>(sizeOf64);

		// Generate random values
		for (size_t i = 0; i < sizeOf64; i++)
#if WYHASH_LITTLE_ENDIAN
			buffer[i] = operator()();
#else
			buffer[i] = byteswap64(operator()());
#endif
		// Convert to bytes
		return std::vector<uint8_t>(reinterpret_cast<uint8_t*>(buffer.get()), reinterpret_cast<uint8_t*>(buffer.get()) + size);
	}

	namespace internal {
		// Hash
		hash_imp::hash_imp() noexcept : secret{ 0xa0761d6478bd642full, 0xe7037ed1a0b428dbull, 0x8ebc6af09c88c6e3ull, 0x589965cc75374cc3ull }
		{}
		hash_imp::hash_imp(uint64_t seed) noexcept
		{
			make_secret(seed, secret);
		}
		hash_imp::hash_imp(const uint64_t psecret[4]) noexcept
		{
			memcpy(secret, psecret, sizeof(secret));
		}
		uint64_t hash_imp::wyhash(const uint8_t* data, size_t len) const noexcept
		{
			return ::wyhash(data, len, 0, secret);
		}
		uint64_t hash_imp::wyhash(uint64_t data) const noexcept
		{
			return ::wyhash64(data, secret[0]);
		}
	};
};
