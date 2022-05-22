/////////////////////////////////////////////////////////////////////////////////
// This file is a C++ wrapper around wyhash: 
// https://github.com/wangyi-fudan/wyhash
// 
// Copyright (c) 2022 by Alain Espinosa.
/////////////////////////////////////////////////////////////////////////////////

// include the required header
#include <wy.hpp>
#include <gtest/gtest.h>
#include "wyhash.h"

/////////////////////////////////////////////////////////////////////////////////
/// General
/////////////////////////////////////////////////////////////////////////////////
TEST(wyrand, Endianness)
{
	uint64_t val = 0x0102'0304'0506'0708;
	uint8_t byteArray[8];

	memcpy(byteArray, &val, sizeof(byteArray));

	for (size_t i = 0; i < 8; i++)
	{
#if WYHASH_LITTLE_ENDIAN
		ASSERT_EQ(byteArray[i], (val >> i * 8) & 0xff);
#else
		ASSERT_EQ(byteArray[7 - i], (val >> i * 8) & 0xff);
#endif
	}
}

#if !WYHASH_LITTLE_ENDIAN
TEST(wyrand, Byteswap)
{
	uint64_t val64 = 0x0102'0304'0506'0708;
	uint64_t swap64 = byteswap64(val64);	
	for (size_t i = 0; i < 8; i++)
	{
		ASSERT_EQ((swap64 >> (7 - i) * 8) & 0xff, (val64 >> i * 8) & 0xff);
	}
	ASSERT_EQ(val64, byteswap64(swap64));

	uint32_t val32 = 0x0102'0304;
	uint32_t swap32 = byteswap32(val32);
	for (size_t i = 0; i < 4; i++)
	{
		ASSERT_EQ((swap32 >> (3 - i) * 8) & 0xff, (val32 >> i * 8) & 0xff);
	}
	ASSERT_EQ(val32, byteswap32(swap32));
}
#endif

/////////////////////////////////////////////////////////////////////////////////
/// Pseudo-random number generation
/////////////////////////////////////////////////////////////////////////////////
TEST(wyrand, Constructors)
{
	wy::rand r0;
	ASSERT_NE(r0.state, 0);// This may occurr, with low probability, but checking for bad compilations

	uint64_t seed = 0x4458adf548;
	wy::rand r1(seed);
	ASSERT_EQ(r1.state, seed);
}

TEST(wyrand, UniformRandomBitGenerator)
{
	ASSERT_EQ(typeid(wy::rand::result_type), typeid(uint64_t));
	ASSERT_EQ(wy::rand::min(), 0);
	ASSERT_EQ(wy::rand::max(), std::numeric_limits<uint64_t>::max());
}

TEST(wyrand, Operator)
{
	for (uint64_t seed = 0; seed < 1'000'000; seed++)
	{
		wy::rand r(seed);

		uint64_t originalSeed = seed;
		ASSERT_EQ(r(), wyrand(&originalSeed));
	}
}

TEST(wyrand, Uniform)
{
	for (uint64_t seed = 0; seed < 1'000'000; seed++)
	{
		wy::rand r(seed);
		uint64_t originalSeed = seed;

		double val = r.uniform_dist();
		ASSERT_GE(val, 0);
		ASSERT_LT(val, 1);
		ASSERT_EQ(val, wy2u01(wyrand(&originalSeed)));

		val = r.uniform_dist(-1.2, -1);
		ASSERT_GE(val, -1.2);
		ASSERT_LT(val, -1);
		ASSERT_EQ(val, -1.2 + (-1 + 1.2) * wy2u01(wyrand(&originalSeed)));

#if !WYHASH_32BIT_MUM
		uint64_t valk = r.uniform_dist(500);
		ASSERT_GE(valk, 0);
		ASSERT_LT(valk, 500);
		ASSERT_EQ(valk, wy2u0k(wyrand(&originalSeed), 500));
#endif
	}
}

TEST(wyrand, Gaussian)
{
	for (uint64_t seed = 0; seed < 1'000'000; seed++)
	{
		wy::rand r(seed);
		uint64_t originalSeed = seed;

		double val = r.gaussian_dist();
		ASSERT_EQ(val, wy2gau(wyrand(&originalSeed)));

		val = r.gaussian_dist(1.1, 2.3);
		ASSERT_EQ(val, 1.1 + 2.3 * wy2gau(wyrand(&originalSeed)));
	}
}

struct StuctTest {
	char buffer[3];
};
TEST(wyrand, Stream)
{
	for (uint64_t seed = 0; seed < 100'000; seed++)
	{
		wy::rand r(seed);
		wy::rand r0(seed);

		std::vector<uint8_t> stream = r.generate_stream(0);
		ASSERT_EQ(stream.size(), 0);

		stream = r.generate_stream(10);
		ASSERT_EQ(stream.size(), 10);
		ASSERT_EQ(((uint64_t*)stream.data())[0], r0());
		ASSERT_EQ(((uint64_t*)stream.data())[1] & 0xffff, r0() & 0xffff);

		r.generate_stream(stream, 20);
		ASSERT_EQ(stream.size(), 20);
		ASSERT_EQ(((uint64_t*)stream.data())[0], r0());
		ASSERT_EQ(((uint64_t*)stream.data())[1], r0());
		ASSERT_EQ(((uint64_t*)stream.data())[2] & 0xffffffff, r0() & 0xffffffff);

		std::vector<int16_t> stream16 = r.generate_stream<int16_t>(1);
		ASSERT_EQ(stream16.size(), 1);
		ASSERT_EQ(((uint64_t*)stream16.data())[0] & 0xffff, r0() & 0xffff);

		r.generate_stream(stream16, 20);
		ASSERT_EQ(stream16.size(), 20);
		ASSERT_EQ(((uint64_t*)stream16.data())[0], r0());
		ASSERT_EQ(((uint64_t*)stream16.data())[1], r0());
		ASSERT_EQ(((uint64_t*)stream16.data())[2], r0());
		ASSERT_EQ(((uint64_t*)stream16.data())[3], r0());
		ASSERT_EQ(((uint64_t*)stream16.data())[4], r0());

		std::vector<StuctTest> streamStruct = r.generate_stream<StuctTest>(3);
		ASSERT_EQ(streamStruct.size(), 3);
		ASSERT_EQ(((uint64_t*)streamStruct.data())[0], r0());
		ASSERT_EQ(((uint64_t*)streamStruct.data())[1] & 0xff, r0() & 0xff);
	}
}

/////////////////////////////////////////////////////////////////////////////////
/// Hasher
/////////////////////////////////////////////////////////////////////////////////
TEST(wyhash, BaseImplementation)
{
	wy::internal::hash_imp h;
	ASSERT_EQ(h.secret[0], 0xa0761d6478bd642full);
	ASSERT_EQ(h.secret[1], 0xe7037ed1a0b428dbull);
	ASSERT_EQ(h.secret[2], 0x8ebc6af09c88c6e3ull);
	ASSERT_EQ(h.secret[3], 0x589965cc75374cc3ull);

	for (uint64_t seed = 0; seed < 10'000; seed++)
	{
		wy::internal::hash_imp h1(seed);

		uint64_t secret[4];
		make_secret(seed, secret);
		ASSERT_EQ(h1.secret[0], secret[0]);
		ASSERT_EQ(h1.secret[1], secret[1]);
		ASSERT_EQ(h1.secret[2], secret[2]);
		ASSERT_EQ(h1.secret[3], secret[3]);

		wy::internal::hash_imp h2(secret);
		ASSERT_EQ(h2.secret[0], secret[0]);
		ASSERT_EQ(h2.secret[1], secret[1]);
		ASSERT_EQ(h2.secret[2], secret[2]);
		ASSERT_EQ(h2.secret[3], secret[3]);

		// General hashing
		ASSERT_EQ(h1.wyhash((uint8_t*)secret, sizeof(secret)), wyhash(secret, sizeof(secret), 0, secret));
		// uint64_t
		ASSERT_EQ(h1.wyhash(secret[1]), wyhash64(secret[1], secret[0]));
	}
}

TEST(wyhash, TemplateSpecializations)
{
	wy::internal::hash_imp h;

	// Normal class
	StuctTest t;
	wy::hash<StuctTest> h0;
	ASSERT_EQ(h0(t), h.wyhash((uint8_t*)(&t), sizeof(t)));

	// Pointer
	wy::hash<StuctTest*> h1;
	ASSERT_EQ(h1(&t), h.wyhash((uint8_t*)(&t), sizeof(t)));

	// Number
	uint64_t t2 = 7;
	wy::hash<uint64_t> h2;
	ASSERT_EQ(h2(t2), h.wyhash(t2));

	// C-string
	const char* t3 = "an example to hash";
	wy::hash<const char*> h3;
	ASSERT_EQ(h3(t3), h.wyhash((uint8_t*)t3, strlen(t3)));

	// String
	std::string t4 = "an example to hash std::string";
	wy::hash<std::string> h4;
	ASSERT_EQ(h4(t4), h.wyhash((uint8_t*)t4.data(), t4.size()));

	// string_view
	std::string_view t5 = "an example to hash std::string_view";
	wy::hash<std::string_view> h5;
	ASSERT_EQ(h5(t5), h.wyhash((uint8_t*)t5.data(), t5.size()));

	// std::pmr::string
	std::pmr::string t6 = "an example to hash std::pmr::string";
	wy::hash<std::pmr::string> h6;
	ASSERT_EQ(h6(t6), h.wyhash((uint8_t*)t6.data(), t6.size()));

	// std::u8string
	std::u8string t7 = u8"an example to hash std::u8string";
	wy::hash<std::u8string> h7;
	ASSERT_EQ(h7(t7), h.wyhash((uint8_t*)t7.data(), t7.size()));
}