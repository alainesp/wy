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

// Random common
static void BM_random_common(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	for (auto _ : _benchmark_state) r();

	benchmark::DoNotOptimize(r()); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(BM_random_common);

// Random uniform [0, 1)
static void BM_random_uniform_0_1(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	for (auto _ : _benchmark_state) r.uniform_dist();

	benchmark::DoNotOptimize(r()); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(BM_random_uniform_0_1);
// Random uniform [min, max)
static void BM_random_uniform_min_max(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	for (auto _ : _benchmark_state) r.uniform_dist(5.6, 11.7);

	benchmark::DoNotOptimize(r()); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(BM_random_uniform_min_max);
// Random uniform [0, k)
#if !WYHASH_32BIT_MUM
static void BM_random_uniform_0_k(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	for (auto _ : _benchmark_state) r.uniform_dist(5000);

	benchmark::DoNotOptimize(r()); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(BM_random_uniform_0_k);
#endif

// Random gaussian [0, 1]
static void BM_random_gaussian_0_1(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	for (auto _ : _benchmark_state) r.gaussian_dist();

	benchmark::DoNotOptimize(r()); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(BM_random_gaussian_0_1);
// Random gaussian [mean, std]
static void BM_random_gaussian_mean_std(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	for (auto _ : _benchmark_state) r.gaussian_dist(1.2, 2.5);

	benchmark::DoNotOptimize(r()); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(BM_random_gaussian_mean_std);

// Random stream
static void BM_random_stream(benchmark::State& _benchmark_state)
{
	wy::rand r; // Create a pseudo-random generator
	int64_t vector_length = _benchmark_state.range(0);
	std::vector<uint8_t> vec(vector_length);
	for (auto _ : _benchmark_state) r.generate_stream(vec, vector_length);

	benchmark::DoNotOptimize(r()); // Restrict compiler optimizations
	_benchmark_state.SetBytesProcessed(_benchmark_state.iterations() * vector_length);
}
BENCHMARK(BM_random_stream)->Range(16, 4096);

// Hash uint64_t
static void BM_hash_uint64(benchmark::State& _benchmark_state)
{
	wy::hash<uint64_t> hasher; // Create a hash generator
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op ^= hasher(no_op);

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(BM_hash_uint64);
// Hash string
static void BM_hash_string(benchmark::State& _benchmark_state)
{
	wy::hash<std::string> hasher; // Create a hash generator
	int64_t string_length = _benchmark_state.range(0);
	std::string s;
	s.assign(string_length, 'a');
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op ^= hasher(s);

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(BM_hash_string)->Range(16, 4096);

// Hash struct
template<uint32_t SIZE> static void BM_hash_struct(benchmark::State& _benchmark_state)
{
	wy::hash<std::array<uint8_t, SIZE>> hasher; // Create a hash generator
	int64_t string_length = _benchmark_state.range(0);
	std::array<uint8_t, SIZE> array_to_hash;
	uint64_t no_op = 0; // variable to restrict compiler optimizations
	for (auto _ : _benchmark_state) no_op ^= hasher(array_to_hash);

	benchmark::DoNotOptimize(no_op); // Restrict compiler optimizations
	_benchmark_state.SetItemsProcessed(_benchmark_state.iterations());
}
BENCHMARK(BM_hash_struct<8>);
BENCHMARK(BM_hash_struct<9>);

BENCHMARK(BM_hash_struct<16>);
BENCHMARK(BM_hash_struct<18>);

BENCHMARK(BM_hash_struct<32>);
BENCHMARK(BM_hash_struct<35>);

BENCHMARK(BM_hash_struct<64>);
BENCHMARK(BM_hash_struct<67>);
