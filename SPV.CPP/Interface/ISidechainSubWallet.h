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
			 * Get balances of all addresses in json format.
			 * @param assetID asset hex code from asset hash.
			 * @return balances of all addresses in json format.
			 */
			virtual nlohmann::json GetBalanceInfo(const std::string &assetID) const = 0;

			/**
			 * Get sum of balances of all addresses.
			 * @param assetID asset hex code from asset hash.
			 * @return sum of balances.
			 */
			virtual uint64_t GetBalance(const std::string &assetID) const = 0;

			/**
			 * Get balance of only the specified address.
			 * @param assetID asset hex code from asset hash.
			 * @param address is one of addresses created by current sub wallet.
			 * @return balance of specified address.
			 */
			virtual uint64_t GetBalanceWithAddress(const std::string &assetID, const std::string &address) const = 0;

			/**
			 * Create a normal transaction and return the content of transaction in json format.
			 * @param fromAddress specify which address we want to spend, or just input empty string to let wallet choose UTXOs automatically.
			 * @param toAddress specify which address we want to send.
			 * @param amount specify amount we want to send.
			 * @param assetID specify asset ID
			 * @param memo input memo attribute for describing.
			 * @param remark is used to record message of local wallet.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					const std::string &assetID,
					const std::string &memo,
					const std::string &remark) = 0;

			/**
			 * Create a withdraw transaction and return the content of transaction in json format. Note that \p amount should greater than sum of \p so that we will leave enough fee for mainchain.
			 * @param fromAddress specify which address we want to spend, or just input empty string to let wallet choose UTXOs automatically.
			 * @param amount specify amount we want to send.
			 * @param mainchainAccounts a list of mainchain accounts in json format.
			 * @param mainchainAmounts a list of mainchain amounts in json format, each amount should correspond to \p mainchainAccounts by order.
			 * @param mainchainIndexs a list of mainchain indices in json format, each index should correspond to \p mainchainAccounts by order.
			 * @param memo input memo attribute for describing.
			 * @param remark is used to record message of local wallet.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateWithdrawTransaction(
					const std::string &fromAddress,
					uint64_t amount,
					const std::string &mainChainAddress,
					const std::string &memo,
					const std::string &remark) = 0;

			/**
			 * Get genesis address of the side chain, the address is a special address will be set to toAddress in CreateDepositTransaction.
			 * @return genesis address of the side chain.
			 */
			virtual std::string GetGenesisAddress() const = 0;

			/**
			 * Get all visible assets in json format. Note this is a sub set of supported assets.
			 * @return assets list in json format
			 */
			virtual nlohmann::json GetAllVisibleAssets() const = 0;

			/**
			 * Set visible assets by specify a json that contains a list of asset ID.
			 * @param assets a json contains a list of asset ID
			 */
			virtual void SetVisibleAssets(const nlohmann::json &assets) = 0;

			/**
			 * Get all supported assets in json format
			 * @return assets list in json format
			 */
			virtual nlohmann::json GetAllSupportedAssets() const = 0;

		};

	}
}

#endif //__ELASTOS_SDK_ISIDECHAINSUBWALLET_H__
