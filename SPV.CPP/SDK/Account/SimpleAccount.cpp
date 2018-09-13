// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>
#include "SimpleAccount.h"
#include "ErrorCode.h"

namespace Elastos {
	namespace ElaWallet {

		SimpleAccount::SimpleAccount(const std::string &privKey, const std::string &payPassword) {
			CMBlock keyData = Utils::decodeHex(privKey);
			_encryptedKey = Utils::encrypt(keyData, payPassword);

			Key key;
			UInt256 secret;
			memcpy(secret.u8, keyData, keyData.GetSize());
			key.setSecret(secret, true);
			_publicKey = Utils::encodeHex(key.getPubkey());

			memset(keyData, 0, keyData.GetSize());
			var_clean(&secret);
		}

		Key SimpleAccount::DeriveKey(const std::string &payPassword) {
			CMBlock keyData = Utils::decrypt(GetEncryptedKey(), payPassword);
			ParamChecker::checkDataNotEmpty(keyData);

			Key key;
			UInt256 secret;
			memcpy(secret.u8, keyData, keyData.GetSize());
			key.setSecret(secret, true);

			memset(keyData, 0, keyData.GetSize());
			var_clean(&secret);

			return key;
		}

		UInt512 SimpleAccount::DeriveSeed(const std::string &payPassword) {
			ErrorCode::StandardLogicError(ErrorCode::WrongAccountType, "Simple account do not support this operation.");
			return UINT512_ZERO;
		}

		void SimpleAccount::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			ParamChecker::checkPassword(oldPassword, "Old");
			ParamChecker::checkPassword(newPassword, "New");

			CMBlock key = Utils::decrypt(GetEncryptedKey(), oldPassword);
			ParamChecker::checkDataNotEmpty(key, false);
			_encryptedKey = Utils::encrypt(key, newPassword);

			memset(key, 0, key.GetSize());
		}

		nlohmann::json SimpleAccount::ToJson() const {
			nlohmann::json j;
			to_json(j, *this);
			return j;
		}

		void SimpleAccount::FromJson(const nlohmann::json &j) {
			from_json(j, *this);
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

		std::string SimpleAccount::GetAddress() {
			//todo complete me
			return std::string();
		}

		void to_json(nlohmann::json &j, const SimpleAccount &p) {
			j["Key"] = Utils::encodeHex(p.GetEncryptedKey());
			j["PublicKey"] = p.GetPublicKey();
		}

		void from_json(const nlohmann::json &j, SimpleAccount &p) {
			p._encryptedKey = Utils::decodeHex(j["Key"].get<std::string>());
			p._publicKey = j["PublicKey"].get<std::string>();
		}

		nlohmann::json SimpleAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Simple";
			return j;
		}

	}
}
