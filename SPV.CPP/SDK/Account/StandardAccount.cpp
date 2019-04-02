// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "StandardAccount.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/WalletCore/BIPs/Address.h>
#include <SDK/WalletCore/Crypto/AES.h>
#include <SDK/WalletCore/BIPs/Mnemonic.h>


namespace Elastos {
	namespace ElaWallet {

		StandardAccount::StandardAccount(const std::string &rootPath,
										 const std::string &mnemonic,
										 const std::string &phrasePassword,
										 const std::string &payPassword) :
				_rootPath(rootPath) {

			_mnemonic = boost::shared_ptr<Mnemonic>(new Mnemonic(boost::filesystem::path(_rootPath)));

			_encryptedMnemonic = AES::EncryptCCM(bytes_t(mnemonic.c_str(), mnemonic.size()), payPassword);
			_encryptedPhrasePass = AES::EncryptCCM(bytes_t(phrasePassword.c_str(), phrasePassword.size()), payPassword);

			Key key = DeriveMultiSignKey(payPassword);
			_multiSignPublicKey = key.PubKey();

			_masterIDPubKey = DeriveIDMasterPubKey(payPassword);
		}

		StandardAccount::StandardAccount(const std::string &rootPath) :
				_rootPath(rootPath) {
			_mnemonic = boost::shared_ptr<Mnemonic>(new Mnemonic(_rootPath));
		}

		const std::string &StandardAccount::GetEncryptedPhrasePassword() const {
			return _encryptedPhrasePass;
		}

		const std::string &StandardAccount::GetEncryptedMnemonic() const {
			return _encryptedMnemonic;
		}

		bytes_t StandardAccount::GetMultiSignPublicKey() const {
			return _multiSignPublicKey;
		}

		const std::string &StandardAccount::GetLanguage() const {
			return _language;
		}

		const HDKeychain &StandardAccount::GetIDMasterPubKey() const {
			return _masterIDPubKey;
		}

		Address StandardAccount::GetAddress() const {
			return Address(PrefixStandard, _multiSignPublicKey);
		}

		nlohmann::json &operator<<(nlohmann::json &j, const StandardAccount &p) {
			to_json(j, p);

			return j;
		}

		const nlohmann::json &operator>>(const nlohmann::json &j, StandardAccount &p) {
			from_json(j, p);

			return j;
		}

		void to_json(nlohmann::json &j, const StandardAccount &p) {
			j["Mnemonic"] = p.GetEncryptedMnemonic();
			j["PhrasePassword"] = p.GetEncryptedPhrasePassword();
			j["Language"] = p.GetLanguage();
			j["PublicKey"] = p.GetMultiSignPublicKey().getHex();
			j["IDChainCode"] = p.GetIDMasterPubKey().chain_code().getHex();
			j["IDMasterKeyPubKey"] = p.GetIDMasterPubKey().pubkey().getHex();
		}

		void from_json(const nlohmann::json &j, StandardAccount &p) {
			p._encryptedMnemonic = j["Mnemonic"].get<std::string>();
			p._encryptedPhrasePass = j["PhrasePassword"].get<std::string>();
			p._language = j["Language"].get<std::string>();
			p._multiSignPublicKey.setHex(j["PublicKey"].get<std::string>());
			bytes_t chainCode = j["IDChainCode"].get<std::string>();
			bytes_t pubKey = j["IDMasterKeyPubKey"].get<std::string>();
			p._masterIDPubKey = HDKeychain(pubKey, chainCode);
		}

		uint512 StandardAccount::DeriveSeed(const std::string &payPassword) {
			bytes_t bytes = AES::DecryptCCM(GetEncryptedMnemonic(), payPassword);
			std::string mnemonic((char *)&bytes[0], bytes.size());

			bytes = AES::DecryptCCM(GetEncryptedPhrasePassword(), payPassword);
			std::string phrasePassword((char *)&bytes[0], bytes.size());

			return _mnemonic->DeriveSeed(mnemonic, phrasePassword);
		}

		Key StandardAccount::DeriveMultiSignKey(const std::string &payPassword) {
			HDSeed hdseed(DeriveSeed(payPassword).bytes());
			HDKeychain rootKey(hdseed.getExtendedKey(true));

			return rootKey.getChild("1'/0");
		}

		HDKeychain StandardAccount::DeriveIDMasterPubKey(const std::string &payPasswd) {
			HDSeed hdseed(DeriveSeed(payPasswd).bytes());
			HDKeychain rootKey(hdseed.getExtendedKey(true));

			HDKeychain IDMasterPubKey = rootKey.getChild("44'/0'/0'").getPublic();

			return IDMasterPubKey;
		}

		void StandardAccount::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			ErrorChecker::CheckPassword(newPassword, "New");

			bytes_t phrasePasswd = AES::DecryptCCM(GetEncryptedPhrasePassword(), oldPassword);
			bytes_t phrase = AES::DecryptCCM(GetEncryptedMnemonic(), oldPassword);

			_encryptedPhrasePass = AES::EncryptCCM(phrasePasswd, newPassword);
			_encryptedMnemonic = AES::EncryptCCM(phrase, newPassword);

			phrase.clean();
			phrasePasswd.clean();
		}

		nlohmann::json StandardAccount::ToJson() const {
			nlohmann::json j;
			to_json(j, *this);
			return j;
		}

		void StandardAccount::FromJson(const nlohmann::json &j) {
			from_json(j, *this);
		}

		nlohmann::json StandardAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Standard";
			return j;
		}

		std::string StandardAccount::GetType() const {
			return "Standard";
		}

		bool StandardAccount::IsReadOnly() const {
			return false;
		}

		bool StandardAccount::IsEqual(const IAccount &account) const {
			if (GetType() != account.GetType())
				return false;

			return GetMultiSignPublicKey() == account.GetMultiSignPublicKey();
		}

	}
}
