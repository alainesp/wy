/////////////////////////////////////////////////////////////////////////////////
// This file is a C++ wrapper around wyhash: 
// https://github.com/wangyi-fudan/wyhash
// 
// Copyright (c) 2022 by Alain Espinosa.
/////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <random>
#include "wyhash.h"
#include "wy.hpp"

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
