// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MultiSignAccounts.h"
#include "ErrorCode.h"

namespace Elastos {
	namespace ElaWallet {

		MultiSignAccounts::MultiSignAccounts(IAccount *innerAccount) :
				_currentAccount(nullptr),
				_innerAccount(innerAccount) {

		}

		void MultiSignAccounts::AddAccount(const std::vector<std::string> &coSigners, uint32_t requiredSignCount) {
			MultiSignAccount *account = new MultiSignAccount(_innerAccount.get(), coSigners, requiredSignCount);
			if (_accounts.find(account->GetAddress()) == _accounts.end()) {
				_accounts[account->GetAddress()] = MultiSignAccoutPtr(account);
			}
		}

		void MultiSignAccounts::RemoveAccount(const std::string &address) {
			if (_accounts.find(address) != _accounts.end())
				_accounts.erase(address);
		}

		void MultiSignAccounts::Begin(const std::string &address) {
			if (_accounts.find(address) != _accounts.end())
				_currentAccount = _accounts[address];
		}

		void MultiSignAccounts::End() {
			_currentAccount.reset();
		}

		MultiSignAccount *MultiSignAccounts::FindAccount(const std::string &address) {
			if (_accounts.find(address) != _accounts.end())
				return _accounts[address].get();
			return nullptr;
		}

		Key MultiSignAccounts::DeriveKey(const std::string &payPassword) {
			checkCurrentAccount();
			return _currentAccount->DeriveKey(payPassword);
		}

		UInt512 MultiSignAccounts::DeriveSeed(const std::string &payPassword) {
			checkCurrentAccount();
			return _currentAccount->DeriveSeed(payPassword);
		}

		void MultiSignAccounts::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			checkCurrentAccount();
			_currentAccount->ChangePassword(oldPassword, newPassword);
		}

		nlohmann::json MultiSignAccounts::ToJson() const {
			//todo complete me
			return nlohmann::json();
		}

		void MultiSignAccounts::FromJson(const nlohmann::json &j) {
			//todo complete me

		}

		const CMBlock &MultiSignAccounts::GetEncryptedKey() const {
			checkCurrentAccount();
			return _currentAccount->GetEncryptedKey();
		}

		const CMBlock &MultiSignAccounts::GetEncryptedMnemonic() const {
			checkCurrentAccount();
			return _currentAccount->GetEncryptedMnemonic();
		}

		const CMBlock &MultiSignAccounts::GetEncryptedPhrasePassword() const {
			checkCurrentAccount();
			return _currentAccount->GetEncryptedPhrasePassword();
		}

		const std::string &MultiSignAccounts::GetPublicKey() const {
			checkCurrentAccount();
			return _currentAccount->GetPublicKey();
		}

		const MasterPubKey &MultiSignAccounts::GetIDMasterPubKey() const {
			checkCurrentAccount();
			return _currentAccount->GetIDMasterPubKey();
		}

		void MultiSignAccounts::checkCurrentAccount() const {
			if (_currentAccount == nullptr)
				ErrorCode::StandardLogicError(ErrorCode::NoCurrentMultiSinAccount, "Do not have current account.");
		}

		std::string MultiSignAccounts::GetAddress() {
			checkCurrentAccount();
			return _currentAccount->GetAddress();
		}
	}
}
