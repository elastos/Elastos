// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <Core/BRKey.h>
#include <SDK/Common/Log.h>
#include "SDK/Common/Utils.h"
#include "SDK/Common/WalletTool.h"
#include "Core/BRBIP39Mnemonic.h"
#include "SDK/Common/ParamChecker.h"
#include "StandardAccount.h"

namespace Elastos {
	namespace ElaWallet {

		StandardAccount::StandardAccount(const std::string &rootPath,
										 const std::string &phrase,
										 const std::string &phrasePassword,
										 const std::string &payPassword) :
				_rootPath(rootPath) {

			_mnemonic = boost::shared_ptr<Mnemonic>(new Mnemonic(boost::filesystem::path(_rootPath)));
			std::string standardPhrase;
			ParamChecker::checkCondition(!_mnemonic->PhraseIsValid(phrase, standardPhrase),
										 Error::Mnemonic, "Invalid mnemonic words");
			_language = _mnemonic->GetLanguage();

			Utils::Encrypt(_encryptedMnemonic, standardPhrase, payPassword);
			Utils::Encrypt(_encryptedPhrasePass, phrasePassword, payPassword);

			//init master public key and private key
			UInt512 seed = DeriveSeed(payPassword);

			BRKey masterKey;
			BRBIP32APIAuthKey(&masterKey, &seed, sizeof(seed));

			CMBlock secret(sizeof(UInt256));
			secret.SetMemFixed(masterKey.secret.u8, sizeof(masterKey.secret));
			Utils::Encrypt(_encryptedKey, secret, payPassword);

			Key key(masterKey);
			key.setPublicKey();
			_publicKey = Utils::encodeHex(key.getPubkey());

			//init id chain derived master public key
			BRKey idMasterKey;
			UInt256 idChainCode;
			BRBIP32PrivKeyPath(&idMasterKey, &idChainCode, &seed, sizeof(seed), 1, 0 | BIP32_HARD);
			Key wrapperKey(idMasterKey.secret, idMasterKey.compressed);
			_masterIDPubKey = MasterPubKey(wrapperKey.getPubkey(), idChainCode);

			var_clean(&seed);
			var_clean(&masterKey.secret);
			std::for_each(standardPhrase.begin(), standardPhrase.end(), [](char &c) { c = 0; });
		}

		StandardAccount::StandardAccount(const std::string &rootPath) :
				_rootPath(rootPath) {
			_mnemonic = boost::shared_ptr<Mnemonic>(new Mnemonic(_rootPath));
		}

		const std::string &StandardAccount::GetEncryptedPhrasePassword() const {
			return _encryptedPhrasePass;
		}

		const std::string &StandardAccount::GetEncryptedKey() const {
			return _encryptedKey;
		}

		const std::string &StandardAccount::GetEncryptedMnemonic() const {
			return _encryptedMnemonic;
		}

		const std::string &StandardAccount::GetPublicKey() const {
			return _publicKey;
		}

		const std::string &StandardAccount::GetLanguage() const {
			return _language;
		}

		const MasterPubKey &StandardAccount::GetIDMasterPubKey() const {
			return _masterIDPubKey;
		}

		std::string StandardAccount::GetAddress() const {
			Key key;
			key.setPubKey(Utils::decodeHex(_publicKey));
			return key.address();
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
			j["Key"] = p.GetEncryptedKey();
			j["Mnemonic"] = p.GetEncryptedMnemonic();
			j["PhrasePassword"] = p.GetEncryptedPhrasePassword();
			j["Language"] = p.GetLanguage();
			j["PublicKey"] = p.GetPublicKey();
			j["IDChainCode"] = Utils::UInt256ToString(p.GetIDMasterPubKey().getChainCode());
			j["IDMasterKeyPubKey"] = Utils::encodeHex(p.GetIDMasterPubKey().getPubKey());
		}

		void from_json(const nlohmann::json &j, StandardAccount &p) {
			p._encryptedKey = j["Key"].get<std::string>();
			p._encryptedMnemonic = j["Mnemonic"].get<std::string>();
			p._encryptedPhrasePass = j["PhrasePassword"].get<std::string>();
			p._language = j["Language"].get<std::string>();
			p._publicKey = j["PublicKey"].get<std::string>();
			UInt256 chainCode = Utils::UInt256FromString(j["IDChainCode"].get<std::string>());
			CMBlock pubKey = Utils::decodeHex(j["IDMasterKeyPubKey"].get<std::string>());
			p._masterIDPubKey = MasterPubKey(pubKey, chainCode);
			p._mnemonic->LoadLanguage(p._language);
		}

		UInt512 StandardAccount::DeriveSeed(const std::string &payPassword) {
			UInt512 result;

			std::string phrase;
			ParamChecker::CheckDecrypt(!Utils::Decrypt(phrase, GetEncryptedMnemonic(), payPassword));

			std::string phrasePassword;
			ParamChecker::CheckDecrypt(!Utils::Decrypt(phrasePassword, GetEncryptedPhrasePassword(), payPassword));

			BRBIP39DeriveKey(&result, phrase.c_str(), phrasePassword.c_str());

			return result;
		}

		Key StandardAccount::DeriveKey(const std::string &payPassword) {
			CMBlock keyData;
			ParamChecker::CheckDecrypt(!Utils::Decrypt(keyData, GetEncryptedKey(), payPassword));

			Key key;
			UInt256 secret;
			memcpy(secret.u8, keyData, keyData.GetSize());
			key.setSecret(secret, true);

			return key;
		}

		void StandardAccount::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			ParamChecker::checkPassword(newPassword, "New");

			CMBlock key;
			std::string phrasePasswd, phrase;
			ParamChecker::CheckDecrypt(!Utils::Decrypt(key, GetEncryptedKey(), oldPassword));
			ParamChecker::CheckDecrypt(!Utils::Decrypt(phrasePasswd, GetEncryptedPhrasePassword(), oldPassword));
			ParamChecker::CheckDecrypt(!Utils::Decrypt(phrase, GetEncryptedMnemonic(), oldPassword));

			Utils::Encrypt(_encryptedKey, key, newPassword);
			Utils::Encrypt(_encryptedPhrasePass, phrasePasswd, newPassword);
			Utils::Encrypt(_encryptedMnemonic, phrase, newPassword);

			memset(key, 0, key.GetSize());
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

			return GetPublicKey() == account.GetPublicKey();
		}

	}
}
