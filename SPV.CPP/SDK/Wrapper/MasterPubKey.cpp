// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>

#include "BRBIP39Mnemonic.h"
#include "BRBIP32Sequence.h"
#include "MasterPubKey.h"

namespace Elastos {
	namespace SDK {


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
			Key *key = new Key(boost::shared_ptr<BRKey>(brkey));
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

			char *resultChar = new char[size];
			memcpy(resultChar, result, size);
			return std::string(resultChar, size);
		}

		MasterPubKey::MasterPubKey(const BRKey &key, const UInt256 &chainCode) {
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey);
			BRKeyPubKey((BRKey *)&key, _masterPubKey->pubKey, sizeof(_masterPubKey->pubKey));
			_masterPubKey->chainCode = chainCode;
			_masterPubKey->fingerPrint = BRKeyHash160((BRKey *)&key).u32[0];
		}

	}
}
