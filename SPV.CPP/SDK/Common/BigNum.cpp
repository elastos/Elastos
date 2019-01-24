// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BigNum.h"

namespace Elastos {
	namespace ElaWallet {

		BigNum::BigNum() {
			_bigNum = BN_new();
			BN_zero(_bigNum);
		}

		BigNum::BigNum(const BigNum &bigNum) {
			_bigNum = BN_dup(bigNum._bigNum);
		}

		BigNum::BigNum(BIGNUM *bn) {
			assert(bn != nullptr);
			_bigNum = bn;
		}

		BigNum::BigNum(const void *bin, size_t len) {
			_bigNum = BN_bin2bn((const unsigned char *)bin, (int)len, nullptr);
		}

		BigNum::BigNum(const CMBlock &bin) {
			_bigNum = BN_bin2bn(bin, bin.GetSize(), nullptr);
		}

		BigNum::~BigNum() {
			BN_free(_bigNum);
		}

		BigNum BigNum::operator=(const BigNum &bigNum) {
			BN_free(_bigNum);
			_bigNum = BN_dup(bigNum._bigNum);

			return *this;
		}

		BigNum BigNum::operator=(BIGNUM *bn) {
			BN_free(_bigNum);
			_bigNum = bn;

			return *this;
		}

		bool BigNum::operator<(const BigNum &bigNum) const {
			return BN_cmp(_bigNum, bigNum._bigNum) < 0;
		}

		bool BigNum::operator<=(const BigNum &bigNum) const {
			return BN_cmp(_bigNum, bigNum._bigNum) <= 0;
		}

		bool BigNum::operator==(const BigNum &bigNum) const {
			return BN_cmp(_bigNum, bigNum._bigNum) == 0;
		}

		bool BigNum::operator>(const BigNum &bigNum) const {
			return BN_cmp(_bigNum, bigNum._bigNum) > 0;
		}

		bool BigNum::operator>=(const BigNum &bigNum) const {
			return BN_cmp(_bigNum, bigNum._bigNum) >= 0;
		}

		std::string BigNum::ToDecString() const {
			char *str = BN_bn2dec(_bigNum);
			std::string decString = std::string(str);
			OPENSSL_free(str);
			return decString;
		}

		std::string BigNum::ToHexString() const {
			char *str = BN_bn2hex(_bigNum);
			std::string hexString = std::string(str);
			OPENSSL_free(str);
			return hexString;
		}

	}
}