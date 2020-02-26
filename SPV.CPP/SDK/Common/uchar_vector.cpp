// Copyright (c) 2011-2012 Eric Lombrozo
// Copyright (c) 2011-2016 Ciphrex Corp.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "uchar_vector.h"

uchar_vector::uchar_vector() : std::vector<unsigned char>() {

}

uchar_vector::uchar_vector(vector::size_type n, const unsigned char& value) : std::vector<unsigned char>(n, value) {

}

uchar_vector::uchar_vector(const std::vector<unsigned char>& vec) : std::vector<unsigned char>(vec) {

}

uchar_vector::uchar_vector(const void *array, unsigned int size) : std::vector<unsigned char>((const unsigned char *)array,
                                                                                              (const unsigned char *)array + size) {

}

uchar_vector::uchar_vector(const std::string& hex) {
	this->setHex(hex);
}

bool uchar_vector::operator < (const uchar_vector &rhs) const {
	if (size() < rhs.size())
		return true;
	else if (size() > rhs.size())
		return false;
	return memcmp(this->data(), rhs.data(), size()) < 0;
}

bool uchar_vector::operator ==(const uchar_vector &rhs) const {
	return size() == rhs.size() && memcmp(this->data(), rhs.data(), size()) == 0;
}

bool uchar_vector::operator !=(const uchar_vector &rhs) const {
	return ! operator==(rhs);
}

uchar_vector& uchar_vector::operator+=(const std::vector<unsigned char>& rhs) {
	this->insert(this->end(), rhs.begin(), rhs.end());
	return *this;
}

uchar_vector& uchar_vector::operator<<(const std::vector<unsigned char>& rhs) {
	this->insert(this->end(), rhs.begin(), rhs.end());
	return *this;
}

uchar_vector& uchar_vector::operator<<(unsigned char byte) {
	this->push_back(byte);
	return *this;
}

const uchar_vector uchar_vector::operator+(const std::vector<unsigned char>& rightOperand) const {
	return uchar_vector(*this) += rightOperand;
}

uchar_vector& uchar_vector::operator=(const std::string& hex) {
	this->setHex(hex);
	return *this;
}

void uchar_vector::copyToArray(unsigned char* array) {
	std::copy(this->begin(),this->end(), array);
}

void uchar_vector::padLeft(unsigned char pad, uint total_length) {
	this->reverse();
	this->padRight(pad, total_length);
	this->reverse();
}

void uchar_vector::padRight(unsigned char pad, uint total_length) {
	for (uint i = this->size(); i < total_length; i++)
		this->push_back(pad);
}

void uchar_vector::append(const void *array, size_t size) {
	operator+=(uchar_vector(array, size));
}

std::string uchar_vector::getHex(bool spaceBytes) const {
	std::string hex;
	hex.reserve(this->size() * 2);
	for (uint i = 0; i < this->size(); i++) {
		if (spaceBytes && (i > 0)) hex += " ";
		hex += g_hexBytes[(*this)[i]];
	}
	return hex;
}

void uchar_vector::setHex(std::string hex) {
	this->clear();

	// pad on the left if hex contains an odd number of digits.
	if (hex.size() % 2 == 1)
		hex = "0" + hex;

	this->reserve(hex.size() / 2);

	for (uint i = 0; i < hex.size(); i+=2) {
		uint byte;
		sscanf(hex.substr(i, 2).c_str(), "%x", &byte);
		this->push_back(byte);
	}
}

void uchar_vector::reverse() {
	std::reverse(this->begin(), this->end());
}

uchar_vector uchar_vector::getReverse() const {
	uchar_vector rval(*this);
	rval.reverse();
	return rval;
}

std::string uchar_vector::getCharsAsString() const {
	std::string chars;
	chars.reserve(this->size());
	for (uint i = 0; i < this->size(); i++)
		chars += (*this)[i];
	return chars;
}

void uchar_vector::setCharsFromString(const std::string& chars) {
	this->clear();
	this->reserve(chars.size());
	for (uint i = 0; i < chars.size(); i++)
		this->push_back(chars[i]);
}

std::string uchar_vector::getBase64() const {
	unsigned int padding = (3 - (this->size() % 3)) % 3;
	std::string base64;

	uchar_vector paddedBytes = *this;
	for (unsigned int i = 1; i <= padding; i++)
		paddedBytes.push_back(0);

	base64.reserve(4*(paddedBytes.size()) / 3);

	for (unsigned int i = 0; i < paddedBytes.size(); i += 3) {
		uint32_t triple = ((uint32_t)paddedBytes[i] << 16) | ((uint32_t)paddedBytes[i+1] << 8) | (uint32_t)paddedBytes[i+2];
		base64 += base64chars[(triple & 0x00fc0000) >> 18];
		base64 += base64chars[(triple & 0x0003f000) >> 12];
		base64 += base64chars[(triple & 0x00000fc0) >> 6];
		base64 += base64chars[triple & 0x0000003f];
	}

	for (unsigned int i = 1; i <= padding; i++)
		base64[base64.size() - i] = '=';

	return base64;
}

void uchar_vector::setBase64(const std::string &base64) {
	unsigned int padding = (4 - (base64.size() % 4)) % 4;

	std::string paddedBase64;
	paddedBase64.reserve(base64.size() + padding);
	paddedBase64 = base64;
	paddedBase64.append(padding, '=');
	padding = 0; // we'll count them again in the loop so we also get any that were already there.

	this->clear();
	this->reserve(3*paddedBase64.size() / 4);

	bool bEnd = false;
	for (unsigned int i = 0; (i < paddedBase64.size()) && (!bEnd); i+=4) {
		uint32_t digits[4];
		for (unsigned int j = 0; j < 4; j++) {
			const char* pPos = strchr(base64chars, paddedBase64[i+j]);
			if (!pPos) bEnd = true;
			if (bEnd) {
				digits[j] = 0;
				padding++;
			}
			else
				digits[j] = (uint32_t)(pPos - base64chars);
		}

		uint32_t quadruple = (digits[0] << 18) | (digits[1] << 12) | (digits[2] << 6) | digits[3];

		this->push_back((quadruple & 0x00ff0000) >> 16);
		this->push_back((quadruple & 0x0000ff00) >> 8);
		this->push_back(quadruple & 0x000000ff);
	}

	for (unsigned int i = 0; i < padding; i++)
		this->pop_back();
}

void uchar_vector::clean() {
	memset(this->data(), 0, size());
}

bool uchar_vector::isZero() const {
	for (size_t i = 0; i < this->size(); ++i) {
		if ((*this)[i] != 0)
			return false;
	}
	return true;
}