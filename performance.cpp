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

static void main_rand()
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
	std::cout << "Random              : " << numIter / duration.count() << "M op/sec\n";// Show performance on millions operations per second

	std::cout << "--------------------------------------------------" << std::endl;
	//////////////////////////////////////////////////////////////////////////////////////
	// Uniform operation
	//////////////////////////////////////////////////////////////////////////////////////
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_opd += r.uniform_dist();// Generate a random number
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "Uniform [0, 1)      : " << numIter / duration.count() << "M op/sec\n";// Show performance on millions operations per second

	//////////////////////////////////////////////////////////////////////////////////////
	// Uniform operation
	//////////////////////////////////////////////////////////////////////////////////////
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_opd += r.uniform_dist(5.6, 11.7);// Generate a random number
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "Uniform [min, max)  : " << numIter / duration.count() << "M op/sec\n";// Show performance on millions operations per second

#if !WYHASH_32BIT_MUM
	//////////////////////////////////////////////////////////////////////////////////////
	// Uniform operation
	//////////////////////////////////////////////////////////////////////////////////////
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_op ^= r.uniform_dist(5000);// Generate a random number
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "Uniform [0, k)      : " << numIter / duration.count() << "M op/sec\n";// Show performance on millions operations per second

	std::cout << "--------------------------------------------------" << std::endl;
#endif

	//////////////////////////////////////////////////////////////////////////////////////
	// Gaussian operation
	//////////////////////////////////////////////////////////////////////////////////////
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_opd += r.gaussian_dist();// Generate a random number
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "Gaussian [0, 1]     : " << numIter / duration.count() << "M op/sec\n";// Show performance on millions operations per second

	//////////////////////////////////////////////////////////////////////////////////////
	// Gaussian operation
	//////////////////////////////////////////////////////////////////////////////////////
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_opd += r.gaussian_dist(1.2, 2.5);// Generate a random number
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "Gaussian [mean, std]: " << numIter / duration.count() << "M op/sec\n";// Show performance on millions operations per second

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
	std::cout << "Stream  [1024]      : " << GiB2GB * numIterStream / duration.count() << " GB/sec\n";// Show performance on GB per second

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
	std::cout << "Stream  [4096]      : " << GiB2GB * numIterStream * 4 / duration.count() << " GB/sec\n";// Show performance on GB per second

	std::cout << "--------------------------------------------------" << std::endl;

	// Ensures no optimization
	std::cout << ((no_op + no_opd) ? "" : "Bad luck!") << std::endl;
}

static void main_hash()
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
	std::cout << "uint64_t         : " << numIter / duration.count() << "M op/sec\n";// Show performance on millions operations per second

	//////////////////////////////////////////////////////////////////////////////////////
	// String operation
	//////////////////////////////////////////////////////////////////////////////////////
	wy::hash<std::string> hasherStr;
	std::string small("01234567890123");
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_op ^= hasherStr(small);// Hash
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "std::string(14)  : " << numIter / duration.count() << "M op/sec\n";// Show performance on millions operations per second

	small += "01234567890123";
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_op ^= hasherStr(small);// Hash
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "std::string(28)  : " << numIter / duration.count() << "M op/sec\n";// Show performance on millions operations per second

	small += small; small += small;
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_op ^= hasherStr(small);// Hash
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "std::string(112) : " << numIter / duration.count() << "M op/sec\n";// Show performance on millions operations per second

	small += small; small += small;
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numIter; i++) no_op ^= hasherStr(small);// Hash
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "std::string(448) : " << numIter / duration.count() << "M op/sec\n";// Show performance on millions operations per second

	std::cout << "--------------------------------------------------" << std::endl;

	// Ensures no optimization
	std::cout << (no_op ? "" : "Bad luck!") << std::endl;
}

int main()
{
	main_rand();
	main_hash();

	return 0;
}