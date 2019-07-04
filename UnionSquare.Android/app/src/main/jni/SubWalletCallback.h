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

            virtual void OnBlockSyncStarted();

            /**
			 * Callback method fired when best block chain height increased. This callback could be used to show progress.
			 * @param currentBlockHeight is the of current block when callback fired.
			 * @param estimatedHeight is max height of blockchain.
			 * @param lastBlockTime timestamp of the last block.
			 */
            virtual void OnBlockSyncProgress(uint32_t currentBlockHeight, uint32_t estimatedHeight,
                                             time_t lastBlockTime);

            /**
             * Callback method fired when block end synchronizing with a peer. This callback could be used to show progress.
             */
            virtual void OnBlockSyncStopped();

            virtual void OnBalanceChanged(const std::string &asset, const std::string &balance);

            virtual void OnTxPublished(const std::string &hash, const nlohmann::json &result);

            virtual void OnAssetRegistered(const std::string &asset, const nlohmann::json &info);

            virtual void OnConnectStatusChanged(const std::string &status);

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
