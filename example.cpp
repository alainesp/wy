/////////////////////////////////////////////////////////////////////////////////
// This file is a C++ wrapper around wyhash: 
// https://github.com/wangyi-fudan/wyhash
// 
// Copyright (c) 2022 by Alain Espinosa.
/////////////////////////////////////////////////////////////////////////////////

// include the required header
#include <wy.hpp>
#include <random>

void main()
{
	// Create a pseudo-random generator
	wy::rand r;

	uint64_t r_value = r();                          // Generate a random number

	// Using direct methods
	double r_uniform01 = r.uniform_dist();           // Generate a random number from the uniform distribution [0, 1)
	uint64_t runiformk = r.uniform_dist(13);         // Generate a random number from the uniform distribution [0, 13)
	double r_uniform_p = r.uniform_dist(1.5, 4.7);   // Generate a random number from the uniform distribution [1.5, 4.7)

	double r_gaussian01 = r.gaussian_dist();         // Generate a random number from the Gaussian distribution with mean=0 and std=1
	double r_gaussian_p = r.gaussian_dist(1.1, 2.3); // Generate a random number from the Gaussian distribution with mean=1.1 and std=2.3

	// Using C++ <random> distributions
	std::uniform_int_distribution<uint64_t> dist(0, 13);
	runiformk = dist(r); // Similar to r.uniform_dist(13) but slower

	std::normal_distribution<double> gdist(1.1, 2.3);
	r_gaussian_p = gdist(r); // Similar to r.gaussian_dist(1.1, 2.3) but slower
}