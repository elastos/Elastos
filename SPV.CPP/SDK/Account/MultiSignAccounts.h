// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MULTISIGNACCOUNTS_H__
#define __ELASTOS_SDK_MULTISIGNACCOUNTS_H__

#include <vector>
#include <map>

#include "MultiSignAccount.h"

namespace Elastos {
	namespace ElaWallet {

		class MultiSignAccounts : public IAccount {
		public:
			MultiSignAccounts(IAccount *innerAccount);

			void AddAccount(const std::vector<std::string> &coSigners, uint32_t requiredSignCount);

			void RemoveAccount(const std::string &address);

			void Begin(const std::string &address);

			void End();

			MultiSignAccount *FindAccount(const std::string &address);

			virtual Key DeriveKey(const std::string &payPassword);

			virtual UInt512 DeriveSeed(const std::string &payPassword);

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

		public: //properties

			virtual const std::string &GetEncryptedKey() const;

			virtual const std::string &GetEncryptedMnemonic() const;

			virtual const std::string &GetEncryptedPhrasePassword() const;

			virtual const std::string &GetPublicKey() const;

			virtual const MasterPubKey &GetIDMasterPubKey() const;

			virtual std::string GetAddress();

		private:
			void checkCurrentAccount() const;

		private:
			typedef boost::shared_ptr<MultiSignAccount> MultiSignAccoutPtr;
			typedef std::map<std::string, MultiSignAccoutPtr> AccountMap;
			AccountMap _accounts;
			MultiSignAccoutPtr _currentAccount;
			AccountPtr _innerAccount;
		};

	}
}

#endif //__ELASTOS_SDK_MULTISIGNACCOUNTS_H__
