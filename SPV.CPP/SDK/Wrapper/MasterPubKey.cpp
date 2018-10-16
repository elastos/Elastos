// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>
#include <Core/BRBIP32Sequence.h>
#include <SDK/Common/Log.h>

#include "BRBIP39Mnemonic.h"
#include "BRBIP32Sequence.h"
#include "MasterPubKey.h"
#include "Utils.h"
#include "ByteStream.h"

namespace Elastos {
	namespace ElaWallet {
		MasterPubKey::MasterPubKey() {
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey);
			*_masterPubKey = BR_MASTER_PUBKEY_NONE;
		}

		MasterPubKey::MasterPubKey(const std::string &phrase, const std::string &phrasePassword) {
			UInt512 seed = UINT512_ZERO;
			const char *phrasePass = phrasePassword.empty() ? nullptr : phrasePassword.c_str();
			BRBIP39DeriveKey(seed.u8, phrase.c_str(), phrasePass);
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey(BRBIP32MasterPubKey(&seed, sizeof(seed))));
			var_clean(&seed);
		}

		std::string MasterPubKey::toString() const {
			//todo complete me
			return "";
		}

		BRMasterPubKey *MasterPubKey::getRaw() const {
			return _masterPubKey.get();
		}

		void MasterPubKey::Serialize(ByteStream &stream) const {
			stream.writeUint32(_masterPubKey->fingerPrint);
			stream.writeBytes(&_masterPubKey->chainCode, sizeof(_masterPubKey->chainCode));
			stream.writeBytes(_masterPubKey->pubKey, sizeof(_masterPubKey->pubKey));
		}

		bool MasterPubKey::Deserialize(ByteStream &stream) {
			if (!stream.readUint32(_masterPubKey->fingerPrint)) {
				Log::getLogger()->error("Master public key deserialize finger print fail");
				return false;
			}

			if (!stream.readBytes(&_masterPubKey->chainCode, sizeof(_masterPubKey->chainCode))) {
				Log::getLogger()->error("Master public key deserialize chain code fail");
				return false;
			}

			if (!stream.readBytes(_masterPubKey->pubKey, sizeof(_masterPubKey->pubKey))) {
				Log::getLogger()->error("Master public key deserialize pubkey fail");
				return false;
			}

			return true;
		}

		CMBlock MasterPubKey::getPubKey() const {
			CMBlock ret(33);
			memcpy(ret, _masterPubKey->pubKey, 33);

			return ret;
		}

		//todo fix or remove ? this is not use derive is not use BRBIP32PubKey
		boost::shared_ptr<Key> MasterPubKey::getPubKeyAsKey() const {
			uint8_t pubKey[33];
			BRBIP32PubKey(pubKey, sizeof(pubKey), *_masterPubKey, 0, 0);
			BRKey *brkey = new BRKey;
			BRKeySetPubKey(brkey, pubKey, sizeof(pubKey));
			Key *key = new Key(brkey);
			return boost::shared_ptr<Key>(key);
		}

		const UInt256& MasterPubKey::getChainCode() const {
			return _masterPubKey->chainCode;
		}

		MasterPubKey::MasterPubKey(const CMBlock &pubKey, const UInt256 &chainCode) {
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey);

			memcpy(_masterPubKey->pubKey, pubKey, pubKey.GetSize());
			_masterPubKey->chainCode = chainCode;
		}

		MasterPubKey::MasterPubKey(const BRKey &key, const UInt256 &chainCode) {
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey);

			Key wrapperKey(key.secret, key.compressed);
			CMBlock pubKey = wrapperKey.getPubkey();

			memcpy(_masterPubKey->pubKey, pubKey, pubKey.GetSize());
			_masterPubKey->chainCode = chainCode;
			_masterPubKey->fingerPrint = wrapperKey.hashTo160().u32[0];
		}

		MasterPubKey::MasterPubKey(const BRMasterPubKey &pubKey) {
			_masterPubKey = boost::shared_ptr<BRMasterPubKey>(new BRMasterPubKey);
			*_masterPubKey = pubKey;
		}

	}
}
