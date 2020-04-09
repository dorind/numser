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
#include <string>
#include <fstream>
#include <vector>
#include <cassert>

#include "../../numser.hpp"

template<typename T>
void make_file_vec(T init_val, T inc, int count, std::string name) {
	auto v = std::vector<T>(count);
	T value = init_val;
	for (int i = 0; i < count; ++i) {
		v[i] = value;
		value += inc;
	}
	assert(numser::ser_num_vecf<T>(v, name) == numser::NUMSER_OK && "error writing vector file");
}

template<typename T>
std::vector<T> load_file_vec(std::string name) {
	auto v = std::vector<T>{};

	assert(numser::deser_num_vecf<T>(name, v) == numser::NUMSER_OK && "error reading vector file");

	return v;
}

template<typename T>
void run_test(int count) {
	std::string file_left = "left.vec";
	std::string file_right = "right.vec";
	std::string file_result = "result.vec";

	make_file_vec<T>(-count, 1,count, file_left);
	make_file_vec<T>(count, -1, count, file_right);

	auto left = load_file_vec<T>(file_left);
	auto right = load_file_vec<T>(file_right);

	assert(left.size() == right.size() && "left and right disagree on size!");

	auto result = std::vector<T>(count);
	
	for (size_t i = 0; i < left.size(); ++i) {
		T sum = left[i] + right[i];

		assert(sum == 0 && "expected sum to be zero");

		result[i] = sum;
	}

	assert(numser::ser_num_vecf<T>(result, file_result) == numser::NUMSER_OK && "error writing result");
}

int main(void) {
	run_test<int>(1e3);

	std::cout << std::endl << "\033[1;32mEverything went well\033[0m" << std::endl << std::endl;

	return 0;
}
