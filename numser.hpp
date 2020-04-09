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

#ifndef NUMSER
#define NUMSER

#include <stdio.h>
#include <string>
#include <vector>
#include <cstring>

#include <iostream>
#include <fstream>
#include <sstream>

namespace numser {

template <typename T>
struct TypeName {
	static const char* Get() { return typeid(T).name(); }
};

#define DEFINE_TYPENAME(T) template<>\
	struct TypeName<T> { static const char *Get() { return #T; }};
#define TNAME(T) std::string(numser::TypeName<T>::Get())

DEFINE_TYPENAME(int8_t)
DEFINE_TYPENAME(uint8_t)
DEFINE_TYPENAME(int16_t)
DEFINE_TYPENAME(uint16_t)
DEFINE_TYPENAME(int32_t)
DEFINE_TYPENAME(uint32_t)
DEFINE_TYPENAME(int64_t)
DEFINE_TYPENAME(uint64_t)
DEFINE_TYPENAME(float)
DEFINE_TYPENAME(double)

//  allow only ints and floats
#define NUMSER_TPL_REQ_NUM template <typename T, \
	typename std::enable_if<(std::is_integral<T>() || \
		std::is_floating_point<T>()), int>::type = 0>

//	version, maybe there will be a few
#define NUMSER_VERSION (uint8_t)0

typedef char NUMSER_SIGNATURE[6];

#define SZNUMSER_SIG sizeof(NUMSER_SIGNATURE)
#define NUMSER_SIG "NUMSER"

typedef struct {
	NUMSER_SIGNATURE signature;
	uint8_t version;
	uint8_t szelem;
	uint64_t count;
	uint64_t reserved;
} numser_hdr;

//	no exceptions, please!
typedef enum {
	NUMSER_OK = 0,
	NUMSER_ERR_WRITE,
	NUMSER_ERR_READ,
	NUMSER_ERR_SIGNATURE,
	NUMSER_ERR_VERSION,
	NUMSER_ERR_SZELEM_MISMATCH,
	NUMSER_ERR_UNKNOWN,
} NUMSER_RET;

const std::string NUMSER_RET_STR[] = {
	"OK",
	"Write error",
	"Read error",
	"Signature error",
	"Version error",
	"Element size mismatch",
	"Unkown error"
};

//	convenience
#define sznumser_hdr sizeof(numser_hdr)

//	64KiB read/write buffer size
#define sznumser_buf 64 * 1024

std::string numser_err_str(NUMSER_RET r) {
	//	validate range
	if ((r >= NUMSER_OK) && (r <= NUMSER_ERR_UNKNOWN)) {
		return NUMSER_RET_STR[r];
	}

	//	invalid range, return error
	return std::string("Invalid NUMSER_RET value");
}

//	serialize a vector<T> to ostream
//	T can be:
//		int8_t,  int16_t,  int32_t,  int64_t
//	   uint8_t, uint16_t, uint32_t, uint64_t
//		single,   double
NUMSER_TPL_REQ_NUM
NUMSER_RET ser_num_vec(const std::vector<T>& v, std::ostream& s) {
	// size of vector element in bytes
	size_t sztype_elem = sizeof(T);
	size_t count = v.size();
	char* buf = (char*)malloc(sznumser_buf);
	char* pbuf = buf;
	NUMSER_RET ret = NUMSER_OK;
	
	auto last_elem = v.end() -1;

	//	setup header
	numser_hdr hdr;
	std::memcpy(&hdr.signature, &NUMSER_SIG, SZNUMSER_SIG);
	hdr.version = NUMSER_VERSION;
	hdr.szelem = (uint8_t)sztype_elem;
	hdr.count = count;
	hdr.reserved = (uint64_t)0;

	//	write header
	s.write(reinterpret_cast<char*>(&hdr), sznumser_hdr);

	if (!s.good()) {
		ret = NUMSER_ERR_WRITE;
		goto cleanup;
	}

	//	loop over vector
	for (auto it = v.begin(); it <= last_elem; ++it) {
		// copy element into current buffer pointer
		*reinterpret_cast<T*>(pbuf) = *it;

		// increment buffer pointer by one element size
		pbuf += sztype_elem;

		//	should we write to stream?
		if ((size_t)(pbuf - buf) >= sznumser_buf || it == last_elem) {
			//	write buffer to stream
			s.write(reinterpret_cast<char*>(buf), pbuf - buf);
			if (!s.good()) {
				ret = NUMSER_ERR_WRITE;
				goto cleanup;
			}

			//	reset buffer pointer
			pbuf = buf;

			// flush stream buffer
			s.flush();
		}
	}
// don't judge
cleanup:
	free(buf);
	return ret;
}

//	serialize a vector<T> to a file
//	T can be:
//		int8_t,  int16_t,  int32_t,  int64_t
//	   uint8_t, uint16_t, uint32_t, uint64_t
//		single,   double
NUMSER_TPL_REQ_NUM
NUMSER_RET ser_num_vecf(const std::vector<T>& v, const std::string filename) {
	std::ofstream s(filename);
	return ser_num_vec<T>(v, s);
}

NUMSER_TPL_REQ_NUM
NUMSER_RET numser_check_hdr(const numser_hdr* hdr) {
	// size of vector element in bytes
	size_t sztype_elem = sizeof(T);

	//	check signature
	if (std::memcmp(&hdr->signature, &NUMSER_SIG, SZNUMSER_SIG) != 0) return NUMSER_ERR_SIGNATURE;

	//	check header version
	if (hdr->version != NUMSER_VERSION) return NUMSER_ERR_VERSION;

	//	check element size
	if (hdr->szelem != (uint8_t)sztype_elem) return NUMSER_ERR_SZELEM_MISMATCH;

	return NUMSER_OK;
}

//	deserialize a vector<T> from istream
//	T can be:
//		int8_t,  int16_t,  int32_t,  int64_t
//	   uint8_t, uint16_t, uint32_t, uint64_t
//		single,   double
NUMSER_TPL_REQ_NUM
NUMSER_RET deser_num_vec(std::istream& s, std::vector<T>& v) {
	// size of vector element in bytes
	size_t sztype_elem = sizeof(T);
	char* buf = (char*)malloc(sznumser_buf);
	NUMSER_RET ret = NUMSER_OK;

	//	minimal sanity check
	numser_hdr hdr;

	s.read(reinterpret_cast<char*>(&hdr), sznumser_hdr);
	
	size_t bread = 0;
	size_t btotal = hdr.count * hdr.szelem;

	if (!s.good()) {
		ret = NUMSER_ERR_READ;
		goto cleanup;
	}

	ret = numser_check_hdr<T>(&hdr);
	if (ret != NUMSER_OK) goto cleanup;

	//	preallocate
	v.reserve(hdr.count);

	while (bread < btotal) {
		//	how many bytes can we read at a time?
		size_t bytes = std::min<size_t>(btotal - bread, sznumser_buf);

		//	read this many bytes
		s.read(buf, bytes);

		if (!s.good()) {
			ret = NUMSER_ERR_READ;
			goto cleanup;
		}

		//	keep count
		bread += bytes;

		//	grap ref to buf
		char* pbuf = buf;

		while ((size_t)(pbuf - buf) < bytes) {
			//	add item to vector
			v.push_back(*reinterpret_cast<T*>(pbuf));

			//	increment pointer to buf by element size
			pbuf += sztype_elem;
		}
	}
//	don't judge
cleanup:
	free(buf);
	return ret;
}

//	deserialize a vector<T> from file
//	T can be:
//		int8_t,  int16_t,  int32_t,  int64_t
//	   uint8_t, uint16_t, uint32_t, uint64_t
//		single,   double
NUMSER_TPL_REQ_NUM
NUMSER_RET deser_num_vecf(const std::string filename, std::vector<T>& v) {
	std::ifstream s(filename);
	return deser_num_vec<T>(s, v);
}

//	serialize a vector<vector<T>> to ostream
//	T can be:
//		int8_t,  int16_t,  int32_t,  int64_t
//	   uint8_t, uint16_t, uint32_t, uint64_t
//		single, double
NUMSER_TPL_REQ_NUM
NUMSER_RET ser_num_vec2d(const std::vector<std::vector<T>>& v, std::ostream& s) {
	//	size of vector element in bytes
	size_t sztype_elem = sizeof(T);

	//	setup header
	numser_hdr hdr;
	std::memcpy(&hdr.signature, &NUMSER_SIG, SZNUMSER_SIG);
	hdr.version = NUMSER_VERSION;
	hdr.szelem = (uint8_t)sztype_elem;
	hdr.count = v.size();
	hdr.reserved = (uint8_t)0;

	//	write header
	s.write(reinterpret_cast<char*>(&hdr), sznumser_hdr);

	if (!s.good()) {
		return NUMSER_ERR_WRITE;
	}

	//	write vectors
	for (auto vi : v) {
		NUMSER_RET ret = ser_num_vec<T>(vi, s);
		if (ret != NUMSER_OK) {
			return ret;
		}
	}
	
	return NUMSER_OK;
}

//	serialize a vector<vector<T>> to file
//	T can be:
//		int8_t,  int16_t,  int32_t,  int64_t
//	   uint8_t, uint16_t, uint32_t, uint64_t
//		single,   double
NUMSER_TPL_REQ_NUM
NUMSER_RET ser_num_vec2df(const std::vector<std::vector<T>>& v, const std::string filename) {
	std::ofstream s(filename);
	return ser_num_vec2d<T>(v, s);
}

//	deserialize a vector<vector<T>> from istream
//	I'm not judging...
//	T can be:
//		int8_t,  int16_t,  int32_t,  int64_t
//	   uint8_t, uint16_t, uint32_t, uint64_t
//		single,   double
NUMSER_TPL_REQ_NUM
NUMSER_RET deser_num_vec2d(std::istream& s, std::vector<std::vector<T>>& v) {
	//	minimal sanity check
	numser_hdr hdr;
	s.read(reinterpret_cast<char*>(&hdr), sznumser_hdr);

	if (!s.good()) {
		return NUMSER_ERR_READ;
	}

	NUMSER_RET ret = numser_check_hdr<T>(&hdr);
	if (ret != NUMSER_OK) {
		return ret;
	}

	//	preallocate
	v.reserve(hdr.count);

	//	deserialize each vector
	for (uint64_t i = 0; i < hdr.count; ++i) {
		auto vi = std::vector<T>{};
		ret = deser_num_vec<T>(s, vi);
		if (ret != NUMSER_OK) {
			return ret;
		}
		v.push_back(vi);
	}

	return NUMSER_OK;
}

//	deserialize a vector<vector<T>> from file
//	T can be:
//		int8_t,  int16_t,  int32_t,  int64_t
//	   uint8_t, uint16_t, uint32_t, uint64_t
//		single,   double
NUMSER_TPL_REQ_NUM
NUMSER_RET deser_num_vec2df(const std::string filename, std::vector<std::vector<T>>& v) {
	std::ifstream s(filename);
	return deser_num_vec2d<T>(s, v);
}

} // namepsace numser

#endif



