// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SDK/Common/Utils.h"
#include "SDK/Common/WalletTool.h"
#include "Core/BRBIP39Mnemonic.h"
#include "SDK/ELACoreExt/ErrorCode.h"
#include "SDK/Common/ParamChecker.h"
#include "StandardAccount.h"

namespace Elastos {
	namespace ElaWallet {

		StandardAccount::StandardAccount(const std::string &rootPath,
										 const std::string &phrase,
										 const std::string &language,
										 const std::string &phrasePassword,
										 const std::string &payPassword) :
				_rootPath(rootPath),
				_language(language) {

			CMemBlock<char> phraseData;
			phraseData.SetMemFixed(phrase.c_str(), phrase.size() + 1);
			resetMnemonic(_language);

			if (!WalletTool::PhraseIsValid(phraseData, _mnemonic->words())) {
				resetMnemonic("chinese");
				if (!WalletTool::PhraseIsValid(phraseData, _mnemonic->words())) {
					throw std::logic_error("Import key error.");
				}
			}

			CMBlock cbPhrase0 = Utils::convertToMemBlock(phrase);
			_encryptedMnemonic = Utils::encrypt(cbPhrase0, payPassword);
			CMBlock phrasePass = Utils::convertToMemBlock(phrasePassword);
			_encryptedPhrasePass = Utils::encrypt(phrasePass, payPassword);

			//init master public key and private key
			UInt512 seed = DeriveSeed(payPassword);

			BRKey masterKey;
			BRBIP32APIAuthKey(&masterKey, &seed, sizeof(seed));

			CMBlock cbTmp(sizeof(UInt256));
			memcpy(cbTmp, &masterKey.secret, sizeof(UInt256));
			_encryptedKey = Utils::encrypt(cbTmp, payPassword);

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
		}

		StandardAccount::StandardAccount(const std::string &rootPath) :
				_rootPath(rootPath) {

		}

		const CMBlock &StandardAccount::GetEncryptedPhrasePassword() const {
			return _encryptedPhrasePass;
		}

		const CMBlock &StandardAccount::GetEncryptedKey() const {
			return _encryptedKey;
		}

		const CMBlock &StandardAccount::GetEncryptedMnemonic() const {
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

		std::string StandardAccount::GetAddress() {
			//todo complete me
			return "";
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
			j["Key"] = Utils::encodeHex(p.GetEncryptedKey());
			j["Mnemonic"] = Utils::encodeHex(p.GetEncryptedMnemonic());
			j["PhrasePassword"] = Utils::encodeHex(p.GetEncryptedPhrasePassword());
			j["Language"] = p.GetLanguage();
			j["PublicKey"] = p.GetPublicKey();
			j["IDChainCode"] = Utils::UInt256ToString(p.GetIDMasterPubKey().getChainCode());
			j["IDMasterKeyPubKey"] = Utils::encodeHex(p.GetIDMasterPubKey().getPubKey());
		}

		void from_json(const nlohmann::json &j, StandardAccount &p) {
			p._encryptedKey = Utils::decodeHex(j["Key"].get<std::string>());
			p._encryptedMnemonic = Utils::decodeHex(j["Mnemonic"].get<std::string>());
			p._encryptedPhrasePass = Utils::decodeHex(j["PhrasePassword"].get<std::string>());
			p.resetMnemonic(j["Language"].get<std::string>());
			p._publicKey = j["PublicKey"].get<std::string>();
			UInt256 chainCode = Utils::UInt256FromString(j["IDChainCode"].get<std::string>());
			CMBlock pubKey = Utils::decodeHex(j["IDMasterKeyPubKey"].get<std::string>());
			p._masterIDPubKey = MasterPubKey(pubKey, chainCode);
		}

		void StandardAccount::resetMnemonic(const std::string &language) {
			_language = language;
			_mnemonic = boost::shared_ptr<Mnemonic>(new Mnemonic(language, _rootPath));
		}

		UInt512 StandardAccount::DeriveSeed(const std::string &payPassword) {
			UInt512 result;
			std::string mnemonic = Utils::convertToString(
					Utils::decrypt(GetEncryptedMnemonic(), payPassword));
			if (mnemonic.empty())
				ErrorCode::StandardLogicError(ErrorCode::PasswordError, "Invalid password.");

			std::string phrasePassword = GetEncryptedPhrasePassword().GetSize() == 0
										 ? ""
										 : Utils::convertToString(
							Utils::decrypt(GetEncryptedPhrasePassword(), payPassword));

			BRBIP39DeriveKey(&result, mnemonic.c_str(), phrasePassword.c_str());
			return result;
		}

		Key StandardAccount::DeriveKey(const std::string &payPassword) {
			CMBlock keyData = Utils::decrypt(GetEncryptedKey(), payPassword);
			ParamChecker::checkDataNotEmpty(keyData);

			Key key;
			UInt256 secret;
			memcpy(secret.u8, keyData, keyData.GetSize());
			key.setSecret(secret, true);

			return key;
		}

		void StandardAccount::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			ParamChecker::checkPassword(oldPassword, "Old");
			ParamChecker::checkPassword(newPassword, "New");

			CMBlock key = Utils::decrypt(GetEncryptedKey(), oldPassword);
			ParamChecker::checkDataNotEmpty(key, false);
			CMBlock phrasePass = Utils::decrypt(GetEncryptedPhrasePassword(), oldPassword);
			CMBlock mnemonic = Utils::decrypt(GetEncryptedMnemonic(), oldPassword);
			ParamChecker::checkDataNotEmpty(mnemonic, false);

			_encryptedKey = Utils::encrypt(key, newPassword);
			_encryptedPhrasePass = Utils::encrypt(phrasePass, newPassword);
			_encryptedMnemonic = Utils::encrypt(mnemonic, newPassword);

			memset(key, 0, key.GetSize());
			memset(phrasePass, 0, phrasePass.GetSize());
			memset(mnemonic, 0, mnemonic.GetSize());
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

		std::string StandardAccount::GetType() {
			return "Standard";
		}

		bool StandardAccount::IsReadOnly() const {
			return false;
		}

	}
}
