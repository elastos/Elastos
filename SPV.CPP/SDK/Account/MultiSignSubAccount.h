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

			virtual bytes_t GetRedeemScript(const Address &addr) const;

			virtual bool FindKey(Key &key, const bytes_t &pubKey, const std::string &payPasswd);

			virtual bytes_t GetOwnerPublicKey() const;

			virtual Key DeriveOwnerKey(const std::string &payPasswd);

			virtual size_t TxInternalChainIndex(const TransactionPtr &tx) const;

			virtual size_t TxExternalChainIndex(const TransactionPtr &tx) const;

		private:
			MultiSignAccount *_multiSignAccount;
		};

	}
}

#endif //__ELSTOS_SDK_MULTISIGNSUBACCOUNT_H__
