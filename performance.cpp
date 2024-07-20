/////////////////////////////////////////////////////////////////////////////////
// This file is a C++ wrapper around wyhash: 
// https://github.com/wangyi-fudan/wyhash
// 
// Copyright (c) 2022-2024 by Alain Espinosa.
/////////////////////////////////////////////////////////////////////////////////

// include the required header
#include <wy.hpp>
#include <benchmark/benchmark.h>
#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <random>
#include <utility>

// Random common
static void std_mt19937_64(benchmark::State& _benchmark_state)
{
	std::mt19937_64 r; // Create a pseudo-random generator
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += r();

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(std_mt19937_64);
static void wy_rand_common(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += r();

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(wy_rand_common);

// Random uniform [0, 1)
static void wy_rand_uniform_0_1(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	double no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += r.uniform_dist();

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(wy_rand_uniform_0_1);
// Random uniform [min, max)
static void wy_rand_uniform_min_max(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	double no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += r.uniform_dist(5.6, 11.7);

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(wy_rand_uniform_min_max);
// Random uniform [0, k)
#if !WYHASH_32BIT_MUM
static void wy_rand_uniform_0_k(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += r.uniform_dist(5000);

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(wy_rand_uniform_0_k);
#endif

// Random gaussian [0, 1]
static void wy_rand_gaussian_0_1(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	double no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += r.gaussian_dist();

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(wy_rand_gaussian_0_1);
// Random gaussian [mean, std]
static void wy_rand_gaussian_mean_std(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	double no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += r.gaussian_dist(1.2, 2.5);

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(wy_rand_gaussian_mean_std);

// Random stream
static void wy_rand_stream(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	int64_t vector_length = _benchmark_state.range(0);
	std::vector<uint8_t> vec(vector_length);
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state)
	{
		r.generate_stream(vec, vector_length);
		no_op += vec[0];
	}

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetBytesProcessed(_benchmark_state.iterations() * vector_length);
}
BENCHMARK(wy_rand_stream)->Range(16, 4096);

// Hash uint32_t
static void std_hash_uint32(benchmark::State& _benchmark_state)
{
	std::hash<uint32_t> hasher; // Create a hash generator
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += hasher((uint32_t)no_op);

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(std_hash_uint32);
static void wy_hash_uint32(benchmark::State& _benchmark_state)
{
	wy::hash<uint32_t> hasher; // Create a hash generator
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += hasher((uint32_t)no_op);

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(wy_hash_uint32);
// Hash uint64_t
static void std_hash_uint64(benchmark::State& _benchmark_state)
{
	std::hash<uint64_t> hasher; // Create a hash generator
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += hasher(no_op);

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(std_hash_uint64);
static void wy_hash_uint64(benchmark::State& _benchmark_state)
{
	wy::hash<uint64_t> hasher; // Create a hash generator
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += hasher(no_op);

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(wy_hash_uint64);
// Hash std::pair<uint64_t, uint64_t>
static forceinline uint64_t std_hash_pair(const std::pair<uint64_t, uint64_t> &p) noexcept
{
	std::hash<uint64_t> hasher;
	return hasher(p.first) ^ hasher(p.second);
}
static void std_hash_uint64_pair(benchmark::State& _benchmark_state)
{
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += std_hash_pair(std::make_pair(no_op, no_op));

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(std_hash_uint64_pair);
static void wy_hash_uint64_pair(benchmark::State& _benchmark_state)
{
	wy::hash<std::pair<uint64_t, uint64_t>> hasher; // Create a hash generator
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op += hasher(std::make_pair(no_op, no_op));

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(wy_hash_uint64_pair);

// Hash string
static void wy_hash_string(benchmark::State& _benchmark_state)
{
	wy::hash<std::string> hasher; // Create a hash generator
	int64_t string_length = _benchmark_state.range(0);
	std::string s;
	s.assign(string_length, 'a');
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state)
	{
		no_op += hasher(s);
		memcpy(s.data(), &no_op, sizeof(no_op));
	}

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(wy_hash_string)->Range(16, 4096);
static void std_hash_string(benchmark::State& _benchmark_state)
{
	std::hash<std::string> hasher; // Create a hash generator
	int64_t string_length = _benchmark_state.range(0);
	std::string s;
	s.assign(string_length, 'a');
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state)
	{
		no_op += hasher(s);
		memcpy(s.data(), &no_op, sizeof(no_op));
	}

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(std_hash_string)->Range(16, 4096);

// Hash struct
template<uint32_t SIZE> static void wy_hash_array(benchmark::State& _benchmark_state)
{
	wy::hash<std::array<uint8_t, SIZE>> hasher; // Create a hash generator
	int64_t string_length = _benchmark_state.range(0);
	std::array<uint8_t, SIZE> array_to_hash;
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state)
	{
		no_op += hasher(array_to_hash);
		memcpy(array_to_hash.data(), &no_op, sizeof(no_op));
	}

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(wy_hash_array<8>);
BENCHMARK(wy_hash_array<9>);

BENCHMARK(wy_hash_array<16>);
BENCHMARK(wy_hash_array<18>);

BENCHMARK(wy_hash_array<32>);
BENCHMARK(wy_hash_array<35>);

BENCHMARK(wy_hash_array<64>);
BENCHMARK(wy_hash_array<67>);
