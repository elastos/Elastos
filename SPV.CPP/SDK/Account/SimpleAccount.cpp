// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SimpleAccount.h"
#include "ErrorCode.h"

namespace Elastos {
	namespace ElaWallet {

		SimpleAccount::SimpleAccount(const std::string &privKey, const std::string &payPassword) {
			//todo set encrypted key, and generate public key
		}

		Key SimpleAccount::DeriveKey(const std::string &payPassword) {
			//todo complete me
			return Key();
		}

		UInt512 SimpleAccount::DeriveSeed(const std::string &payPassword) {
			ErrorCode::StandardLogicError(ErrorCode::WrongAccountType, "Simple account do not support this operation.");
			return UINT512_ZERO;
		}

		void SimpleAccount::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {

			//todo complete me
		}

		nlohmann::json SimpleAccount::ToJson() const {
			//todo complete me

			return nlohmann::json();
		}

		void SimpleAccount::FromJson(const nlohmann::json &j) {
			//todo complete me

		}

		const CMBlock &SimpleAccount::GetEncryptedKey() const {
			return _encryptedKey;
		}

		const CMBlock &SimpleAccount::GetEncryptedMnemonic() const {
			ErrorCode::StandardLogicError(ErrorCode::WrongAccountType, "Simple account do not support this operation.");
			return CMBlock();
		}

		const CMBlock &SimpleAccount::GetEncryptedPhrasePassword() const {
			ErrorCode::StandardLogicError(ErrorCode::WrongAccountType, "Simple account do not support this operation.");
			return CMBlock();
		}

		const std::string &SimpleAccount::GetPublicKey() const {
			return _publicKey;
		}

		const MasterPubKey &SimpleAccount::GetIDMasterPubKey() const {
			ErrorCode::StandardLogicError(ErrorCode::WrongAccountType, "Simple account do not support this operation.");
			return MasterPubKey();
		}
	}
}
