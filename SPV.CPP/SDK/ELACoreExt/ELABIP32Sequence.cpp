// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRBIP32Sequence.h"
#include "BRCrypto.h"

#include "ELABIP32Sequence.h"
#include "Key.h"
#include "BTCKey.h"

namespace Elastos {
	namespace ElaWallet {

		void ELABIP32Sequence::BIP32APIAuthKey(const BRKey *key, const CMBlock &seed) {
			ELABIP32Sequence::BIP32PrivKeyPath(key, seed, 2, 1 | BIP32_HARD, 0);
		}

		void ELABIP32Sequence::BIP32PrivKeyPath(const BRKey *key, const CMBlock &seed, int depth, ...) {
			va_list ap;

			va_start(ap, depth);
			ELABIP32Sequence::BIP32vPrivKeyPath(key, seed, depth, ap);
			va_end(ap);
		}

		void ELABIP32Sequence::BIP32vPrivKeyPath(const BRKey *key, const CMBlock &seed, int depth, va_list vlist) {

			assert(key != nullptr);
			assert(seed.GetSize() > 0);
			assert(depth >= 0);

			UInt256 secret, chainCode = UINT256_ZERO;
			if (seed.GetSize() > 0) {
				CMBlock privateKey = BTCKey::getDerivePrivKey_depth(seed, chainCode, true, NID_X9_62_prime256v1, depth,
				                                                    vlist);
				memcpy(secret.u8, privateKey, privateKey.GetSize());
				BRKeySetSecret((BRKey *)key, &secret, 1);
			}
			var_clean(&secret, &chainCode);
		}

		// sets the private key for path m/0H/chain/index to key
		void ELABIP32Sequence::BIP32PrivKey(BRKey *key, const CMBlock &seed, uint32_t chain, uint32_t index)
		{
			UInt256 chainCode = UINT256_ZERO;
			CMBlock secret = BTCKey::getDerivePrivKey(seed, chain, index, chainCode);
			UInt256 privateKey;
			memcpy(privateKey.u8, secret, secret.GetSize());
			BRKeySetSecret((BRKey *)key, &privateKey, 1);
		}

	}
}