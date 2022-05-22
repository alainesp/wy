/////////////////////////////////////////////////////////////////////////////////
// This file is a C++ wrapper around wyhash: 
// https://github.com/wangyi-fudan/wyhash
// 
// Copyright (c) 2022 by Alain Espinosa.
/////////////////////////////////////////////////////////////////////////////////

// include the required header
#include <wy.hpp>
#include <chrono>
#include <iostream>
#include <format>

void main_rand()
{
	constexpr size_t numIter = 1'000'000'000;
	constexpr size_t numIterStream = 10'000'000;
	// Create a pseudo-random generator
	wy::rand r;
	uint64_t no_op = 0;// variable to restrict compiler optimizations
	double no_opd = 0;

	std::cout << "--------------------------------------------------" << std::endl;
	std::cout << "Random Performance" << std::endl;
	std::cout << "--------------------------------------------------" << std::endl;

	//////////////////////////////////////////////////////////////////////////////////////
	// Common operation
	//////////////////////////////////////////////////////////////////////////////////////
	auto start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_op ^= r();// Generate a random number
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("Random              : {}M op/sec\n", numIter / duration.count());// Show performance on millions operations per second

	std::cout << "--------------------------------------------------" << std::endl;
	//////////////////////////////////////////////////////////////////////////////////////
	// Uniform operation
	//////////////////////////////////////////////////////////////////////////////////////
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_opd += r.uniform_dist();// Generate a random number
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("Uniform [0, 1)      : {}M op/sec\n", numIter / duration.count());// Show performance on millions operations per second

	//////////////////////////////////////////////////////////////////////////////////////
	// Uniform operation
	//////////////////////////////////////////////////////////////////////////////////////
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_opd += r.uniform_dist(5.6, 11.7);// Generate a random number
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("Uniform [min, max)  : {}M op/sec\n", numIter / duration.count());// Show performance on millions operations per second

#if !WYHASH_32BIT_MUM
	//////////////////////////////////////////////////////////////////////////////////////
	// Uniform operation
	//////////////////////////////////////////////////////////////////////////////////////
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_op ^= r.uniform_dist(5000);// Generate a random number
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("Uniform [0, k)      : {}M op/sec\n", numIter / duration.count());// Show performance on millions operations per second

	std::cout << "--------------------------------------------------" << std::endl;
#endif

	//////////////////////////////////////////////////////////////////////////////////////
	// Gaussian operation
	//////////////////////////////////////////////////////////////////////////////////////
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_opd += r.gaussian_dist();// Generate a random number
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("Gaussian [0, 1]     : {}M op/sec\n", numIter / duration.count());// Show performance on millions operations per second

	//////////////////////////////////////////////////////////////////////////////////////
	// Gaussian operation
	//////////////////////////////////////////////////////////////////////////////////////
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_opd += r.gaussian_dist(1.2, 2.5);// Generate a random number
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("Gaussian [mean, std]: {}M op/sec\n", numIter / duration.count());// Show performance on millions operations per second

	std::cout << "--------------------------------------------------" << std::endl;

	//////////////////////////////////////////////////////////////////////////////////////
	// Stream operation
	//////////////////////////////////////////////////////////////////////////////////////
	std::vector<uint8_t> vec(4096, 0); 
	constexpr double GiB2GB = 1'000'000. / (1024 * 1024);// Constant to convert to base-2 GB definition
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIterStream; i++)
	{
		r.generate_stream(vec, 1024);// Generate a stream of random numbers
		no_op ^= vec[0];
	}
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("Stream  [1024]      : {:.1f} GB/sec\n", GiB2GB * numIterStream / duration.count());// Show performance on GB per second

	//////////////////////////////////////////////////////////////////////////////////////
	// Stream operation
	//////////////////////////////////////////////////////////////////////////////////////
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIterStream; i++)
	{
		r.generate_stream(vec, 4096);// Generate a stream of random numbers
		no_op ^= vec[0];
	}
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("Stream  [4096]      : {:.1f} GB/sec\n", GiB2GB * numIterStream * 4 / duration.count());// Show performance on GB per second

	std::cout << "--------------------------------------------------" << std::endl;

	// Ensures no optimization
	std::cout << std::format("{}", (no_op + no_opd) ? "" : "Bad luck!");
}

void main_hash()
{
	constexpr size_t numIter = 100'000'000;
	uint64_t no_op = 0;// variable to restrict compiler optimizations

	std::cout << "--------------------------------------------------" << std::endl;
	std::cout << "Hashing Performance" << std::endl;
	std::cout << "--------------------------------------------------" << std::endl;

	//////////////////////////////////////////////////////////////////////////////////////
	// uint64_t operation
	//////////////////////////////////////////////////////////////////////////////////////
	wy::hash<uint64_t> hasherUint64;
	auto start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_op ^= hasherUint64(i);// Hash
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("uint64_t         : {:4}M op/sec\n", numIter / duration.count());// Show performance on millions operations per second

	//////////////////////////////////////////////////////////////////////////////////////
	// String operation
	//////////////////////////////////////////////////////////////////////////////////////
	wy::hash<std::string> hasherStr;
	std::string small("01234567890123");
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_op ^= hasherStr(small);// Hash
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("std::string(14)  : {:4}M op/sec\n", numIter / duration.count());// Show performance on millions operations per second

	small += "01234567890123";
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_op ^= hasherStr(small);// Hash
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("std::string(28)  : {:4}M op/sec\n", numIter / duration.count());// Show performance on millions operations per second

	small += small; small += small;
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_op ^= hasherStr(small);// Hash
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("std::string(112) : {:4}M op/sec\n", numIter / duration.count());// Show performance on millions operations per second

	small += small; small += small;
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_op ^= hasherStr(small);// Hash
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << std::format("std::string(448) : {:4}M op/sec\n", numIter / duration.count());// Show performance on millions operations per second

	std::cout << "--------------------------------------------------" << std::endl;

	// Ensures no optimization
	std::cout << std::format("{}", no_op ? "" : "Bad luck!");
}

int main()
{
	main_rand();
	main_hash();

	return 0;
}