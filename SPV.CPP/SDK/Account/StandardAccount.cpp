// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "StandardAccount.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/BIPs/BIP32Sequence.h>
#include <SDK/Base/Address.h>

#include <Core/BRCrypto.h>
#include <Core/BRBIP39Mnemonic.h>

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

			Key key = DeriveMultiSignKey(payPassword);
			_multiSignPublicKey = key.PubKey();

			_masterIDPubKey = DeriveIDMasterPubKey(payPassword);

			std::for_each(standardPhrase.begin(), standardPhrase.end(), [](char &c) { c = 0; });
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

		CMBlock StandardAccount::GetMultiSignPublicKey() const {
			return _multiSignPublicKey;
		}

		const std::string &StandardAccount::GetLanguage() const {
			return _language;
		}

		const MasterPubKey &StandardAccount::GetIDMasterPubKey() const {
			return _masterIDPubKey;
		}

		Address StandardAccount::GetAddress() const {
			return Address(_multiSignPublicKey, PrefixStandard);
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
			j["PublicKey"] = Utils::encodeHex(p.GetMultiSignPublicKey());
			j["IDChainCode"] = Utils::UInt256ToString(p.GetIDMasterPubKey().GetChainCode());
			j["IDMasterKeyPubKey"] = Utils::encodeHex(p.GetIDMasterPubKey().GetPubKey());
		}

		void from_json(const nlohmann::json &j, StandardAccount &p) {
			p._encryptedMnemonic = j["Mnemonic"].get<std::string>();
			p._encryptedPhrasePass = j["PhrasePassword"].get<std::string>();
			p._language = j["Language"].get<std::string>();
			p._multiSignPublicKey = Utils::decodeHex(j["PublicKey"].get<std::string>());
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

		Key StandardAccount::DeriveMultiSignKey(const std::string &payPassword) {
			UInt512 seed = DeriveSeed(payPassword);

			Key key = BIP32Sequence::APIAuthKey(&seed, sizeof(seed));

			var_clean(&seed);

			return key;
		}

		MasterPubKey StandardAccount::DeriveIDMasterPubKey(const std::string &payPasswd) {
			UInt512 seed = DeriveSeed(payPasswd);
			UInt256 chainCode;
			Key IDMasterKey = BIP32Sequence::PrivKeyPath(&seed, sizeof(seed), chainCode, 1, 0 | BIP32_HARD);
			MasterPubKey IDmasterPubKey = MasterPubKey(IDMasterKey.PubKey(), chainCode);

			var_clean(&seed);
			var_clean(&chainCode);

			return IDmasterPubKey;
		}

		void StandardAccount::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			ParamChecker::checkPassword(newPassword, "New");

			CMBlock key;
			std::string phrasePasswd, phrase;
			ParamChecker::CheckDecrypt(!Utils::Decrypt(phrasePasswd, GetEncryptedPhrasePassword(), oldPassword));
			ParamChecker::CheckDecrypt(!Utils::Decrypt(phrase, GetEncryptedMnemonic(), oldPassword));

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

			return GetMultiSignPublicKey() == account.GetMultiSignPublicKey();
		}

	}
}
