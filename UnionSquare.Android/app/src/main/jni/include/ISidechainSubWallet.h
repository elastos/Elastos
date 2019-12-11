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
			 * @param amount specify amount we want to send.
			 * @param mainChainAddress mainchain address.
			 * @param memo input memo attribute for describing.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateWithdrawTransaction(
					const std::string &fromAddress,
					const std::string &amount,
					const std::string &mainChainAddress,
					const std::string &memo) = 0;

			/**
			 * Get genesis address of the side chain, the address is a special address will be set to toAddress in CreateDepositTransaction.
			 * @return genesis address of the side chain.
			 */
			virtual std::string GetGenesisAddress() const = 0;

		};

	}
}

#endif //__ELASTOS_SDK_ISIDECHAINSUBWALLET_H__
