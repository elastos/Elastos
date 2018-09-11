// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/ELACoreExt/ErrorCode.h>
#include "MultiSignAccount.h"

namespace Elastos {
	namespace ElaWallet {

		MultiSignAccount::MultiSignAccount(IAccount *me, const std::vector<std::string> &coSigners) :
				_me(me),
				_coSigners(coSigners) {
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
			//todo complete me
			return nlohmann::json();
		}

		void MultiSignAccount::FromJson(const nlohmann::json &j) {
			//todo complete me

		}

		const CMBlock &MultiSignAccount::GetEncryptedKey() const {
			checkSigners();
			return _me->GetEncryptedKey();
		}

		const CMBlock &MultiSignAccount::GetEncryptedMnemonic() const {
			checkSigners();
			return _me->GetEncryptedMnemonic();
		}

		const CMBlock &MultiSignAccount::GetEncryptedPhrasePassword() const {
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

		void MultiSignAccount::SortSigners() {
			//todo complete me

		}

		void MultiSignAccount::checkSigners() const {
			if (_me == nullptr)
				ErrorCode::StandardLogicError(ErrorCode::WrongAccountType,
											  "Readonly account do not support this operation.");
		}

		std::string MultiSignAccount::GetAddress() {
			SortSigners();
			//todo complete me
			return std::string();
		}
	}
}
