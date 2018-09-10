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

		StandardAccount::StandardAccount(const std::string &rootPath) : _rootPath(rootPath) {

		}

		const CMBlock &StandardAccount::GetEncrptedPhrasePassword() const {
			return _encryptedPhrasePass;
		}

		void StandardAccount::SetEncryptedPhrasePassword(const CMBlock &data) {
			_encryptedPhrasePass = data;
		}

		const CMBlock &StandardAccount::GetEncrpytedKey() const {
			return _encryptedKey;
		}

		void StandardAccount::SetEncryptedKey(const CMBlock &data) {
			_encryptedKey = data;
		}

		const CMBlock &StandardAccount::GetEncryptedMnemonic() const {
			return _encryptedMnemonic;
		}

		void StandardAccount::SetEncryptedMnemonic(const CMBlock &data) {
			_encryptedMnemonic = data;
		}

		const std::string &StandardAccount::GetPublicKey() const {
			return _publicKey;
		}

		void StandardAccount::SetPublicKey(const std::string &pubKey) {
			_publicKey = pubKey;
		}

		const std::string &StandardAccount::GetLanguage() const {
			return _language;
		}

		void StandardAccount::SetLanguage(const std::string &language) {
			_language = language;
		}

		const MasterPubKey &StandardAccount::GetIDMasterPubKey() const {
			return _masterIDPubKey;
		}

		void StandardAccount::SetIDMasterPubKey(const MasterPubKey &masterPubKey) {
			_masterIDPubKey = masterPubKey;
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
			j["Key"] = Utils::encodeHex(p.GetEncrpytedKey());
			j["Mnemonic"] = Utils::encodeHex(p.GetEncryptedMnemonic());
			j["PhrasePassword"] = Utils::encodeHex(p.GetEncrptedPhrasePassword());
			j["Language"] = p.GetLanguage();
			j["PublicKey"] = p.GetPublicKey();
			j["IDChainCode"] = Utils::UInt256ToString(p.GetIDMasterPubKey().getChainCode());
			j["IDMasterKeyPubKey"] = Utils::encodeHex(p.GetIDMasterPubKey().getPubKey());
		}

		void from_json(const nlohmann::json &j, StandardAccount &p) {
			p.SetEncryptedKey(Utils::decodeHex(j["Key"].get<std::string>()));
			p.SetEncryptedMnemonic(Utils::decodeHex(j["Mnemonic"].get<std::string>()));
			p.SetEncryptedPhrasePassword(Utils::decodeHex(j["PhrasePassword"].get<std::string>()));
			p.SetLanguage(j["Language"].get<std::string>());
			p.SetPublicKey(j["PublicKey"].get<std::string>());
			UInt256 chainCode = Utils::UInt256FromString(j["IDChainCode"].get<std::string>());
			CMBlock pubKey = Utils::decodeHex(j["IDMasterKeyPubKey"].get<std::string>());
			p.SetIDMasterPubKey(MasterPubKey(pubKey, chainCode));
		}

		bool StandardAccount::initFromPhrase(const std::string &phrase, const std::string &phrasePassword,
											 const std::string &payPassword) {
			CMemBlock<char> phraseData;
			phraseData.SetMemFixed(phrase.c_str(), phrase.size() + 1);
			std::string originLanguage = _mnemonic->getLanguage();
			if (!_language.empty() && _language != originLanguage) {
				resetMnemonic(_language);
			}

			if (!WalletTool::PhraseIsValid(phraseData, _mnemonic->words())) {
				resetMnemonic("chinese");
				if (!WalletTool::PhraseIsValid(phraseData, _mnemonic->words())) {
					throw std::logic_error("Import key error.");
				}
			}

			CMBlock cbPhrase0 = Utils::convertToMemBlock(phrase);
			SetEncryptedMnemonic(Utils::encrypt(cbPhrase0, payPassword));
			CMBlock phrasePass = Utils::convertToMemBlock(phrasePassword);
			SetEncryptedPhrasePassword(Utils::encrypt(phrasePass, payPassword));

			//init master public key and private key
			UInt512 seed = deriveSeed(payPassword);

			BRKey masterKey;
			BRBIP32APIAuthKey(&masterKey, &seed, sizeof(seed));

			CMBlock cbTmp(sizeof(UInt256));
			memcpy(cbTmp, &masterKey.secret, sizeof(UInt256));
			SetEncryptedKey(Utils::encrypt(cbTmp, payPassword));

			Key key(masterKey);
			key.setPublicKey();
			SetPublicKey(Utils::encodeHex(key.getPubkey()));

			//init id chain derived master public key
			BRKey idMasterKey;
			UInt256 idChainCode;
			BRBIP32PrivKeyPath(&idMasterKey, &idChainCode, &seed, sizeof(seed), 1, 0 | BIP32_HARD);
			Key wrapperKey(idMasterKey.secret, idMasterKey.compressed);
			SetIDMasterPubKey(MasterPubKey(wrapperKey.getPubkey(), idChainCode));

			var_clean(&seed);
			var_clean(&masterKey.secret);

			return true;
		}

		void StandardAccount::resetMnemonic(const std::string &language) {
			SetLanguage(language);
			_mnemonic = boost::shared_ptr<Mnemonic>(new Mnemonic(language, _rootPath));
		}

		UInt512 StandardAccount::deriveSeed(const std::string &payPassword) {
			UInt512 result;
			std::string mnemonic = Utils::convertToString(
					Utils::decrypt(GetEncryptedMnemonic(), payPassword));
			if (mnemonic.empty())
				ErrorCode::StandardLogicError(ErrorCode::PasswordError, "Invalid password.");

			std::string phrasePassword = GetEncrptedPhrasePassword().GetSize() == 0
										 ? ""
										 : Utils::convertToString(
							Utils::decrypt(GetEncrptedPhrasePassword(), payPassword));

			BRBIP39DeriveKey(&result, mnemonic.c_str(), phrasePassword.c_str());
			return result;
		}

		Key StandardAccount::deriveKey(const std::string &payPassword) {
			CMBlock keyData = Utils::decrypt(GetEncrpytedKey(), payPassword);
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

			CMBlock key = Utils::decrypt(GetEncrpytedKey(), oldPassword);
			ParamChecker::checkDataNotEmpty(key, false);
			CMBlock phrasePass = Utils::decrypt(GetEncrptedPhrasePassword(), oldPassword);
			CMBlock mnemonic = Utils::decrypt(GetEncryptedMnemonic(), oldPassword);
			ParamChecker::checkDataNotEmpty(mnemonic, false);

			SetEncryptedKey(Utils::encrypt(key, newPassword));
			SetEncryptedPhrasePassword(Utils::encrypt(phrasePass, newPassword));
			SetEncryptedMnemonic(Utils::encrypt(mnemonic, newPassword));
		}

	}
}
