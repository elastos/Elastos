// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MULTISIGNACCOUNT_H__
#define __ELASTOS_SDK_MULTISIGNACCOUNT_H__

#include <vector>

#include "IAccount.h"
#include "StandardAccount.h"

namespace Elastos {
	namespace ElaWallet {

		class MultiSignAccount : public IAccount {
		public:
			MultiSignAccount(IAccount *me, const std::vector<std::string> &coSigners, uint32_t requiredSignCount);

			CMBlock GenerateRedeemScript() const;

			virtual Key DeriveKey(const std::string &payPassword);

			virtual UInt512 DeriveSeed(const std::string &payPassword);

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

		public: //properties

			virtual const CMBlock &GetEncryptedKey() const;

			virtual const CMBlock &GetEncryptedMnemonic() const;

			virtual const CMBlock &GetEncryptedPhrasePassword() const;

			virtual const std::string &GetPublicKey() const;

			virtual const MasterPubKey &GetIDMasterPubKey() const;

			virtual std::string GetAddress();

		private:
			JSON_SM_LS(MultiSignAccount);

			JSON_SM_RS(MultiSignAccount);

			TO_JSON(MultiSignAccount);

			FROM_JSON(MultiSignAccount);

			bool Compare(const std::string &a, const std::string &b) const;

			void checkSigners() const;

		private:
			IAccount *_me;
			std::vector<std::string> _coSigners;
			uint32_t _requiredSignCount;
			std::string _address;
		};

	}
}

#endif //__ELASTOS_SDK_MULTISIGNACCOUNT_H__
