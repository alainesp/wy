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

#include <cstdint>
#include <vector>
#include <string>

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
		uint64_t operator()() noexcept;
		////////////////////////////////////////////////////////////////////////////////////

		/// <summary>
		/// Construct a pseudo-random generator with a random seed
		/// </summary>
		rand() noexcept;
		/// <summary>
		/// Construct a pseudo-random generator with a given seed
		/// </summary>
		/// <param name="seed">The only value needed to generate the same sequence of pseudo-random numbers</param>
		rand(uint64_t seed) noexcept;

		/// <summary>
		/// Generate a random value from the uniform distribution [0,1)
		/// </summary>
		/// <returns>The random value</returns>
		double uniform_dist() noexcept;

		/// <summary>
		/// Generate a random value from the uniform distribution [min_value, max_value)
		/// </summary>
		/// <param name="min_value">The minimum value (inclusive)</param>
		/// <param name="max_value">The maximum value (exclusive)</param>
		/// <returns>The random value</returns>
		double uniform_dist(double min_value, double max_value) noexcept;

		/// <summary>
		/// Fast generation of a random value from the uniform distribution [0, max_value)
		/// </summary>
		/// <param name="max_value">The maximum value (exclusive)</param>
		/// <returns>The random value</returns>
		uint64_t uniform_dist(uint64_t max_value) noexcept;

		/// <summary>
		/// Generate a random value from APPROXIMATE Gaussian distribution with mean=0 and std=1
		/// </summary>
		/// <returns>The random value</returns>
		double gaussian_dist() noexcept;

		/// <summary>
		/// Generate a random value from APPROXIMATE Gaussian distribution with mean and std
		/// </summary>
		/// <param name="mean">The Gaussian mean</param>
		/// <param name="std">The Gaussian Standard Deviation</param>
		/// <returns>The random value</returns>
		double gaussian_dist(double mean, double std) noexcept;
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
			hash_imp() noexcept;
			/// <summary>
			/// Create a wyhasher with secret generated from a seed
			/// </summary>
			/// <param name="seed">The seed to generate the secret from</param>
			hash_imp(uint64_t seed) noexcept;
			/// <summary>
			/// Create a wyhasher with a specific secret
			/// </summary>
			/// <param name="secret">The secret to use</param>
			hash_imp(const uint64_t secret[4]) noexcept;

			/// <summary>
			/// Hash general data
			/// </summary>
			/// <param name="data">The data to hash</param>
			/// <param name="len">The size of the data</param>
			/// <returns>A 64-bits hash</returns>
			uint64_t wyhash(const uint8_t* data, size_t len) const noexcept;
			/// <summary>
			/// Hash a 64-bit number
			/// </summary>
			/// <param name="number">The number to hash</param>
			/// <returns>A 64-bits hash</returns>
			uint64_t wyhash(uint64_t number) const noexcept;
		};

		/// <summary>
		/// Hash base class for string types
		/// </summary>
		/// <typeparam name="STRING_TYPE">The type of the string, ex: std::string, std::wstring, ...</typeparam>
		template<class STRING_TYPE> struct hash_string_base : private hash_imp
		{
			using hash_imp::hash_imp;// Inherit constructors
			uint64_t operator()(const STRING_TYPE& elem) const noexcept
			{
				return wyhash(reinterpret_cast<const uint8_t*>(elem.data()), sizeof(STRING_TYPE::value_type) * elem.size());
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
		uint64_t operator()(const T& elem) const noexcept
		{
			static_assert(sizeof(T) > 0, "Type to hash T should have variables");
			return wyhash(reinterpret_cast<const uint8_t*>(&elem), sizeof(T));
		}
	};
	/// <summary>
	/// Partial specialization for pointer
	/// </summary>
	/// <typeparam name="T">Type of elements</typeparam>
	template<class T> struct hash<T*> : private internal::hash_imp
	{
		using hash_imp::hash_imp;// Inherit constructors
		uint64_t operator()(const T* elem) const noexcept
		{
			static_assert(sizeof(T) > 0, "Type to hash T should have variables");
			return wyhash(reinterpret_cast<const uint8_t*>(elem), sizeof(T));
		}
	};
	
	// Partial specializations: number
	template<> struct hash<uint64_t> : private internal::hash_imp
	{
		using hash_imp::hash_imp;// Inherit constructors
		uint64_t operator()(uint64_t number) const noexcept
		{
			return wyhash(number);
		}
	};
	template<> struct hash<int64_t> : private internal::hash_imp
	{
		using hash_imp::hash_imp;// Inherit constructors
		uint64_t operator()(int64_t number) const noexcept
		{
			return wyhash(number);
		}
	};

	// Partial specializations: std::vector
	template<class T> struct hash<std::vector<T>> : public internal::hash_string_base<std::vector<T>>
	{
		using hash_string_base::hash_string_base;// Inherit constructors
	};

	// C strings
	template<> struct hash<char*> : private internal::hash_imp
	{
		using hash_imp::hash_imp;// Inherit constructors
		uint64_t operator()(const char* data) const noexcept
		{
			return wyhash(reinterpret_cast<const uint8_t*>(data), strlen(data));
		}
	};
	template<> struct hash<const char*> : private internal::hash_imp
	{
		using hash_imp::hash_imp;// Inherit constructors
		uint64_t operator()(const char* data) const noexcept
		{
			return wyhash(reinterpret_cast<const uint8_t*>(data), strlen(data));
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
