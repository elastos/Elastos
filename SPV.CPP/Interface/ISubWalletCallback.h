// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ISUBWALLETCALLBACK_H__
#define __ELASTOS_SDK_ISUBWALLETCALLBACK_H__

#include <string>

#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

#define SPV_API_PUBLIC  __attribute__((__visibility__("default")))

		class ISubWalletCallback {
		public:
			virtual ~ISubWalletCallback() noexcept {}

			/**
			 * Callback method fired when status of a transaction changed.
			 * @param txid indicate hash of the transaction.
			 * @param status can be "Added", "Deleted" or "Updated".
			 * @param desc is an detail description of transaction status.
			 * @param confirms is confirm count util this callback fired.
			 */
			virtual void OnTransactionStatusChanged(
					const std::string &txid,
					const std::string &status,
					const nlohmann::json &desc,
					uint32_t confirms) = 0;

			/**
			 * Callback method fired when best block chain height increased. This callback could be used to show progress.
			 * @param currentBlockHeight is the of current block when callback fired.
			 * @param estimatedHeight is max height of blockchain.
			 * @param lastBlockTime timestamp of the last block.
			 */
			virtual void OnBlockSyncProgress(uint32_t currentBlockHeight, uint32_t estimatedHeight, time_t lastBlockTime) = 0;

			/**
			 * Callback method fired when balance changed.
			 * @param asset ID.
			 * @param balance after changed.
			 */
			virtual void OnBalanceChanged(const std::string &asset, const std::string &balance) = 0;

			/**
			 * Callback method fired when tx published.
			 * @param hash of published tx.
			 * @param result in json format.
			 */
			virtual void OnTxPublished(const std::string &hash, const nlohmann::json &result) = 0;

			/**
			 * Callback method fired when a new asset registered.
			 * @param asset ID.
			 * @param information of asset.
			 */
			virtual void OnAssetRegistered(const std::string &asset, const nlohmann::json &info) = 0;

			/**
			 * Callback method fired when status of connection changed.
			 * @param status value can be one of below: "Connecting", "Connected", "Disconnected"
			 */
			virtual void OnConnectStatusChanged(const std::string &status) = 0;

		};

	}
}

#endif //__ELASTOS_SDK_ISUBWALLETCALLBACK_H__
