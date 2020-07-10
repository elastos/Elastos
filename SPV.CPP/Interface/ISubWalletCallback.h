/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ELASTOS_SDK_ISUBWALLETCALLBACK_H__
#define __ELASTOS_SDK_ISUBWALLETCALLBACK_H__

#include <string>

#include "nlohmann/json.hpp"

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
			 * @param progressInfo progress info contain detail as below:
			 * {
			 *     "Progress": 50,                    # 0% ~ 100%
			 *     "BytesPerSecond": 12345678,        # 12.345678 MByte / s
			 *     "LastBlockTime": 1573799697,       # timestamp of last block
			 *     "DownloadPeer": "127.0.0.1"        # IP address of node
			 * }
			 */
			virtual void OnBlockSyncProgress(const nlohmann::json &progressInfo) = 0;

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

			virtual void OnETHSCEventHandled(const nlohmann::json &event) = 0;

		};

	}
}

#endif //__ELASTOS_SDK_ISUBWALLETCALLBACK_H__
