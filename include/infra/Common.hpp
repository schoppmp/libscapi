/**
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
* Copyright (c) 2016 LIBSCAPI (http://crypto.biu.ac.il/SCAPI)
* This file is part of the SCAPI project.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* We request that any publication and/or code referring to and/or based on SCAPI contain an appropriate citation to SCAPI, including a reference to
* http://crypto.biu.ac.il/SCAPI.
*
* Libscapi uses several open source libraries. Please see these projects for any further licensing issues.
* For more information , See https://github.com/cryptobiu/libscapi/blob/master/LICENSE.MD
*
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*/


#pragma once

#define WIN32_LEAN_AND_MEAN

#include <boost/random.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/timer/timer.hpp>
#include <math.h> /* pow */
#include <random>
#include <memory>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <chrono>

#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/export.hpp>


using namespace std;

class IllegalStateException : public logic_error
{
public:
	IllegalStateException(const string & msg) : logic_error(msg) {};
};

class NotImplementedException : public logic_error
{
public:
	NotImplementedException(const string & msg) : logic_error(msg) {};
};

class InvalidKeyException : public logic_error
{
public:
	InvalidKeyException(const string & msg) : logic_error(msg) {};
};

class KeyException : public logic_error
{
public:
	KeyException(const string & msg) : logic_error(msg) {};
};

class UnsupportedOperationException : public logic_error
{
public:
	UnsupportedOperationException(const string & msg) : logic_error(msg) {};
};

class SecurityLevelException : public logic_error
{
public:
	SecurityLevelException(const string & msg) : logic_error(msg) {};
};

class CheatAttemptException : public logic_error
{
public:
	CheatAttemptException(const string & msg) : logic_error(msg) {};
};

// using boost::multiprecision:cpp_int - Arbitrary precision integer type.
namespace mp = boost::multiprecision;     // reduce the typing a bit later...

#ifdef _WIN32
using biginteger = boost::multiprecision::cpp_int;
#else
using biginteger = boost::multiprecision::mpz_int;
// boost::serialization compatibility for gmp_int backend
namespace boost { namespace serialization {
template<class Archive>
inline void save(
	Archive & ar,
	const boost::multiprecision::gmp_int & t,
	const unsigned int /* file_version */
){
	auto t2 = boost::multiprecision::cpp_int(biginteger(t));
	ar & t2;
	// size_t count = (mpz_sizeinbase(t.data(), 2) + 7) / 8;
	// std::vector<char> buf(count);
	// mpz_export(&buf[0], nullptr, 1, 1, 1, 0, t.data());
	// ar << buf;
}
template<class Archive>
inline void load(
	Archive & ar,
	boost::multiprecision::gmp_int & t,
	const unsigned int /* file_version */
){
	boost::multiprecision::cpp_int t2;
	ar & t2;
	t = std::move(biginteger(t2).backend());
	// std::vector<char> buf;
	// ar >> buf;
	// mpz_t z;
	// mpz_init(z);
	// mpz_import(z, buf.size(), 1, 1, 1, 0, &buf[0]);
	// t = std::move(z);
}
}}
BOOST_SERIALIZATION_SPLIT_FREE(boost::multiprecision::gmp_int)
#endif



typedef unsigned char byte;		// put in global namespace to avoid ambiguity with other byte typedefs


int find_log2_floor(biginteger);
int NumberOfBits(const biginteger & bi);

/*
* Retruns the number of bytes needed to represent a biginteger
* Notice that due to the sign number of byte can exceed log(value)
*/
size_t bytesCount(const biginteger & value);

class PrgFromOpenSSLAES;

shared_ptr<PrgFromOpenSSLAES> get_seeded_prg();

void copy_byte_vector_to_byte_array(const vector<byte> &source_vector, byte * dest, int beginIndex);
void copy_byte_array_to_byte_vector(const byte* src, int src_len, vector<byte>& target_vector, int beginIndex);

/*
* Converting big integer to a byte array. Array must be allocated already
* Number can be postive or negative - the sign will be preserved in the encoding
* Use byteCount(biginteger) method to calculate the number of bytes needed.
*/
void encodeBigInteger(const biginteger & value, byte* output, size_t length);

/*
* Decodoing big integer from byte array back to a biginteger object
*/
biginteger decodeBigInteger(const byte* input, size_t length);

biginteger convert_hex_to_biginteger(const string & hex);

/*
* Returns a random biginteger uniformly distributed in [min, max]
*/
biginteger getRandomInRange(const biginteger & min, const biginteger & max, PrgFromOpenSSLAES* random);

/*
* Returns a random prime number with the given number of bytes.
*/
biginteger getRandomPrime(int numBytes, int certainty, PrgFromOpenSSLAES* random);
const vector<string> explode(const string& s, const char& c);

bool isPrime(const biginteger & bi, int certainty = 40);

/********************/
/* Debugging Methods*/
/********************/

void print_elapsed_ms(std::chrono::time_point<std::chrono::system_clock> start, string message);
void print_elapsed_micros(std::chrono::time_point<std::chrono::system_clock> start, string message);
std::chrono::time_point<std::chrono::system_clock> scapi_now();
string hexStr(vector<byte> const & data);
void print_byte_array(byte * arr, int len, string message);
void gen_random_bytes_vector(vector<byte> &v, const int len, PrgFromOpenSSLAES* random);

/**
* Abstract marker interface that allow serialization and deserialization from byte array and size
*/
class NetworkSerialized {
public:
	virtual string toString() = 0;
	virtual void initFromString(const string & raw) = 0;
	virtual void initFromByteVector(const vector<byte> & byteVector) {
		const byte * uc = &(byteVector[0]);
		std::string s(reinterpret_cast<char const*>(uc), byteVector.size());
		initFromString(s);
	}
};
