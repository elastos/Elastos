// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "uint256.h"

uint128::uint128() {
	for (int i = 0; i < WIDTH; i++)
		pn[i] = 0;
}

uint128::uint128(const basetype &b) {
	for (int i = 0; i < WIDTH; i++)
		pn[i] = b.pn[i];
}

uint128::uint128(uint64_t b) {
	pn[0] = (unsigned int) b;
	pn[1] = (unsigned int) (b >> 32);
	for (int i = 2; i < WIDTH; i++)
		pn[i] = 0;
}

uint128::uint128(const std::string &str) {
	SetHex(str);
}

uint128 &uint128::operator=(const basetype &b) {
	for (int i = 0; i < WIDTH; i++)
		pn[i] = b.pn[i];
	return *this;
}

uint128 &uint128::operator=(uint64_t b) {
	pn[0] = (unsigned int) b;
	pn[1] = (unsigned int) (b >> 32);
	for (int i = 2; i < WIDTH; i++)
		pn[i] = 0;
	return *this;
}

uint128::uint128(const std::vector<unsigned char> &vch) {
	if (vch.size() == sizeof(pn))
		memcpy(pn, &vch[0], sizeof(pn));
	else
		*this = 0;
}

uint160::uint160() {
	for (int i = 0; i < WIDTH; i++)
		pn[i] = 0;
}

uint160::uint160(const basetype &b) {
	for (int i = 0; i < WIDTH; i++)
		pn[i] = b.pn[i];
}

uint160::uint160(uint64_t b) {
	pn[0] = (unsigned int) b;
	pn[1] = (unsigned int) (b >> 32);
	for (int i = 2; i < WIDTH; i++)
		pn[i] = 0;
}

uint160::uint160(const std::string &str) {
	SetHex(str);
}

uint160::uint160(const std::vector<unsigned char> &vch) {
	if (vch.size() == sizeof(pn))
		memcpy(pn, &vch[0], sizeof(pn));
	else
		*this = 0;
}

uint160 &uint160::operator=(uint64_t b) {
	pn[0] = (unsigned int) b;
	pn[1] = (unsigned int) (b >> 32);
	for (int i = 2; i < WIDTH; i++)
		pn[i] = 0;
	return *this;
}

uint160 &uint160::operator=(const basetype &b) {
	for (int i = 0; i < WIDTH; i++)
		pn[i] = b.pn[i];
	return *this;
}

uint168::uint168() {
	_u168.resize(21, 0);
}

uint168::uint168(const uint168 &b) {
	operator=(b);
}

uint168::uint168(const std::string &str) {
	SetHex(str);
}

uint168::uint168(const bytes_t &vch) {
	if (vch.size() == 21) {
		_u168 = vch;
	} else {
		_u168.resize(21, 0);
	}
}

uint168::uint168(uint8_t prefix, const bytes_t &vch) {
	if (vch.size() == 20) {
		_u168.push_back(prefix);
		_u168 += vch;
	} else {
		_u168.resize(21, 0);
	}
}

uint168 &uint168::operator=(const uint168 &b) {
	_u168 = b._u168;
	return *this;
}

bool uint168::operator<(const uint168 &b) const {
	return _u168 < b._u168;
}

bool uint168::operator==(const uint168 &b) const {
	return _u168 == b._u168;
}

bool uint168::operator!=(const uint168 &b) const {
	return !(operator==(b));
}

unsigned char *uint168::begin() {
	return (unsigned char *) _u168.data();
}

unsigned char *uint168::end() {
	return (unsigned char *) _u168.data() + _u168.size();
}

const unsigned char *uint168::begin() const {
	return (unsigned char *) _u168.data();
}

const unsigned char *uint168::end() const {
	return (unsigned char *) _u168.data() + _u168.size();
}

std::string uint168::GetHex() const {
	return _u168.getReverse().getHex();
}

void uint168::SetHex(const std::string &str) {
	_u168.setHex(str);
	_u168.reverse();
	if (_u168.size() != 21)
		_u168.resize(21, 0);
}

unsigned int uint168::size() const {
	return _u168.size();
}

uint8_t uint168::prefix() const {
	return _u168[0];
}

const bytes_t &uint168::bytes() const {
	return _u168;
}

uint256::uint256() {
	for (int i = 0; i < WIDTH; i++)
		pn[i] = 0;
}

uint256::uint256(const basetype &b) {
	for (int i = 0; i < WIDTH; i++)
		pn[i] = b.pn[i];
}

uint256::uint256(uint64_t b) {
	pn[0] = (unsigned int) b;
	pn[1] = (unsigned int) (b >> 32);
	for (int i = 2; i < WIDTH; i++)
		pn[i] = 0;
}

uint256::uint256(const std::string &str) {
	SetHex(str);
}

uint256::uint256(const bytes_t &vch) {
	operator=(vch);
}

uint256 &uint256::operator=(const basetype &b) {
	for (int i = 0; i < WIDTH; i++)
		pn[i] = b.pn[i];
	return *this;
}

uint256 &uint256::operator=(const bytes_t &vch) {
	if (vch.size() == sizeof(pn))
		memcpy(pn, &vch[0], sizeof(pn));
	else
		memset(pn, 0, sizeof(pn));

	return *this;
}

uint256 &uint256::operator=(uint64_t b) {
	pn[0] = (unsigned int) b;
	pn[1] = (unsigned int) (b >> 32);
	for (int i = 2; i < WIDTH; i++)
		pn[i] = 0;
	return *this;
}

uint512::uint512() {
	for (int i = 0; i < WIDTH; i++)
		pn[i] = 0;
}

uint512::uint512(const basetype &b) {
	for (int i = 0; i < WIDTH; i++)
		pn[i] = b.pn[i];
}

uint512::uint512(uint64_t b) {
	pn[0] = (unsigned int) b;
	pn[1] = (unsigned int) (b >> 32);
	for (int i = 2; i < WIDTH; i++)
		pn[i] = 0;
}

uint512::uint512(const std::string &str) {
	SetHex(str);
}

uint512::uint512(const std::vector<unsigned char> &vch) {
	if (vch.size() == sizeof(pn))
		memcpy(pn, &vch[0], sizeof(pn));
	else
		*this = 0;
}

uint512 &uint512::operator=(const basetype &b) {
	for (int i = 0; i < WIDTH; i++)
		pn[i] = b.pn[i];
	return *this;
}

uint512 &uint512::operator=(uint64_t b) {
	pn[0] = (unsigned int) b;
	pn[1] = (unsigned int) (b >> 32);
	for (int i = 2; i < WIDTH; i++)
		pn[i] = 0;
	return *this;
}

uint256 uint512::trim256() const {
	uint256 ret;
	for (unsigned int i = 0; i < uint256::WIDTH; i++) {
		ret.pn[i] = pn[i];
	}
	return ret;
}