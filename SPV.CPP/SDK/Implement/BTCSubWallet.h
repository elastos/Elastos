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

#ifndef __ELASTOS_SDK_BTCSUBWALLET_H__
#define __ELASTOS_SDK_BTCSUBWALLET_H__

#include <support/BRKey.h>
#include "IBTCSubWallet.h"
#include "SubWallet.h"
#include "support/BRAddress.h"

namespace Elastos {
    namespace ElaWallet {

#define DEFAULT_FEE_PER_KB (TX_FEE_PER_KB*10)                  // 10 satoshis-per-byte
#define MIN_FEE_PER_KB     TX_FEE_PER_KB                       // bitcoind 0.12 default min-relay fee
#define MAX_FEE_PER_KB     ((TX_FEE_PER_KB*1000100 + 190)/191) // slightly higher than a 10,000bit fee on a 191byte tx

        class BTCSubWallet : public virtual IBTCSubWallet, public SubWallet {
        public: // implement IBTCSubWallet
            ~BTCSubWallet();

            virtual nlohmann::json GetLegacyAddresses(uint32_t index, uint32_t count, bool internal) const;

            virtual nlohmann::json CreateTransaction(const nlohmann::json &inputs, const nlohmann::json &outputs,
                                                     const std::string &changeAddress,
                                                     const std::string &feePerKB) const;

        public: // implement ISubWallet
            virtual nlohmann::json GetAddresses(uint32_t index, uint32_t count, bool internal = false) const;

            virtual nlohmann::json GetPublicKeys(uint32_t index, uint32_t count, bool internal = false) const;

            virtual nlohmann::json SignTransaction(const nlohmann::json &txJson, const std::string &passwd) const;

            virtual std::string SignDigest(const std::string &address, const std::string &digest, const std::string &passwd) const;

            virtual bool VerifyDigest(const std::string &pubkey, const std::string &digest, const std::string &signature) const;

            virtual void FlushData();

        private:
            void GetAddressInternal(std::vector<uint160> &chainAddresses, uint32_t index, uint32_t count, bool internal) const;

            uint64_t MinOutputAmountWithFeePerKb(uint64_t feePerKb) const;

            uint64_t TxFee(uint64_t feePerKb, size_t size) const;

            std::vector<HDKeychain> FindKeys(const std::vector<uint160> &pkhs, const std::string &passwd) const;

            std::vector<BRKey> FindBRKeys(const std::vector<uint160> &pkhs, const std::string &passwd) const;

        protected:
            friend class MasterWallet;

            BTCSubWallet(const CoinInfoPtr &info,
                         const ChainConfigPtr &config,
                         MasterWallet *parent,
                         const std::string &netType);

        private:
            BRAddressParams _addrParams;
            mutable std::map<uint32_t, std::vector<uint160>> _chainAddressCached;
        };

    }
}


#endif
