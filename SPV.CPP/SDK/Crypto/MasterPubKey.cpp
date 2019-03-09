// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MasterPubKey.h"

#include <SDK/Common/ByteStream.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/ByteStream.h>

#include <Core/BRBIP39Mnemonic.h>
#include <Core/BRCrypto.h>

#include <cstring>

namespace Elastos {
	namespace ElaWallet {
		MasterPubKey::MasterPubKey() {
			_fingerPrint = 0;
			memset(_chainCode.u8, 0, sizeof(_chainCode));
			memset(_pubKey, 0, sizeof(_pubKey));
		}

		MasterPubKey::MasterPubKey(const MasterPubKey &masterPubKey) {
			operator=(masterPubKey);
		}

		MasterPubKey::MasterPubKey(const CMBlock &pubKey, const UInt256 &chainCode) {
			_fingerPrint = 0;
			memset(_pubKey, 0, sizeof(_pubKey));
			if (pubKey.GetSize() > 33) {
				Log::error("Master public key length too large");
			} else {
				memcpy(_pubKey, pubKey, pubKey.GetSize());
			}
			_chainCode = chainCode;
		}

		MasterPubKey::~MasterPubKey() {
			Clean();
		}

		MasterPubKey& MasterPubKey::operator=(const MasterPubKey &masterPubKey) {
			_fingerPrint = masterPubKey._fingerPrint;
			memcpy(_pubKey, masterPubKey._pubKey, sizeof(_pubKey));
			_chainCode = masterPubKey._chainCode;

			return *this;
		}

		void MasterPubKey::Serialize(ByteStream &stream) const {
			stream.WriteUint32(_fingerPrint);
			stream.WriteBytes(&_chainCode, sizeof(_chainCode));
			stream.WriteBytes(_pubKey, sizeof(_pubKey));
		}

		bool MasterPubKey::Deserialize(ByteStream &stream) {
			if (!stream.ReadUint32(_fingerPrint)) {
				Log::error("MasterPubKey deserialize fingerPrint fail");
				return false;
			}

			if (!stream.ReadBytes(&_chainCode, sizeof(_chainCode))) {
				Log::error("MasterPubKey deserialize chainCode fail");
				return false;
			}

			if (!stream.ReadBytes(_pubKey, sizeof(_pubKey))) {
				Log::error("MasterPubKey deserialize pubkey fail");
				return false;
			}

			return true;
		}

		void MasterPubKey::SetFingerPrint(uint32_t fingerPrint) {
			_fingerPrint = fingerPrint;
		}

		uint32_t MasterPubKey::GetFingerPrint() const {
			return _fingerPrint;
		}

		void MasterPubKey::SetPubKey(const CMBlock &pubKey) {
			if (pubKey.GetSize() > sizeof(_pubKey)) {
				Log::error("Public key length too large");
			} else {
				memset(_pubKey, 0, sizeof(_pubKey));
				memcpy(_pubKey, pubKey, pubKey.GetSize());
			}
		}

		CMBlock MasterPubKey::GetPubKey() const {
			return CMBlock(_pubKey, sizeof(_pubKey));
		}

		void MasterPubKey::SetChainCode(const UInt256 &chainCode) {
			_chainCode = chainCode;
		}

		const UInt256& MasterPubKey::GetChainCode() const {
			return _chainCode;
		}

		void MasterPubKey::Clean() {
			memset(_pubKey, 0, sizeof(_pubKey));
			memset(_chainCode.u8, 0, sizeof(_chainCode));
			_fingerPrint = 0;
		}

		bool MasterPubKey::Empty() const {
			uint8_t empty[sizeof(_pubKey)];
			memset(empty, 0, sizeof(empty));
			UInt256 zero = UINT256_ZERO;

			return memcmp(_pubKey, empty, sizeof(_pubKey)) == 0 &&
				   UInt256Eq(&_chainCode, &zero) && _fingerPrint == 0;
		}

	}
}
