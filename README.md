**num**ber **ser**ialization

super small c++ header that serializes and deserializes `std::vector<T>` and `std::vector<std::vector<T>>` to and from a stream, where `T` can be u/int8\_t..u/int64\_t, float or double

simply `#include "numser.hpp"` and start using it

## Serialization

```cpp
numser::ser_num_vec(const std::vector<T>& v, std::ostream& s)
```

takes a `std::vector<T>` and serializes it to a `std::ostream`

### example

```cpp

std::vector<int> v = {1, 2, 3};
std::ofstream s("filename.vec");

if (numser::ser_num_vec<int>(v, s) != numser::NUMSER_OK) {
	std::cout << "failed to serialize v to s" << std::endl;
}
```


## Deserialization

```cpp
deser_num_vec(std::istream& s, std::vector<T>& v)
```

takes a `std::istream` and deserializes it to a `std::vector<T>`

### example

```cpp
std::vector<int> v;
std::ifstream s("filename.vec");

if (numser::deser_num_vec(s, v) != numser::NUMSER_OK) {
	std::cout << "failed to deserialize v from s" << std::endl;
} else {
	for (auto x : v) {
		std::cout << x << std::endl;
	}
}
```

you should see the values `1, 2 and 3` printed one per line

full code sample

```cpp
#include <stdio.h>
#include <vector>

#include "numser.hpp"

#define filename "filename.vec"

void save(void) {
	std::vector<int> v = {1, 2, 3};
	std::ofstream s(filename);
	if (numser::ser_num_vec<int>(v, s) != numser::NUMSER_OK) {
		std::cout << "failed to serialize v to s" << std::endl;
	}
}

void load(void) {
	std::vector<int> v;
	std::ifstream s(filename);
	if (numser::deser_num_vec(s, v) != numser::NUMSER_OK) {
		std::cout << "failed to deserialize v from s" << std::endl;
	} else {
		for (auto x : v) {
			std::cout << x << std::endl;
		}
	}
}

int main(void) {
	save();
	load();	

	return 0;
}

```

build and run command

```shell
$ g++ -std=c++14 aww.cxx -o aww && ./aww
```

## testing

before using this library, make sure that it passes all tests

```shell
$ cd tests && make && ./test_numser --cleanup
```

exit code of `test_numser` is going to be the number of failed tests, if it passes all tests, it returns zero, therefore, you can automate `numser` update and testing

## potential use cases

* debugging large-ish vectors
* pre-computed matrixes
* streaming vectors from one system to another over a socket
* embed vectors within your existing container(s)
* etc.



