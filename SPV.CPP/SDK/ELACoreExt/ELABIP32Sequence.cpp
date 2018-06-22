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

			UInt512 I;
			UInt256 secret, chainCode;

			assert(key != nullptr);
			assert(seed.GetSize() > 0);
			assert(depth >= 0);

			if (seed.GetSize() > 0) {
				BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seed.GetSize());
				secret = *(UInt256 *)&I;
				chainCode = *(UInt256 *)&I.u8[sizeof(UInt256)];
				var_clean(&I);

				CMBlock privateKey;
				privateKey.SetMemFixed(secret.u8, sizeof(UInt256));
				for (int i = 0; i < depth; i++) {

					CMBlock mbChildPrivkey = BTCKey::getDerivePrivKey_Secret(privateKey, SEQUENCE_INTERNAL_CHAIN, 0,
							chainCode, NID_X9_62_prime256v1);
					memcpy(privateKey, mbChildPrivkey, privateKey.GetSize());
				}
				BRKeySetSecret((BRKey *)key, &secret, 1);
				var_clean(&secret, &chainCode);
			}
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