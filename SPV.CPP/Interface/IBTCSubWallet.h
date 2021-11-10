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

#ifndef __ELASTOS_SDK_IBTCSUBWALLET_H__
#define __ELASTOS_SDK_IBTCSUBWALLET_H__

#include "ISubWallet.h"

namespace Elastos {
    namespace ElaWallet {

        class IBTCSubWallet : public virtual ISubWallet {
        public:
            /**
             * get legay addresses of btc
             * @index start from where.
             * @count how many address we need.
             * @internal change address for true or normal receive address for false.
             * @return as required
             */
            virtual nlohmann::json GetLegacyAddresses(uint32_t index, uint32_t count, bool internal) const = 0;

            /**
             * create btc transaction
             * @inputs in json array format. eg:
             * [
             * {
             *   "TxHash": "...", // uint256 string
             *   "Index": 0, // uint16_t
             *   "Address": "...", // btc address
             *   "Amount": "100000000" // bigint string in satoshi
             * },
             * {
             *   ...
             * }
             * ]
             * @outputs in json array format. eg:
             * [
             * {
             *   "Address": "...", // btc address
             *   "Amount": "100000000" // bigint string in satoshi
             * },
             * {
             *   ...
             * }
             * ]
             * @changeAddress change address in string format.
             * @feePerKB how much fee (satoshi) per kb of tx size.
             * @return unsigned serialized transaction in json format.
             */
            virtual nlohmann::json CreateTransaction(const nlohmann::json &inputs,
                                                     const nlohmann::json &outputs,
                                                     const std::string &changeAddress,
                                                     const std::string &feePerKB) const = 0;

        };

    }
}

#endif
