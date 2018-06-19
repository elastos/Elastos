// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MasterPrivKey.h"

namespace Elastos {
	namespace ElaWallet {

		MasterPrivKey::MasterPrivKey() {

		}

		MasterPrivKey::~MasterPrivKey() {

		}

		const CMBlock &MasterPrivKey::GetEncryptedKey() const {
			return _encryptedKey;
		}

		void MasterPrivKey::SetEncryptedKey(const CMBlock &data) {
			_encryptedKey = data;
		}

		const UInt256 &MasterPrivKey::GetChainCode() const {
			return _chainCode;
		}

		void MasterPrivKey::SetChainCode(const UInt256 &chainCode) {
			_chainCode = chainCode;
		}
	}
}