// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BIGNUM_H__
#define __ELASTOS_SDK_BIGNUM_H__

#include <SDK/Common/CMemBlock.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>

namespace Elastos {
	namespace ElaWallet {

		class BigNum {
		public:
			BigNum();

			BigNum(const BigNum &bigNum);

			BigNum(BIGNUM *bn);

			BigNum(const void *bin, size_t len);

			BigNum(const CMBlock &bin);

			~BigNum();

			BigNum operator=(const BigNum &bigNum);

			BigNum operator=(BIGNUM *bn);

			bool operator>(const BigNum &bigNum) const;

			bool operator>=(const BigNum &bigNum) const;

			bool operator==(const BigNum &bigNum) const;

			bool operator<(const BigNum &bigNum) const;

			bool operator<=(const BigNum &bigNum) const;

			std::string ToDecString() const;

			std::string ToHexString() const;

		private:
			BIGNUM *_bigNum;
		};

	}
}

#endif
