// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ISubWallet.h"

#ifndef __ELASTOS_SDK_IMAINCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IMAINCHAINSUBWALLET_H__

namespace Elastos {
	namespace ElaWallet {

		class IMainchainSubWallet : public virtual ISubWallet {
		public:
			/**
			 * Virtual destructor.
			 */
			virtual ~IMainchainSubWallet() noexcept {}

			/**
			 * Create a deposit transaction and return the content of transaction in json format. Note that \p amount should greater than sum of \p so that we will leave enough fee for sidechain.
			 * @param fromAddress specify which address we want to spend, or just input empty string to let wallet choose UTXOs automatically.
			 * @param toAddress specify which address we want to send, in this method to address shall be genesis address of the side chain
			 * @param amount specify amount we want to send.
			 * @param sidechainAccounts a list of sidechain accounts in json format.
			 * @param sidechainAmounts a list of sidechain amounts in json format, each amount should correspond to \p sidechainAccounts by order.
			 * @param sidechainIndices a list of sidechain indices in json format, each index should correspond to \p sidechainAccounts by order.
			 * @param fee [Obsoleted] specify fee for miners.
			 * @param memo input memo attribute for describing.
			 * @param remark is used to record message of local wallet.
			 * @return　If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateDepositTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					const uint64_t amount,
					const nlohmann::json &sidechainAccounts,
					const nlohmann::json &sidechainAmounts,
					const nlohmann::json &sidechainIndices,
					uint64_t fee,
					const std::string &memo,
					const std::string &remark) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IMAINCHAINSUBWALLET_H__
