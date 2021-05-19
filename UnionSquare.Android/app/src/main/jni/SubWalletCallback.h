// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ELA_WALLET_ANDROID_SUBWALLETCALLBACK_H
#define ELA_WALLET_ANDROID_SUBWALLETCALLBACK_H

#include "ISubWalletCallback.h"
#include "Utils.h"
#include "ISubWallet.h"
#include "nlohmann/json.hpp"

namespace Elastos {
    namespace ElaWallet {

        class SubWalletCallback : public ISubWalletCallback {
        public:
            SubWalletCallback(JNIEnv *env, jobject jobj);

        public:
            virtual void OnTransactionStatusChanged(
                    const std::string &txid,
                    const std::string &status,
                    const nlohmann::json &desc,
                    uint32_t confirms);

            /**
             * Callback method fired when best block chain height increased. This callback could be used to show progress.
             * @param progressInfo progress info contain detail as below:
             * {
             *     "Progress": 50,                    # 0% ~ 100%
             *     "BytesPerSecond": 12345678,        # 12.345678 MBytes / s
             *     "LastBlockTime": 1573799697,       # timestamp of last block
             *     "DownloadPeer": "127.0.0.1"        # IP address of node
             * }
             */
            virtual void OnBlockSyncProgress(const nlohmann::json &progressInfo);

            virtual void OnBalanceChanged(const std::string &asset, const std::string &balance);

            virtual void OnTxPublished(const std::string &hash, const nlohmann::json &result);

            virtual void OnAssetRegistered(const std::string &asset, const nlohmann::json &info);

            virtual void OnConnectStatusChanged(const std::string &status);

            virtual void OnETHSCEventHandled(const nlohmann::json &event);
			/**
			 * @param id request id
			 * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
			 * {
			 *   "id": 0,
			 *   "result": "0x1dfd14000" // 8049999872 Wei
			 * }
			 */
			virtual nlohmann::json GasPrice(int id);

			/**
			 * @param from
			 * @param to
			 * @param amount
			 * @param gasPrice
			 * @param data
			 * @param id request id
			 * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
			 * {
			 *   "id": 0,
			 *   "result": "0x5208" // 21000
			 * }
			 */
			virtual nlohmann::json EstimateGas(const std::string &from,
											   const std::string &to,
											   const std::string &amount,
											   const std::string &gasPrice,
											   const std::string &data,
											   int id);

			/**
			 * @param address
			 * @param id
			 * @return
			 * {
			 *   "id": 0,
			 *   "result": "0x0234c8a3397aab58" // 158972490234375000
			 * }
			 */
			virtual nlohmann::json GetBalance(const std::string &address, int id);

			/**
			 * @param tx Signed raw transaction.
			 * @param id
			 * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
			 * {
			 *   "id": 0,
			 *   "result": "0xe670ec64341771606e55d6b4ca35a1a6b75ee3d5145a99d05921026d1527331"
			 * }
			 */
			virtual nlohmann::json SubmitTransaction(const std::string &tx, int id);

			/**
			 * @param begBlockNumber
			 * @param endBlockNumber
			 * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
			 * {
			 *   "id": 0,
			 *   "result": [{
			 *     "hash":"0x88df016429689c079f3b2f6ad39fa052532c56795b733da78a91ebe6a713944b",
			 *     "from":"0xa7d9ddbe1f17865597fbd27ec712455208b6b76d",
			 *     "to":"0xf02c1c8e6114b1dbe8937a39260b5b0a374432bb",
			 *     "contract":"0xb60e8dd61c5d32be8058bb8eb970870f07233155", // or "", if none was created
			 *     "amount":"0xf3dbb76162000", // 4290000000000000
			 *     "gasLimit":"0x1388",
			 *     "gasPrice":"0x4a817c800", // 20000000000
			 *     "data":"0x68656c6c6f21", // input
			 *     "nonce":"0x15", // 21
			 *     "gasUsed":"0xc350", // 50000
			 *     "blockNumber":"0x5daf3b", // 6139707
			 *     "blockHash":"0x1d59ff54b1eb26b013ce3cb5fc9dab3705b415a67127a003c3e61eb445bb8df2",
			 *     "blockConfirmations":"0x100", // 256
			 *     "blockTransactionIndex":"0x41", // 65
			 *     "blockTimestamp":"0x55ba467c",
			 *     "isError": "0"
			 *   },
			 *   {
			 *     ...
			 *   }]
			 * }
			 */
			virtual nlohmann::json GetTransactions(const std::string &address,
												   uint64_t begBlockNumber,
												   uint64_t endBlockNumber,
												   int id);

			/**
			 * @param contract
			 * @param address
			 * @param event
			 * @param begBlockNumber
			 * @param endBlockNumber
			 * @param id
			 * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
			 * {
			 *   "id": 0,
			 *   "result": [{
			 *     "hash":"0xdf829c5a142f1fccd7d8216c5785ac562ff41e2dcfdf5785ac562ff41e2dcf",
			 *     "contract":"0xb60e8dd61c5d32be8058bb8eb970870f07233155", // or "", if none was created
			 *     "topics":["0x59ebeb90bc63057b6515673c3ecf9438e5058bca0f92585014eced636878c9a5"]
			 *     "data":"0x0000000000000000000000000000000000000000000000000000000000000000",
			 *     "gasPrice":"0x4a817c800", // 20000000000
			 *     "gasUsed":"0x4dc", // 1244
			 *     "logIndex":"0x1", // 1
			 *     "blockNumber":"0x1b4", // 436
			 *     "blockTransactionIndex":"0x0", // 0
			 *     "blockTimestamp":"0x55ba467c",
			 *   },{
			 *    ...
			 *   }]
			 * }
			 */
			virtual nlohmann::json GetLogs(const std::string &contract,
										   const std::string &address,
										   const std::string &event,
										   uint64_t begBlockNumber,
										   uint64_t endBlockNumber,
										   int id);

			/**
			 * @param id
			 * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
			 * [{
			 *   "id":0,
			 *   "address": "0x407d73d8a49eeb85d32cf465507dd71d507100c1",
			 *   "symbol": "ELA",
			 *   "name": "elastos",
			 *   "description": "desc",
			 *   "decimals": 18,
			 *   "defaultGasLimit": "0x1388",
			 *   "defaultGasPrice": "0x1dfd14000" // 8049999872 Wei
			 * },{
			 *   ...
			 * }]
			 */
			virtual nlohmann::json GetTokens(int id);

			/**
			 * @param id
			 * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
			 * {
			 *   "id":0,
			 *   "result": "0x4b7" // 1207
			 * }
			 */
			virtual nlohmann::json GetBlockNumber(int id);

			/**
			 * @param address
			 * @param id
			 * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
			 * {
			 *   "id": 0,
			 *   "result": "0x1" // 1
			 * }
			 */
			virtual nlohmann::json GetNonce(const std::string &address, int id);

            virtual ~SubWalletCallback();

        private:
            JNIEnv *GetEnv();

        private:
            JavaVM *_jvm;
            jobject _obj;
        };

    }
}

#endif //ELA_WALLET_ANDROID_SUBWALLETCALLBACK_H
