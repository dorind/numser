/*
Copyright(c) Dorin Duminica. All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, 
	 this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
	 this list of conditions and the following disclaimer in the documentation
	 and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its 
	 contributors may be used to endorse or promote products derived from this
	 software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <vector>

#include <iostream>
#include <fstream>
#include <sstream>

#include <limits>
#include <errno.h>

#include "../numser.hpp"

static uint cnt_test_pass = 0;
static uint cnt_test_fail = 0;
static bool cleanup = false;

#define FILENAME "test."
#define DEFINE_FILE_NAME std::string filename = FILENAME + TNAME(T) + ".vec"
#define PRINT_TEST_NAME std::cout << __FUNCTION__ << ": " << TNAME(T) << " file: " << filename << " ... " << std::flush
#define TEST_HEADER DEFINE_FILE_NAME; PRINT_TEST_NAME
#define TEST_PASS std::cout << "\033[1;32mPASS\033[0m" << std::flush << std::endl;++cnt_test_pass
#define TEST_FAIL std::cout << "\033[1;31mFAIL ERROR: " << (errno == 0 ? "unknown" : strerror(errno)) << "\033[0m" << std::flush << std::endl;++cnt_test_fail

#define TEST_VEC_COUNT 1e6
#define TEST_VEC_COUNT2D 2

#define TEST_DELTA_INT 1
#define TEST_DELTA_FLT (float)1.1f
#define TEST_DELTA_DBL (double)10.02f

//	define and initialize min, max and counter for type
#define TEST_COUNTER(T) T min = std::numeric_limits<T>::min();\
						T max = std::numeric_limits<T>::max();\
						T counter = min;

NUMSER_TPL_REQ_NUM
T inc_counter(const T counter, const T min, const T max) {
	//	initialize for ints
	T delta = TEST_DELTA_INT;

	// special cases for single and double precision
	if (typeid(T) == typeid(float)) {
		delta = TEST_DELTA_FLT;
	} else if (typeid(T) == typeid(double)) {
		delta = TEST_DELTA_DBL;
	}

	//	increment counter, if overflow, set to min value of T
	return (counter + delta) > max ? min : counter + delta;
}

NUMSER_TPL_REQ_NUM
void fill_vector(std::vector<T>& v) {
	v.reserve(TEST_VEC_COUNT);

	TEST_COUNTER(T);

	for (int i = 0; i < TEST_VEC_COUNT; ++i) {
		//	push counter to vector
		v.push_back(counter);
		
		counter = inc_counter<T>(counter, min, max);
	}
}

NUMSER_TPL_REQ_NUM
bool test_vector(const std::vector<T>& v) {
	TEST_COUNTER(T);

	for (auto it : v) {
		if (it != counter) return false;
		
		counter = inc_counter<T>(counter, min, max);
	}
	return true;
}

NUMSER_TPL_REQ_NUM
void test_save_file() {
	TEST_HEADER;

	auto v = std::vector<T>{};
	fill_vector<T>(v);
	if (numser::ser_num_vecf(v, filename) != numser::NUMSER_OK) {
		TEST_FAIL;
		return;
	}

	TEST_PASS;
}

NUMSER_TPL_REQ_NUM
void test_load_file() {
	TEST_HEADER;
	
	auto v = std::vector<T>{};

	numser::NUMSER_RET ret = numser::deser_num_vecf(filename, v);

	//	delete test file
	if (cleanup) std::remove(filename.c_str());

	if (ret != numser::NUMSER_OK) {
		TEST_FAIL;
		return;
	} 

	if (test_vector<T>(v)) {
		TEST_PASS;
	} else {
		TEST_FAIL;
	}
}

NUMSER_TPL_REQ_NUM
void test_save_file2d() {
	TEST_HEADER;
	filename += ".2d";

	auto vv = std::vector<std::vector<T>>{};
	vv.reserve(TEST_VEC_COUNT2D);

	for (int i = 0; i < TEST_VEC_COUNT2D; ++i) {
		auto v = std::vector<T>{};
		fill_vector<T>(v);
		vv.push_back(v);
	}

	if (numser::ser_num_vec2df(vv, filename) != numser::NUMSER_OK) {
		TEST_FAIL;
		return;
	}

	TEST_PASS;
}

NUMSER_TPL_REQ_NUM
void test_load_file2d() {
	TEST_HEADER;
	filename += ".2d";

	auto vv = std::vector<std::vector<T>>{};

	numser::NUMSER_RET ret = numser::deser_num_vec2df(filename, vv);

	//	delete test file
	if (cleanup) std::remove(filename.c_str());

	if (ret != numser::NUMSER_OK) {
		TEST_FAIL;
		return;
	}

	for (auto it : vv) {
		if (!test_vector<T>(it)) {
			TEST_FAIL;
			return;
		}
	}
	
	TEST_PASS;
}


NUMSER_TPL_REQ_NUM
void run_tests() {
	test_save_file<T>();
	test_load_file<T>();
	test_save_file2d<T>();
	test_load_file2d<T>();
}

void print_help(const std::string& self) {
	std::cout << "Usage:" << std::endl
				<< std::endl
				<< "\t" << self << " [options]" << std::endl 
				<< std::endl
				<< "options:" << std::endl
				<< "\t--cleanup, cleanup\tclean test files" << std::endl
				<< "\t--help, help\t\tthis help" << std::endl;
}

int main(int argc, char *argv[]) {
	for (int i = 1; i < argc; ++i) {
		std::string arg(argv[i]);
		if (arg == "--cleanup" || arg == "cleanup") cleanup = true;
		if (arg == "--help" || arg == "help") {
			print_help(std::string(argv[0]));
			return 0;
		}
	}	

	run_tests<int8_t>();
	run_tests<uint8_t>();
	run_tests<int16_t>();
	run_tests<uint16_t>();
	run_tests<int32_t>();
	run_tests<uint32_t>();
	run_tests<int64_t>();
	run_tests<uint64_t>();
	run_tests<float>();
	run_tests<double>();

	std::cout << std::endl << "\033[1;37mTESTS " << cnt_test_pass + cnt_test_fail << " \033[1;32mPASSED " << cnt_test_pass << " \033[1;31mFAILED " << cnt_test_fail << "\033[0m" << std::endl;

	if (cnt_test_fail > 0) {
		std::cout << std::endl << "\033[1;41;33mTesting failed, please don't use this library until you identify and fix the issue(s)\033[0m" << std::endl << std::endl;
	}

	return cnt_test_fail;
}



