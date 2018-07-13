// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ISubWallet.h"

#ifndef __ELASTOS_SDK_ISIDECHAINSUBWALLET_H__
#define __ELASTOS_SDK_ISIDECHAINSUBWALLET_H__

namespace Elastos {
	namespace ElaWallet {

		class ISidechainSubWallet : public virtual ISubWallet {
		public:
			/**
			 * Virtual destructor.
			 */
			virtual ~ISidechainSubWallet() noexcept {}

			/**
			 * Create a withdraw transaction and return the content of transaction in json format. Note that \p amount should greater than sum of \p so that we will leave enough fee for mainchain.
			 * @param fromAddress specify which address we want to spend, or just input empty string to let wallet choose UTXOs automatically.
			 * @param toAddress specify which address we want to send, in this method to address shall be destroy address of the side chain. Note that destroy address is a special address generate by a program hash that all bytes set to zero.
			 * @param amount specify amount we want to send.
			 * @param mainchainAccounts a list of mainchain accounts in json format.
			 * @param mainchainAmounts a list of mainchain amounts in json format, each amount should correspond to \p mainchainAccounts by order.
			 * @param mainchainIndexs a list of mainchain indices in json format, each index should correspond to \p mainchainAccounts by order.
			 * @param fee [Obsoleted] specify fee for miners.
			 * @param memo input memo attribute for describing.
			 * @param remark is used to record message of local wallet.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateWithdrawTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					const uint64_t amount,
					const nlohmann::json& mainchainAccounts,
					const nlohmann::json& mainchainAmounts,
					const nlohmann::json &mainchainIndexs,
					uint64_t fee,
					const std::string &memo,
					const std::string &remark) = 0;

			/**
			 * Get genesis address of the side chain, the address is a special address will be set to toAddress in CreateDepositTransaction.
			 * @return genesis address of the side chain.
			 */
			virtual std::string GetGenesisAddress() const = 0;
		};

	}
}

#endif //__ELASTOS_SDK_ISIDECHAINSUBWALLET_H__
