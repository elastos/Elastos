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

#ifndef __ELASTOS_SDK_IELASTOSBASESUBWALLET_H__
#define __ELASTOS_SDK_IELASTOSBASESUBWALLET_H__

namespace Elastos {
    namespace ElaWallet {

        class IElastosBaseSubWallet : public virtual ISubWallet {
        public:
            /**
             * Create a normal transaction and return the content of transaction in json format.
             * @param inputs UTXO which will be used. eg
             * [
             *   {
             *     "TxHash": "...", // string
             *     "Index": 123, // int
             *     "Address": "...", // string
             *     "Amount": "100000000" // bigint string in SELA
             *   },
             *   ...
             * ]
             * @param outputs Outputs which we want to send to. If there is change, a new output will be append. eg
             * [
             *   {
             *     "Address": "...", // string
             *     "Amount": "100000000" // bigint string in SELA
             *   },
             *   ...
             * ]
             * @param fee Fee amount. Bigint string in SELA
             * @param memo input memo attribute for describing.
             * @return If success return the content of transaction in json format.
             */
            virtual nlohmann::json CreateTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &outputs,
                    const std::string &fee,
                    const std::string &memo) = 0;

            /**
             * Get signers already signed specified transaction.
             * @param tx a signed transaction to find signed signers.
             * @return Signed signers in json format. An example of result will be displayed as follows:
             *
             * [{"M":3,"N":4,"SignType":"MultiSign","Signers":["02753416fc7c1fb43c91e29622e378cd16243b53577ec971c6c3624a775722491a","0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280","030f5bdbee5e62f035f19153c5c32966e0fc72e419c2b4867ba533c43340c86b78"]}]
             * or
             * [{"SignType":"Standard","Signers":["0207d8bc14c4bdd79ea4a30818455f705bcc9e17a4b843a5f8f4a95aa21fb03d77"]},{"SignType":"Standard","Signers":["02a58d1c4e4993572caf0133ece4486533261e0e44fb9054b1ea7a19842c35300e"]}]
             *
             */
            virtual nlohmann::json GetTransactionSignedInfo(
                    const nlohmann::json &tx) const = 0;

            /**
             * Convert tx to raw transaction.
             * @param tx transaction json
             * @return  tx in hex string format.
             */
            virtual std::string ConvertToRawTransaction(const nlohmann::json &tx)  = 0;

        };

    }
}

#endif
