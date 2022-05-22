/////////////////////////////////////////////////////////////////////////////////
// This file is a C++ wrapper around wyhash: 
// https://github.com/wangyi-fudan/wyhash
// 
// Copyright (c) 2022 by Alain Espinosa.
/////////////////////////////////////////////////////////////////////////////////
// wyhash and wyrand are the ideal 64-bit hash function and PRNG respectively:
//
// solid: wyhash passed SMHasher, wyrand passed BigCrush, practrand.
// portable: 64-bit / 32-bit system, big / little endian.
// fastest: Efficient on 64-bit machines, especially for short keys.
// simplest: In the sense of code size.
// salted: We use dynamic secret to avoid intended attack.

#include <cassert>
#include <cstdint>
#include <vector>
#include <random>
#include <string>
#include "wyhash.h"


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
	v = ((v & 0x00FF00FF00FF00FFull) << 8) | ((v & 0xFF00FF00FF00FF00ull) >> 8);
	return v;
}
#endif
#endif


namespace wy {

	/// <summary>
	/// Pseudo random numbers generator using WYRAND
	/// </summary>
	struct rand {
		uint64_t state; // Only value needed to generate pseudo-random numbers

		////////////////////////////////////////////////////////////////////////////////////
		// UniformRandomBitGenerator requirenment
		////////////////////////////////////////////////////////////////////////////////////
		using result_type = uint64_t; // is an unsigned integer type
		/// <summary>
		/// Returns the smallest value that operator() may return.
		/// The value is strictly less than max().
		/// The function must be constexpr.
		/// </summary>
		static constexpr result_type min() { return 0; }
		/// <summary>
		/// Returns the largest value that operator() may return.
		/// The value is strictly greater than min().
		/// The function must be constexpr.
		/// </summary>
		static constexpr result_type max() { return UINT64_MAX; }
		/// <summary>
		/// Returns a random value in the closed interval [0, UINT64_MAX].
		/// </summary>
		/// <returns>A new 64-bit pseudo-random value</returns>
		inline uint64_t operator()() noexcept
		{
			return wyrand(&state);
		}
		////////////////////////////////////////////////////////////////////////////////////

		/// <summary>
		/// Construct a pseudo-random generator with a random seed
		/// </summary>
		rand() noexcept
		{
			// Seed with a real random value, if available
			std::random_device rd;
			state = static_cast<uint64_t>(rd()) | static_cast<uint64_t>(rd()) << 32;
		}
		/// <summary>
		/// Construct a pseudo-random generator with a given seed
		/// </summary>
		/// <param name="seed">The only value needed to generate the same sequence of pseudo-random numbers</param>
		rand(uint64_t seed) noexcept : state(seed)
		{}

		/// <summary>
		/// Generate a random value from the uniform distribution [0,1)
		/// </summary>
		/// <returns>The random value</returns>
		inline double uniform_dist() noexcept
		{
			return wy2u01(operator()());
		}

		/// <summary>
		/// Generate a random value from the uniform distribution [min_value, max_value)
		/// </summary>
		/// <param name="min_value">The minimum value (inclusive)</param>
		/// <param name="max_value">The maximum value (exclusive)</param>
		/// <returns>The random value</returns>
		inline double uniform_dist(double min_value, double max_value) noexcept
		{
			assert(max_value > min_value);

			return uniform_dist() * (max_value - min_value) + min_value;
		}

		/// <summary>
		/// Fast generation of a random value from the uniform distribution [0, max_value)
		/// </summary>
		/// <param name="max_value">The maximum value (exclusive)</param>
		/// <returns>The random value</returns>
		inline uint64_t uniform_dist(uint64_t max_value) noexcept
		{
			return wy2u0k(operator()(), max_value);
		}

		/// <summary>
		/// Generate a random value from APPROXIMATE Gaussian distribution with mean=0 and std=1
		/// </summary>
		/// <returns>The random value</returns>
		inline double gaussian_dist() noexcept
		{
			return wy2gau(operator()());
		}

		/// <summary>
		/// Generate a random value from APPROXIMATE Gaussian distribution with mean and std
		/// </summary>
		/// <param name="mean">The Gaussian mean</param>
		/// <param name="std">The Gaussian Standard Deviation</param>
		/// <returns>The random value</returns>
		inline double gaussian_dist(double mean, double std) noexcept
		{
			assert(std > 0);

			return gaussian_dist() * std + mean;
		}

		/// <summary>
		/// Generate a random stream of bytes.
		/// </summary>
		/// <param name="size">The size of the stream to generate</param>
		/// <returns>A vector of random bytes</returns>
		inline std::vector<uint8_t> generate_stream(size_t size) noexcept
		{
			std::vector<uint8_t> result;
			generate_stream(result, size);
			return result;
		}

		/// <summary>
		/// Generate a random stream of bytes.
		/// </summary>
		/// <param name="vec">out: A vector of random bytes</param>
		/// <param name="size">The size of the stream to generate</param>
		void generate_stream(std::vector<uint8_t>& vec, size_t size) noexcept
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

	/// <summary>
	/// Internal implementations
	/// </summary>
	namespace internal {
		/// <summary>
		/// Hash base class
		/// </summary>
		struct hash_imp
		{
			uint64_t secret[4];// salted: We use dynamic secret to avoid intended attacks.

			/// <summary>
			/// Create a wyhasher with default secret
			/// </summary>
			hash_imp() noexcept : secret{ 0xa0761d6478bd642full, 0xe7037ed1a0b428dbull, 0x8ebc6af09c88c6e3ull, 0x589965cc75374cc3ull }
			{}
			/// <summary>
			/// Create a wyhasher with secret generated from a seed
			/// </summary>
			/// <param name="seed">The seed to generate the secret from</param>
			hash_imp(uint64_t seed) noexcept
			{
				make_secret(seed, secret);
			}
			/// <summary>
			/// Create a wyhasher with a specific secret
			/// </summary>
			/// <param name="secret">The secret to use</param>
			hash_imp(const uint64_t psecret[4]) noexcept
			{
				memcpy(secret, psecret, sizeof(secret));
			}

			/// <summary>
			/// Hash general data
			/// </summary>
			/// <param name="data">The data to hash</param>
			/// <param name="len">The size of the data</param>
			/// <returns>A 64-bits hash</returns>
			inline uint64_t wyhash(const uint8_t* data, size_t len) const noexcept
			{
				return ::wyhash(data, len, 0, secret);
			}
			/// <summary>
			/// Hash a 64-bit number
			/// </summary>
			/// <param name="number">The number to hash</param>
			/// <returns>A 64-bits hash</returns>
			inline uint64_t wyhash(uint64_t number) const noexcept
			{
				return ::wyhash64(number, secret[0]);
			}
		};

		/// <summary>
		/// Hash base class for string types
		/// </summary>
		/// <typeparam name="STRING_TYPE">The type of the string, ex: std::string, std::wstring, ...</typeparam>
		template<class STRING_TYPE> struct hash_string_base : private hash_imp
		{
			using hash_imp::hash_imp;// Inherit constructors
			inline uint64_t operator()(const STRING_TYPE& elem) const noexcept
			{
				return hash_imp::wyhash(reinterpret_cast<const uint8_t*>(elem.data()), sizeof(STRING_TYPE::value_type) * elem.size());
			}
		};
	};

	/// <summary>
	/// Common wyhash for general use
	/// </summary>
	/// <typeparam name="T">Type of the element to hash</typeparam>
	template<class T> struct hash : private internal::hash_imp
	{
		using hash_imp::hash_imp;// Inherit constructors

		/// <summary>
		/// Hash a general type
		/// </summary>
		/// <param name="elem">The element to hash</param>
		/// <returns>A 64-bits hash</returns>
		inline uint64_t operator()(const T& elem) const noexcept
		{
			static_assert(sizeof(T) > 0, "Type to hash T should have variables");
			return hash_imp::wyhash(reinterpret_cast<const uint8_t*>(&elem), sizeof(T));
		}
	};
	/// <summary>
	/// Partial specialization for pointer
	/// </summary>
	/// <typeparam name="T">Type of elements</typeparam>
	template<class T> struct hash<T*> : private internal::hash_imp
	{
		using hash_imp::hash_imp;// Inherit constructors
		inline uint64_t operator()(const T* elem) const noexcept
		{
			static_assert(sizeof(T) > 0, "Type to hash T should have variables");
			return hash_imp::wyhash(reinterpret_cast<const uint8_t*>(elem), sizeof(T));
		}
	};
	
	// Partial specializations: number
	template<> struct hash<uint64_t> : private internal::hash_imp
	{
		using hash_imp::hash_imp;// Inherit constructors
		inline uint64_t operator()(uint64_t number) const noexcept
		{
			return hash_imp::wyhash(number);
		}
	};
	template<> struct hash<int64_t> : private internal::hash_imp
	{
		using hash_imp::hash_imp;// Inherit constructors
		inline uint64_t operator()(int64_t number) const noexcept
		{
			return hash_imp::wyhash(number);
		}
	};

	// Partial specializations: std::vector
	//template<class T> struct hash<std::vector<T>> : public internal::hash_string_base<std::vector<T>>
	//{
	//	using hash_string_base::hash_string_base;// Inherit constructors
	//};

	// C strings
	template<> struct hash<char*> : private internal::hash_imp
	{
		using hash_imp::hash_imp;// Inherit constructors
		inline uint64_t operator()(const char* data) const noexcept
		{
			return hash_imp::wyhash(reinterpret_cast<const uint8_t*>(data), strlen(data));
		}
	};
	template<> struct hash<const char*> : private internal::hash_imp
	{
		using hash_imp::hash_imp;// Inherit constructors
		inline uint64_t operator()(const char* data) const noexcept
		{
			return hash_imp::wyhash(reinterpret_cast<const uint8_t*>(data), strlen(data));
		}
	};

	// Partial specializations: std::string variants
	template<> struct hash<std::string> : public internal::hash_string_base<std::string>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	template<> struct hash<std::wstring> : public internal::hash_string_base<std::wstring>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	template<> struct hash<std::u16string> : public internal::hash_string_base<std::u16string>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	template<> struct hash<std::u32string> : public internal::hash_string_base<std::u32string>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	
#if __cplusplus >= 201703L// C++ 2017
	// std::string_view variants
	template<> struct hash<std::string_view> : public internal::hash_string_base<std::string_view>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	template<> struct hash<std::wstring_view> : public internal::hash_string_base<std::wstring_view>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	template<> struct hash<std::u16string_view> : public internal::hash_string_base<std::u16string_view>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	template<> struct hash<std::u32string_view> : public internal::hash_string_base<std::u32string_view>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	// std::pmr::string variants
	template<> struct hash<std::pmr::string> : public internal::hash_string_base<std::std::pmr::string>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	template<> struct hash<std::pmr::wstring> : public internal::hash_string_base<std::pmr::wstring>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	template<> struct hash<std::pmr::u16string> : public internal::hash_string_base<std::pmr::u16string>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	template<> struct hash<std::pmr::u32string> : public internal::hash_string_base<std::pmr::u32string>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
#endif

	// TODO: Consider using  __cpp_char8_t?
#if __cplusplus >= 202001L// C++ 2020
	// char8_t string variants
	template<> struct hash<std::u8string> : public internal::hash_string_base<std::u8string>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	template<> struct hash<std::u8string_view> : public internal::hash_string_base<std::u8string_view>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
	template<> struct hash<std::pmr::u8string> : public internal::hash_string_base<std::pmr::u8string>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};
#endif
};
