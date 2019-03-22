// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MultiSignAccount.h"
#include "AccountFactory.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ByteStream.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/BIPs/Address.h>

#include <set>
#include <boost/bind.hpp>

namespace Elastos {
	namespace ElaWallet {

		MultiSignAccount::MultiSignAccount(IAccount *me, const std::vector<std::string> &coSigners,
										   uint32_t requiredSignCount) :
			_me(me),
			_requiredSignCount(requiredSignCount) {

			for (size_t i = 0; i < coSigners.size(); ++i)
				_coSigners.push_back(bytes_t(coSigners[i]));

			std::vector<bytes_t> pubKeys(_coSigners.begin(), _coSigners.end());
			if (_me != nullptr)
				pubKeys.push_back(_me->GetMultiSignPublicKey());

			_address = Address(PrefixMultiSign, pubKeys, _requiredSignCount);
		}

		MultiSignAccount::MultiSignAccount(const std::string &rootPath) : _rootPath(rootPath) {

		}

		Key MultiSignAccount::DeriveMultiSignKey(const std::string &payPassword){
			checkSigners();
			return _me->DeriveMultiSignKey(payPassword);
		}

		uint512 MultiSignAccount::DeriveSeed(const std::string &payPassword) {
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

		const std::string &MultiSignAccount::GetEncryptedMnemonic() const {
			checkSigners();
			return _me->GetEncryptedMnemonic();
		}

		const std::string &MultiSignAccount::GetEncryptedPhrasePassword() const {
			checkSigners();
			return _me->GetEncryptedPhrasePassword();
		}

		bytes_t MultiSignAccount::GetMultiSignPublicKey() const {
			checkSigners();
			return _me->GetMultiSignPublicKey();
		}

		const HDKeychain &MultiSignAccount::GetIDMasterPubKey() const {
			checkSigners();
			return _me->GetIDMasterPubKey();
		}

		void MultiSignAccount::checkSigners() const {
			ErrorChecker::CheckCondition(_me == nullptr, Error::WrongAccountType,
										 "Readonly account do not support this operation.");
		}

		Address MultiSignAccount::GetAddress() const {
			return _address;
		}

		const bytes_t &MultiSignAccount::GetRedeemScript() const {
			return _address.RedeemScript();
		}

		void to_json(nlohmann::json &j, const MultiSignAccount &p) {
			if (p._me != nullptr) {
				j["InnerAccount"] = p._me->ToJson();
				j["InnerAccountType"] = p._me->GetType();
			}
			std::vector<std::string> coSigners;
			for (size_t i = 0; i < p._coSigners.size(); ++i) {
				coSigners.push_back(p._coSigners[i].getHex());
			}
			j["CoSigners"] = coSigners;
			j["RequiredSignCount"] = p._requiredSignCount;
		}

		void from_json(const nlohmann::json &j, MultiSignAccount &p) {
			p._me = AccountPtr(AccountFactory::CreateFromJson(p._rootPath, j, "InnerAccount", "InnerAccountType"));
			p._requiredSignCount = j["RequiredSignCount"].get<uint32_t>();

			std::vector<std::string> coSigners = j["CoSigners"].get<std::vector<std::string>>();
			for (size_t i = 0; i < coSigners.size(); ++i)
				p._coSigners.push_back(bytes_t(coSigners[i]));

			std::vector<bytes_t> pubKeys(p._coSigners.begin(), p._coSigners.end());
			if (p._me != nullptr)
				pubKeys.push_back(p._me->GetMultiSignPublicKey());

			p._address = Address(PrefixMultiSign, pubKeys, p._requiredSignCount);
		}

		nlohmann::json MultiSignAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Multi-Sign";
			nlohmann::json details;

			std::vector<std::string> signers;
			for (size_t i = 0; i < _coSigners.size(); ++i)
				signers.push_back(_coSigners[i].getHex());

			std::string innerType;
			if (_me != nullptr) {
				nlohmann::json basicInfo = _me->GetBasicInfo();
				innerType = basicInfo["Type"];
				signers.push_back(_me->GetMultiSignPublicKey().getHex());
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

		const std::vector<bytes_t> &MultiSignAccount::GetCoSigners() const {
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
				return account.GetMultiSignPublicKey() == GetMultiSignPublicKey() && account.GetAddress() == GetAddress();
			}

			return account.GetAddress() == GetAddress();
		}

	}
}
