// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SimpleAccount.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/BIPs/Address.h>
#include <SDK/Crypto/AES.h>

namespace Elastos {
	namespace ElaWallet {

		SimpleAccount::SimpleAccount(const bytes_t &privKey, const std::string &payPassword) {
			_encryptedKey = AES::EncryptCCM(privKey, payPassword);

			Key key;
			ErrorChecker::CheckLogic(!key.SetPrvKey(privKey), Error::Key, "Invalid private key");
			_publicKey = key.PubKey();
		}

		SimpleAccount::SimpleAccount() {

		}

		Key SimpleAccount::DeriveMultiSignKey(const std::string &payPassword) {
			bytes_t prvKey = AES::DecryptCCM(_encryptedKey, payPassword);

			Key key(prvKey);

			prvKey.clean();

			return key;
		}

		Key SimpleAccount::DeriveVoteKey(const std::string &payPasswd, uint32_t coinIndex, uint32_t account, uint32_t change) {
			return DeriveMultiSignKey(payPasswd);
		}

		uint512 SimpleAccount::DeriveSeed(const std::string &payPassword) {
			ErrorChecker::CheckCondition(true, Error::WrongAccountType,
										 "Simple account can not derive seed");
			return uint512();
		}

		void SimpleAccount::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			ErrorChecker::CheckPassword(newPassword, "New");

			bytes_t key = AES::DecryptCCM(_encryptedKey, oldPassword);
			_encryptedKey = AES::EncryptCCM(key, newPassword);

			key.clean();
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
			ErrorChecker::CheckCondition(true, Error::WrongAccountType,
										 "Simple account can not get mnemonic");
			return _emptyString;
		}

		const std::string &SimpleAccount::GetEncryptedPhrasePassword() const {
			ErrorChecker::CheckCondition(true, Error::WrongAccountType,
										 "Simple account can not get phrase password");
			return _emptyString;
		}

		bytes_t SimpleAccount::GetMultiSignPublicKey() const {
			return _publicKey;
		}

		bytes_t SimpleAccount::GetVotePublicKey() const {
			return _publicKey;
		}


		const HDKeychain &SimpleAccount::GetIDMasterPubKey() const {
			ErrorChecker::CheckCondition(true, Error::WrongAccountType, "Simple account can not get ID master pubkey");
			return HDKeychain();
		}

		Address SimpleAccount::GetAddress() const {
			return Address(PrefixStandard, _publicKey);
		}

		void to_json(nlohmann::json &j, const SimpleAccount &p) {
			j["Key"] = p._encryptedKey;
			j["PublicKey"] = p.GetMultiSignPublicKey().getHex();
		}

		void from_json(const nlohmann::json &j, SimpleAccount &p) {
			p._encryptedKey = j["Key"].get<std::string>();
			p._publicKey.setHex(j["PublicKey"].get<std::string>());
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

			return account.GetMultiSignPublicKey() == account.GetMultiSignPublicKey();
		}

	}
}
