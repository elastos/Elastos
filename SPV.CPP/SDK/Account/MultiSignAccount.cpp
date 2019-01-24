// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MultiSignAccount.h"
#include "AccountFactory.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ByteStream.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ParamChecker.h>

#include <Core/BRInt.h>
#include <Core/BRAddress.h>

#include <BigIntegerLibrary.hh>
#include <secp256k1.h>
#include <set>
#include <boost/bind.hpp>

namespace Elastos {
	namespace ElaWallet {

		MultiSignAccount::MultiSignAccount(IAccount *me, const std::vector<std::string> &coSigners,
										   uint32_t requiredSignCount) :
			_me(me),
			_requiredSignCount(requiredSignCount),
			_coSigners(coSigners) {
		}

		MultiSignAccount::MultiSignAccount(const std::string &rootPath) : _rootPath(rootPath) {

		}

		Key MultiSignAccount::DeriveKey(const std::string &payPassword) {
			checkSigners();
			return _me->DeriveKey(payPassword);
		}

		UInt512 MultiSignAccount::DeriveSeed(const std::string &payPassword) {
			checkSigners();
			return _me->DeriveSeed(payPassword);
		}

		void MultiSignAccount::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			if (_me != nullptr)
				_me->ChangePassword(oldPassword, newPassword);
		}

		nlohmann::json MultiSignAccount::ToJson() const {
			nlohmann::json j;
			to_json(j, *this);
			return j;
		}

		void MultiSignAccount::FromJson(const nlohmann::json &j) {
			from_json(j, *this);
		}

		const std::string &MultiSignAccount::GetEncryptedKey() const {
			checkSigners();
			return _me->GetEncryptedKey();
		}

		const std::string &MultiSignAccount::GetEncryptedMnemonic() const {
			checkSigners();
			return _me->GetEncryptedMnemonic();
		}

		const std::string &MultiSignAccount::GetEncryptedPhrasePassword() const {
			checkSigners();
			return _me->GetEncryptedPhrasePassword();
		}

		const std::string &MultiSignAccount::GetPublicKey() const {
			checkSigners();
			return _me->GetPublicKey();
		}

		const MasterPubKey &MultiSignAccount::GetIDMasterPubKey() const {
			checkSigners();
			return _me->GetIDMasterPubKey();
		}

		void MultiSignAccount::checkSigners() const {
			ParamChecker::checkCondition(_me == nullptr, Error::WrongAccountType,
										 "Readonly account do not support this operation.");
		}

		std::string MultiSignAccount::GetAddress() const {
			if (_address.empty()) {
				Key key;
				std::vector<std::string> pubKeys = _coSigners;
				if (_me != nullptr)
					pubKeys.push_back(_me->GetPublicKey());

				const CMBlock &code = GetRedeemScript();
				// redeem script -> program hash
				UInt168 programHash = Key::CodeToProgramHash(PrefixMultiSign, code);
				// program hash -> address
				_address = Utils::UInt168ToAddress(programHash);
			}

			return _address;
		}

		const CMBlock &MultiSignAccount::GetRedeemScript() const {
			if (_redeemScript.GetSize() == 0) {
				Key key;
				std::vector<std::string> pubKeys = _coSigners;
				if (_me != nullptr)
					pubKeys.push_back(_me->GetPublicKey());

				_redeemScript = key.MultiSignRedeemScript(_requiredSignCount, pubKeys);
			}

			return _redeemScript;
		}

		void to_json(nlohmann::json &j, const MultiSignAccount &p) {
			if (p._me != nullptr) {
				j["InnerAccount"] = p._me->ToJson();
				j["InnerAccountType"] = p._me->GetType();
			}
			j["CoSigners"] = p._coSigners;
			j["RequiredSignCount"] = p._requiredSignCount;
		}

		void from_json(const nlohmann::json &j, MultiSignAccount &p) {
			p._me = AccountPtr(AccountFactory::CreateFromJson(p._rootPath, j, "InnerAccount", "InnerAccountType"));
			p._coSigners = j["CoSigners"].get<std::vector<std::string>>();
			p._requiredSignCount = j["RequiredSignCount"].get<uint32_t>();
		}

		nlohmann::json MultiSignAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Multi-Sign";
			nlohmann::json details;
			std::vector<std::string> signers = _coSigners;
			std::string innerType;
			if (_me != nullptr) {
				nlohmann::json basicInfo = _me->GetBasicInfo();
				innerType = basicInfo["Type"];
				signers.push_back(_me->GetPublicKey());
			} else {
				innerType = "Readonly";
			}
			j["InnerType"] = innerType;

			details["Signers"] = signers;
			details["RequiredSignCount"] = _requiredSignCount;
			j["Details"] = details;
			return j;
		}

		std::string MultiSignAccount::GetType() const {
			return "MultiSign";
		}

		IAccount *MultiSignAccount::GetInnerAccount() const {
			return _me.get();
		}

		const std::vector<std::string> &MultiSignAccount::GetCoSigners() const {
			return _coSigners;
		}

		uint32_t MultiSignAccount::GetRequiredSignCount() const {
			return _requiredSignCount;
		}

		bool MultiSignAccount::IsReadOnly() const {
			return _me == nullptr;
		}

		bool MultiSignAccount::IsEqual(const IAccount &account) const {
			if (account.GetType() != GetType() || IsReadOnly() != account.IsReadOnly())
				return false;

			if (!IsReadOnly() && !account.IsReadOnly()) {
				return account.GetPublicKey() == GetPublicKey();
			}

			return account.GetAddress() == GetAddress();
		}

	}
}
