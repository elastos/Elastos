// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MULTISIGNACCOUNT_H__
#define __ELASTOS_SDK_MULTISIGNACCOUNT_H__

#include <SDK/Account/IAccount.h>
#include <SDK/Common/Mstream.h>

#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class MultiSignAccount : public IAccount {
		public:
			MultiSignAccount(IAccount *me, const std::vector<std::string> &coSigners, uint32_t requiredSignCount);

			const bytes_t &GetRedeemScript() const;

			IAccount *GetInnerAccount() const;

			const std::vector<bytes_t> &GetCoSigners() const;

			uint32_t GetRequiredSignCount() const;

			virtual nlohmann::json GetBasicInfo() const;

			virtual Key DeriveMultiSignKey(const std::string &payPassword);

			virtual uint512 DeriveSeed(const std::string &payPassword);

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

			virtual std::string GetType() const;

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

			virtual bool IsReadOnly() const;

			virtual bool IsEqual(const IAccount &account) const;

		public: //properties

			virtual const std::string &GetEncryptedMnemonic() const;

			virtual const std::string &GetEncryptedPhrasePassword() const;

			virtual bytes_t GetMultiSignPublicKey() const;

			virtual const HDKeychain &GetIDMasterPubKey() const;

			virtual Address GetAddress() const;

		private:
			friend class AccountFactory;

			MultiSignAccount(const std::string &rootPath);

			JSON_SM_LS(MultiSignAccount);

			JSON_SM_RS(MultiSignAccount);

			TO_JSON(MultiSignAccount);

			FROM_JSON(MultiSignAccount);

			void checkSigners() const;

		private:
			mutable bytes_t _redeemScript;
			AccountPtr _me;
			std::vector<bytes_t> _coSigners;
			uint32_t _requiredSignCount;
			mutable Address _address;
			std::string _rootPath;
		};

	}
}

#endif //__ELASTOS_SDK_MULTISIGNACCOUNT_H__
