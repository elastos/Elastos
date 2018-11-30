// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MULTISIGNACCOUNT_H__
#define __ELASTOS_SDK_MULTISIGNACCOUNT_H__

#include "IAccount.h"
#include "StandardAccount.h"

#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class MultiSignAccount : public IAccount {
		public:
			MultiSignAccount(IAccount *me, const std::vector<std::string> &coSigners, uint32_t requiredSignCount);

			CMBlock GenerateRedeemScript() const;

			IAccount *GetInnerAccount() const;

			const std::vector<std::string> &GetCoSigners() const;

			uint32_t GetRequiredSignCount() const;

			virtual nlohmann::json GetBasicInfo() const;

			virtual Key DeriveKey(const std::string &payPassword);

			virtual UInt512 DeriveSeed(const std::string &payPassword);

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

			virtual std::string GetType() const;

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

			virtual bool IsReadOnly() const;

			virtual bool IsEqual(const IAccount &account) const;

		public: //properties

			virtual const std::string &GetEncryptedKey() const;

			virtual const std::string &GetEncryptedMnemonic() const;

			virtual const std::string &GetEncryptedPhrasePassword() const;

			virtual const std::string &GetPublicKey() const;

			virtual const MasterPubKey &GetIDMasterPubKey() const;

			virtual std::string GetAddress() const;

		private:
			friend class AccountFactory;

			MultiSignAccount(const std::string &rootPath);

			JSON_SM_LS(MultiSignAccount);

			JSON_SM_RS(MultiSignAccount);

			TO_JSON(MultiSignAccount);

			FROM_JSON(MultiSignAccount);

			bool Compare(const std::string &a, const std::string &b) const;

			void checkSigners() const;

		private:
			AccountPtr _me;
			std::vector<std::string> _coSigners;
			uint32_t _requiredSignCount;
			mutable std::string _address;
			std::string _rootPath;
		};

	}
}

#endif //__ELASTOS_SDK_MULTISIGNACCOUNT_H__
