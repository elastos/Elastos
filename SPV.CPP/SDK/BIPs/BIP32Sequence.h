// Created by Aaron Voisine on 8/19/15.
// Copyright (c) 2015 breadwallet LLC
// Copyright (c) 2017-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BIP32SEQUENCE_H__
#define __ELASTOS_SDK_BIP32SEQUENCE_H__

#include <SDK/Crypto/MasterPubKey.h>
#include <SDK/Common/CMemBlock.h>
#include <SDK/Crypto/Key.h>

#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>

namespace Elastos {
	namespace ElaWallet {

// BIP32 is a scheme for deriving chains of addresses from a seed value
// https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki

#define BIP32_HARD                  0x80000000

#define SEQUENCE_GAP_LIMIT_EXTERNAL 10
#define SEQUENCE_GAP_LIMIT_INTERNAL 5
#define SEQUENCE_EXTERNAL_CHAIN     0
#define SEQUENCE_INTERNAL_CHAIN     1

		class BIP32Sequence {
		public:
			static MasterPubKey GetMasterPubKey(const CMBlock &seed);

			static CMBlock PubKey(const MasterPubKey &mpk, uint32_t change, uint32_t index);

			static Key PrivKey(const CMBlock &seed, uint32_t change, uint32_t index);

			static std::vector<Key> PrivKeyList(const CMBlock &seed, uint32_t change, const std::vector<uint32_t> &indexes);

			// BIP44
			static std::vector<Key> PrivKeyList(const void *seed, size_t seedLen, uint32_t coinType, uint32_t change,
												const std::vector<uint32_t> &indexes);

			static CMBlock PubKeyPath(const MasterPubKey &mpk, int depth, ...);

			static CMBlock PubKeyvPath(const MasterPubKey &mpk, int depth, va_list vlist);

			static Key PrivKeyPath(const void *seed, size_t seedLen, UInt256 &chainCode, int depth, ...);

			static Key PrivKeyPath(UInt256 &chainCode, const CMBlock &seed, int depth, ...);

			static Key PrivKeyvPath(UInt256 &chainCode, const CMBlock &seed, int depth, va_list vlist);

			static Key APIAuthKey(const void *seed, size_t seedLen);

			static Key BitIDKey(const CMBlock &seed, uint32_t index, const std::string &uri);

			static CMBlock PrivKeyToPubKey(const UInt256 &k);

		private:
			static bool ECPointAdd(const EC_GROUP *group, EC_POINT *point, const UInt256 &tweak, ECPoint &result);

			static bool TweakPublic(ECPoint &k, const UInt256 &tweak, int nid);

			static void CKDpub(ECPoint &k, UInt256 &c, uint32_t i);

			static bool TweakSecret(UInt256 &k, const UInt256 &tweak, int nid);

			static void CKDpriv(UInt256 &k, UInt256 &c, uint32_t i);

		};

	}
}

#endif
