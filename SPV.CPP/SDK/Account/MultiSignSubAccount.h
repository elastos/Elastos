// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELSTOS_SDK_MULTISIGNSUBACCOUNT_H__
#define __ELSTOS_SDK_MULTISIGNSUBACCOUNT_H__

#include "MultiSignAccount.h"
#include "SingleSubAccount.h"
#include "SDK/TransactionHub/TransactionHub.h"

namespace Elastos {
	namespace ElaWallet {

		class MultiSignSubAccount : public SingleSubAccount {
		public:
			MultiSignSubAccount(IAccount *account);

			virtual nlohmann::json GetBasicInfo() const;

			virtual CMBlock GetRedeemScript(const std::string &addr) const;

			virtual bool FindKey(Key &key, const CMBlock &pubKey, const std::string &payPasswd);

			virtual CMBlock GetVotePublicKey() const;

			virtual Key DeriveVoteKey(const std::string &payPasswd);

		private:
			MultiSignAccount *_multiSignAccount;
		};

	}
}

#endif //__ELSTOS_SDK_MULTISIGNSUBACCOUNT_H__
