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

#ifndef __ELASTOS_SDK_ELASTOSBASESUBWALLET_H__
#define __ELASTOS_SDK_ELASTOSBASESUBWALLET_H__

#include "SubWallet.h"
#include "IElastosBaseSubWallet.h"

namespace Elastos {
    namespace ElaWallet {

        typedef boost::shared_ptr<SpvService> WalletManagerPtr;

        class ElastosBaseSubWallet : public virtual IElastosBaseSubWallet, public SubWallet {
        public:
            const WalletManagerPtr &GetWalletManager() const;

            virtual void FlushData();

        public: //default implement ISubWallet & IElastosBaseSubWallet
            virtual nlohmann::json GetBasicInfo() const;

            virtual nlohmann::json GetAddresses(uint32_t index, uint32_t count, bool internal = false) const;

            virtual nlohmann::json GetPublicKeys(uint32_t index, uint32_t count, bool internal = false) const;

            virtual nlohmann::json CreateTransaction(
                    const nlohmann::json &inputsJson,
                    const nlohmann::json &outputsJson,
                    const std::string &fee,
                    const std::string &memo);

            virtual nlohmann::json SignTransaction(const nlohmann::json &tx, const std::string &payPassword) const;

            virtual std::string SignDigest(const std::string &address, const std::string &digest, const std::string &passwd) const;

            virtual bool VerifyDigest(const std::string &publicKey, const std::string &digest, const std::string &signature) const;

            virtual nlohmann::json GetTransactionSignedInfo(
                    const nlohmann::json &rawTransaction) const;

            virtual std::string ConvertToRawTransaction(const nlohmann::json &tx);

        protected:
            friend class MasterWallet;

            ElastosBaseSubWallet(const CoinInfoPtr &info,
                                 const ChainConfigPtr &config,
                                 MasterWallet *parent,
                                 const std::string &netType);

            void EncodeTx(nlohmann::json &result, const TransactionPtr &tx) const;

            TransactionPtr DecodeTx(const nlohmann::json &encodedTx) const;

            bool UTXOFromJson(UTXOSet &utxo, const nlohmann::json &j) const;

            bool OutputsFromJson(OutputArray &outputs, const nlohmann::json &j);

        protected:
            WalletManagerPtr _walletManager;
        };

    }
}

#endif
