# wy
C++ wrapper around wyhash and wyrand: https://github.com/wangyi-fudan/wyhash

wyhash and wyrand are the ideal 64-bit hash function and PRNG respectively:

- Solid: wyhash passed SMHasher, wyrand passed BigCrush, practrand.
- Portable: 64-bit / 32-bit system, big / little endian.
- Fastest: Efficient on 64-bit machines, especially for short keys.
- Simplest: In the sense of code size.
- Salted: We use dynamic secret to avoid intended attack.

# Usage

First you need to link the library to your project with cmake:

```cmake
include(FetchContent)
FetchContent_Declare(wy URL https://github.com/alainesp/wy/archive/refs/heads/main.zip)
FetchContent_MakeAvailable(wy)
target_link_libraries(YOUR_TARGET PRIVATE wy)
```

Or simply download the *wy.hpp* header that is the only requirement.

## Random generation example

```cpp
// include the required header
#include <wy.hpp>
#include <random>

void main()
{
	// Create a pseudo-random generator
	wy::rand r;

	// Using direct methods
	uint64_t r_value = r();                          // Generate a random number
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
```

## Hash function example

```cpp
// include the required header
#include <wy.hpp>
#include <unordered_map>

struct Person
{
	std::string name;
	std::string surname;
};

void main()
{
	// Create random persons
	std::vector<Person> persons;
	for (size_t i = 0; i < 500; i++)
		persons.push_back(Person{ std::string("Person Name") + std::to_string(i), std::string("Surname") });

	// Create hashtable
	std::unordered_map<std::string, Person, wy::hash<std::string>> h;

	// Add persons to the hashtable
	for (size_t i = 0; i < persons.size(); i++)
		h[persons[i].name] = persons[i];

	// Count persons
	size_t persons_found = 0;
	for (size_t i = 0; i < persons.size() * 2; i++)
		persons_found += h.count(std::string("Person Name") + std::to_string(i));

	printf("Found %I64i persons", persons_found);
}
```

## Performance

Running on a single threaded Ryzen 7 4800H laptop CPU

```bash
--------------------------------------------------
Random Performance
--------------------------------------------------
Random              : 1384M op/sec
--------------------------------------------------
Uniform [0, 1)      : 1153M op/sec
Uniform [min, max)  : 1013M op/sec
Uniform [0, k)      : 1047M op/sec
--------------------------------------------------
Gaussian [0, 1]     : 769M op/sec
Gaussian [mean, std]: 720M op/sec
--------------------------------------------------
Stream  [1024]      : 12.9 GB/sec
Stream  [4096]      : 10.3 GB/sec

--------------------------------------------------
Hashing Performance
--------------------------------------------------
uint64_t         : 1104M op/sec
std::string(14)  :  368M op/sec
std::string(28)  :  323M op/sec
std::string(112) :  184M op/sec
std::string(448) :   61M op/sec
--------------------------------------------------
```
