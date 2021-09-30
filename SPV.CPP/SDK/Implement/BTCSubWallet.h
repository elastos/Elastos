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

#include "IBTCSubWallet.h"
#include "SubWallet.h"

namespace Elastos {
    namespace ElaWallet {

        class BTCSubWallet : public virtual IBTCSubWallet, public SubWallet {
        public: // implement IBTCSubWallet
            ~BTCSubWallet();

        public: // implement ISubWallet
            virtual std::string GetChainID() const;

            virtual nlohmann::json GetBasicInfo() const;

            virtual nlohmann::json GetAddresses(uint32_t index, uint32_t count, bool internal = false) const;

            virtual nlohmann::json GetPublicKeys(uint32_t index, uint32_t count, bool internal = false) const;

            virtual nlohmann::json CreateTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &outputs,
                    const std::string &fee,
                    const std::string &memo);

            virtual nlohmann::json SignTransaction(const nlohmann::json &tx, const std::string &payPassword) const;

        protected:
            friend class MasterWallet;

            BTCSubWallet(const CoinInfoPtr &info,
                         const ChainConfigPtr &config,
                         MasterWallet *parent,
                         const std::string &netType);
        };

    }
}


#endif
