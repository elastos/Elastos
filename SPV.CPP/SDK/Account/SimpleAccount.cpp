// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SimpleAccount.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		SimpleAccount::SimpleAccount(const std::string &privKey, const std::string &payPassword) {
			CMBlock keyData = Utils::decodeHex(privKey);
			Utils::Encrypt(_encryptedKey, keyData, payPassword);

			Key key;
			UInt256 secret;
			memcpy(secret.u8, keyData, keyData.GetSize());
			key.setSecret(secret, true);
			_publicKey = Utils::encodeHex(key.getPubkey());

			memset(keyData, 0, keyData.GetSize());
			var_clean(&secret);
		}

		SimpleAccount::SimpleAccount() {

		}

		Key SimpleAccount::DeriveKey(const std::string &payPassword) {
			CMBlock keyData;
			ParamChecker::CheckDecrypt(!Utils::Decrypt(keyData, GetEncryptedKey(), payPassword));

			Key key;
			UInt256 secret;
			memcpy(secret.u8, keyData, keyData.GetSize());
			key.setSecret(secret, true);

			memset(keyData, 0, keyData.GetSize());
			var_clean(&secret);

			return key;
		}

		UInt512 SimpleAccount::DeriveSeed(const std::string &payPassword) {
			ParamChecker::checkCondition(true, Error::WrongAccountType,
										 "Simple account can not derive seed");
			return UINT512_ZERO;
		}

		void SimpleAccount::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			ParamChecker::checkPassword(newPassword, "New");

			CMBlock key;
			ParamChecker::CheckDecrypt(!Utils::Decrypt(key, GetEncryptedKey(), oldPassword));

			Utils::Encrypt(_encryptedKey, key, newPassword);

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

		const std::string &SimpleAccount::GetEncryptedKey() const {
			return _encryptedKey;
		}

		const std::string &SimpleAccount::GetEncryptedMnemonic() const {
			ParamChecker::checkCondition(true, Error::WrongAccountType,
										 "Simple account can not get mnemonic");
			return _emptyString;
		}

		const std::string &SimpleAccount::GetEncryptedPhrasePassword() const {
			ParamChecker::checkCondition(true, Error::WrongAccountType,
										 "Simple account can not get phrase password");
			return _emptyString;
		}

		const std::string &SimpleAccount::GetPublicKey() const {
			return _publicKey;
		}

		const MasterPubKey &SimpleAccount::GetIDMasterPubKey() const {
			ParamChecker::checkCondition(true, Error::WrongAccountType, "Simple account can not get ID master pubkey");
			return MasterPubKey();
		}

		std::string SimpleAccount::GetAddress() const {
			Key key;
			key.setPubKey(Utils::decodeHex(_publicKey));
			return key.address();
		}

		void to_json(nlohmann::json &j, const SimpleAccount &p) {
			j["Key"] = p.GetEncryptedKey();
			j["PublicKey"] = p.GetPublicKey();
		}

		void from_json(const nlohmann::json &j, SimpleAccount &p) {
			p._encryptedKey = j["Key"].get<std::string>();
			p._publicKey = j["PublicKey"].get<std::string>();
		}

		nlohmann::json SimpleAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Simple";
			return j;
		}

		std::string SimpleAccount::GetType() const {
			return "Simple";
		}

		bool SimpleAccount::IsReadOnly() const {
			return false;
		}

		bool SimpleAccount::IsEqual(const IAccount &account) const {
			if (account.GetType() != GetType())
				return false;

			return account.GetPublicKey() == account.GetPublicKey();
		}

	}
}
