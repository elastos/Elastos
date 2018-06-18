// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>

#include "BRBIP39Mnemonic.h"
#include "BRBIP32Sequence.h"
#include "MasterPubKey.h"

namespace Elastos {
	namespace SDK {
		namespace {
			// Public parent key -> public child key
			//
			// CKDpub((Kpar, cpar), i) -> (Ki, ci) computes a child extended public key from the parent extended public key.
			// It is only defined for non-hardened child keys.
			//
			// - Check whether i >= 2^31 (whether the child is a hardened key).
			//     - If so (hardened child): return failure
			//     - If not (normal child): let I = HMAC-SHA512(Key = cpar, Data = serP(Kpar) || ser32(i)).
			// - Split I into two 32-byte sequences, IL and IR.
			// - The returned child key Ki is point(parse256(IL)) + Kpar.
			// - The returned chain code ci is IR.
			// - In case parse256(IL) >= n or Ki is the point at infinity, the resulting key is invalid, and one should proceed with
			//   the next value for i.
			//
			static void _CKDpub(BRECPoint *K, UInt256 *c, uint32_t i)
			{
				uint8_t buf[sizeof(*K) + sizeof(i)];
				UInt512 I;

				if ((i & BIP32_HARD) != BIP32_HARD) { // can't derive private child key from public parent key
					*(BRECPoint *)buf = *K;
					UInt32SetBE(&buf[sizeof(*K)], i);

					BRHMAC(&I, BRSHA512, sizeof(UInt512), c, sizeof(*c), buf, sizeof(buf)); // I = HMAC-SHA512(c, P(K) || i)

					*c = *(UInt256 *)&I.u8[sizeof(UInt256)];

					BRSecp256k1PointAdd(K, (UInt256 *)&I);

					var_clean(&I);
					mem_clean(buf, sizeof(buf));
				}
			}

		}

		MasterPubKey::MasterPubKey() {
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey);
		}

		MasterPubKey::MasterPubKey(const std::string &phrase, const std::string &phrasePassword) {
			UInt512 seed = UINT512_ZERO;
			const char *phrasePass = phrasePassword.empty() ? nullptr : phrasePassword.c_str();
			BRBIP39DeriveKey(seed.u8, phrase.c_str(), phrasePass);
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey(BRBIP32MasterPubKey(&seed, sizeof(seed))));
		}

		std::string MasterPubKey::toString() const {
			//todo complete me
			return "";
		}

		BRMasterPubKey *MasterPubKey::getRaw() const {
			return _masterPubKey.get();
		}

		CMBlock MasterPubKey::serialize() const {
			CMBlock ret(sizeof(BRMasterPubKey));
			uint8_t *tmp = (uint8_t*)_masterPubKey.get();
			memcpy(ret, tmp, ret.GetSize());

			return ret;
		}

		void MasterPubKey::deserialize(const CMBlock &data) {
			assert (data.GetSize() == sizeof(BRMasterPubKey));
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey);
			memcpy(_masterPubKey.get(), data, data.GetSize());
		}

		CMBlock MasterPubKey::getPubKey() const {
			CMBlock ret(33);
			memcpy(ret, _masterPubKey->pubKey, 33);

			return ret;
		}

		boost::shared_ptr<Key> MasterPubKey::getPubKeyAsKey() const {
			uint8_t pubKey[33];
			BRBIP32PubKey(pubKey, sizeof(pubKey), *_masterPubKey, 0, 0);
			BRKey *brkey = new BRKey;
			BRKeySetPubKey(brkey, pubKey, sizeof(pubKey));
			Key *key = new Key(brkey);
			return boost::shared_ptr<Key>(key);
		}

		CMBlock MasterPubKey::bip32BitIDKey(const CMBlock &seed, int index, const std::string &uri) {
			BRKey key;
			BRBIP32BitIDKey(&key, seed, (size_t) seed.GetSize(), (uint32_t) index, uri.c_str());
			size_t dataLen = BRKeyPrivKey(&key, nullptr, 0);
			char rawKey[dataLen];
			BRKeyPrivKey(&key, rawKey, sizeof(rawKey));
			CMBlock resultKey(dataLen);
			memcpy(resultKey, rawKey, dataLen);

			return resultKey;
		}

		bool MasterPubKey::validateRecoveryPhrase(const std::vector<std::string> &words, const std::string &phrase) {
			char *wordList[words.size()];
			for (int i = 0; i < words.size(); i++) {
				wordList[i] = const_cast<char *>(words[i].c_str());
			}
			return BRBIP39PhraseIsValid((const char **) wordList, phrase.c_str()) != 0;
		}

		std::string MasterPubKey::generatePaperKey(const UInt128 &seed, const std::vector<std::string> &words) {
			assert(words.size() == 2048);
			char *wordList[words.size()];
			for (int i = 0; i < words.size(); i++) {
				wordList[i] = const_cast<char *>(words[i].c_str());
			}

			size_t size = BRBIP39Encode(nullptr, 0, (const char **) wordList, seed.u8, sizeof(seed));
			char result[size];
			size = BRBIP39Encode(result, sizeof(result), (const char **) wordList, seed.u8, sizeof(seed));

			char resultChar[size];
			memcpy(resultChar, result, size);
			return std::string(resultChar, size);
		}

		MasterPubKey::MasterPubKey(const BRKey &key, const UInt256 &chainCode) {
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey);

			Key wrapperKey(key.secret, key.compressed);
			CMBlock pubKey = wrapperKey.getPubkey();

			memcpy(_masterPubKey->pubKey, pubKey, pubKey.GetSize());
			_masterPubKey->chainCode = chainCode;
			_masterPubKey->fingerPrint = wrapperKey.hashTo160().u32[0];
		}

		size_t MasterPubKey::BIP32PubKey(uint8_t *pubKey, size_t pubKeyLen, BRMasterPubKey mpk, uint32_t chain,
		                                 uint32_t index) {
			UInt256 chainCode = mpk.chainCode;
			BRMasterPubKey zeroPubKey = BR_MASTER_PUBKEY_NONE;
			assert(memcmp(&mpk, &zeroPubKey, sizeof(mpk)) != 0);

			if (pubKey && sizeof(BRECPoint) <= pubKeyLen) {
				*(BRECPoint *)pubKey = *(BRECPoint *)mpk.pubKey;

				_CKDpub((BRECPoint *)pubKey, &chainCode, chain); // path N(m/0H/chain)

				_CKDpub((BRECPoint *)pubKey, &chainCode, index); // index'th key in chain

				var_clean(&chainCode);
			}

			return (! pubKey || sizeof(BRECPoint) <= pubKeyLen) ? sizeof(BRECPoint) : 0;
		}

	}
}
