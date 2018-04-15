// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>

#include "BRBIP39Mnemonic.h"

#include "MasterPubKey.h"

namespace Elastos {
	namespace SDK {


		MasterPubKey::MasterPubKey() {
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey);
		}

		MasterPubKey::MasterPubKey(const std::string &phrase) {
			UInt512 seed = UINT512_ZERO;
			BRBIP39DeriveKey(seed.u8, phrase.c_str(), nullptr);
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey);
			*_masterPubKey = BRBIP32MasterPubKey(&seed, sizeof(seed));
		}

		std::string MasterPubKey::toString() const {
			//todo complete me
			return "";
		}

		BRMasterPubKey *MasterPubKey::getRaw() const {
			return _masterPubKey.get();
		}

		ByteData MasterPubKey::serialize() const {
			return ByteData((uint8_t *) _masterPubKey.get(), sizeof(BRMasterPubKey));
		}

		void MasterPubKey::deserialize(const ByteData &data) {
			assert (data.length == sizeof(BRMasterPubKey));
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey);
			memcpy(_masterPubKey.get(), data.data, data.length);
		}

		ByteData MasterPubKey::getPubKey() const {
			return ByteData(_masterPubKey->pubKey, 33);
		}

		boost::shared_ptr<BRKey> MasterPubKey::getPubKeyAsKey() const {
			uint8_t pubKey[33];
			BRBIP32PubKey(pubKey, sizeof(pubKey), *_masterPubKey, 0, 0);
			BRKey *key = new BRKey;
			BRKeySetPubKey(key, pubKey, sizeof(pubKey));
			return boost::shared_ptr<BRKey>(key);
		}

		ByteData MasterPubKey::bip32BitIDKey(const ByteData &seed, int index, const std::string &uri) {
			BRKey key;
			BRBIP32BitIDKey(&key, seed.data, (size_t) seed.length, (uint32_t) index, uri.c_str());
			size_t dataLen = BRKeyPrivKey(&key, nullptr, 0);
			char rawKey[dataLen];
			BRKeyPrivKey(&key, rawKey, sizeof(rawKey));
			uint8_t *resultKey = new uint8_t[dataLen];
			memcpy(resultKey, rawKey, dataLen);
			return ByteData(resultKey, sizeof(rawKey));
		}

		bool MasterPubKey::validateRecoveryPhrase(const std::vector<std::string> &words, const std::string &phrase) {
			char *wordList[words.size()];
			for (int i = 0; i < words.size(); i++) {
				wordList[i] = const_cast<char *>(words[i].c_str());
			}
			return BRBIP39PhraseIsValid((const char **) wordList, phrase.c_str()) != 0;
		}

		ByteData MasterPubKey::generatePaperKey(const ByteData &seed, const std::vector<std::string> &words) {
			assert(seed.length == 16);
			assert(words.size() == 2048);
			char *wordList[words.size()];
			for (int i = 0; i < words.size(); i++) {
				wordList[i] = const_cast<char *>(words[i].c_str());
			}

			size_t size = BRBIP39Encode(nullptr, 0, (const char **) wordList, seed.data, (size_t) seed.length);
			char result[size];
			size = BRBIP39Encode(result, sizeof(result), (const char **) wordList,
								 (const uint8_t *) seed.data, (size_t) seed.length);

			uint8_t *data = new uint8_t[size];
			memcpy(data, &result, size);

			return ByteData(data, size);
		}

	}
}
