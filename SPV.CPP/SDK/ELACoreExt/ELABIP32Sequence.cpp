// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRBIP32Sequence.h"
#include "BRCrypto.h"

#include "ELABIP32Sequence.h"
#include "Key.h"

#define BIP32_SEED_KEY "Bitcoin seed"

namespace Elastos {
	namespace ElaWallet {

		void ELABIP32Sequence::BIP32APIAuthKey(const BRKey *key, const void *seed, size_t seedLen) {
			ELABIP32Sequence::BIP32PrivKeyPath(key, (const void *)seed, seedLen, 2, 1 | BIP32_HARD, 0);
		}

		void ELABIP32Sequence::BIP32PrivKeyPath(const BRKey *key, const void *seed, size_t seedLen, int depth, ...) {
			va_list ap;

			va_start(ap, depth);
			ELABIP32Sequence::BIP32vPrivKeyPath(key, seed, seedLen, depth, ap);
			va_end(ap);
		}

		void ELABIP32Sequence::BIP32vPrivKeyPath(const BRKey *key, const void *seed, size_t seedLen, int depth, va_list vlist) {
			UInt512 I;
			UInt256 secret, chainCode;

			assert(key != nullptr);
			assert(seed != NULL || seedLen == 0);
			assert(depth >= 0);

			if (seed || seedLen == 0) {
				BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
				secret = *(UInt256 *)&I;
				chainCode = *(UInt256 *)&I.u8[sizeof(UInt256)];
				var_clean(&I);

				for (int i = 0; i < depth; i++) {
					ELABIP32Sequence::CKDpriv(&secret, &chainCode, va_arg(vlist, uint32_t));
				}
				BRKeySetSecret((BRKey *)key, &secret, 1);
				var_clean(&secret, &chainCode);
			}
		}

		// sets the private key for path m/0H/chain/index to key
		void ELABIP32Sequence::BIP32PrivKey(BRKey *key, const void *seed, size_t seedLen, uint32_t chain, uint32_t index)
		{
			ELABIP32Sequence::BIP32PrivKeyPath(key, seed, seedLen, 3, 0 | BIP32_HARD, chain, index);
		}

		void ELABIP32Sequence::CKDpriv(UInt256 *k, UInt256 *c, uint32_t i) {
			uint8_t buf[sizeof(BRECPoint) + sizeof(i)];
			UInt512 I;

			if (i & BIP32_HARD) {
				buf[0] = 0;
				UInt256Set(&buf[1], *k);
			} else BRSecp256k1PointGen((BRECPoint *) buf, k);

			UInt32SetBE(&buf[sizeof(BRECPoint)], i);

			BRHMAC(&I, BRSHA512, sizeof(UInt512), c, sizeof(*c), buf,
			       sizeof(buf)); // I = HMAC-SHA512(c, k|P(k) || i)

			BRSecp256k1ModAdd(k, (UInt256 *) &I); // k = IL + k (mod n)

			*c = *(UInt256 *) &I.u8[sizeof(UInt256)]; // c = IR

			var_clean(&I);
			mem_clean(buf, sizeof(buf));
		}

		BRMasterPubKey ELABIP32Sequence::BIP32MasterPubKey(const void *seed, size_t seedLen) {
			BRMasterPubKey mpk = BR_MASTER_PUBKEY_NONE;
			UInt512 I;
			UInt256 secret, chain;
			BRKey key;

			assert(seed != NULL || seedLen == 0);

			if (seed || seedLen == 0) {
				BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
				secret = *(UInt256 *)&I;
				chain = *(UInt256 *)&I.u8[sizeof(UInt256)];
				var_clean(&I);
				mpk.chainCode = chain;
				mpk.fingerPrint = BRKeyHash160(&key).u32[0];

				ELABIP32Sequence::CKDpriv(&secret, &chain, 0 | BIP32_HARD); // path m/0H

				Key wrapperKey(secret, 1);
				CMBlock pubKey = wrapperKey.getPubkey();
				memcpy(mpk.pubKey, pubKey, pubKey.GetSize());

				var_clean(&secret, &chain);
				BRKeyClean(&key);
			}

			return mpk;
		}

		void ELABIP32Sequence::BIP32PrivKeyList(BRKey keys[], size_t keysCount, const void *seed, size_t seedLen,
		                                        uint32_t chain, const uint32_t indexes[]) {
			UInt512 I;
			UInt256 secret, chainCode, s, c;

			assert(keys != NULL || keysCount == 0);
			assert(seed != NULL || seedLen == 0);
			assert(indexes != NULL || keysCount == 0);

			if (keys && keysCount > 0 && (seed || seedLen == 0) && indexes) {
				BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
				secret = *(UInt256 *)&I;
				chainCode = *(UInt256 *)&I.u8[sizeof(UInt256)];
				var_clean(&I);
				ELABIP32Sequence::CKDpriv(&secret, &chainCode,  0 | BIP32_HARD);
				CKDpriv(&secret, &chainCode, chain); // path m/0H/chain

				for (size_t i = 0; i < keysCount; i++) {
					s = secret;
					c = chainCode;
					CKDpriv(&s, &c, indexes[i]); // index'th key in chain
					BRKeySetSecret(&keys[i], &s, 1);
				}

				var_clean(&secret, &chainCode, &c, &s);
			}
		}
	}
}