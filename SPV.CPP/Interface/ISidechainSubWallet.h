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

#include "ISubWallet.h"

#ifndef __ELASTOS_SDK_ISIDECHAINSUBWALLET_H__
#define __ELASTOS_SDK_ISIDECHAINSUBWALLET_H__

namespace Elastos {
	namespace ElaWallet {

		class ISidechainSubWallet : public virtual IElastosBaseSubWallet {
		public:
			/**
			 * Virtual destructor.
			 */
			virtual ~ISidechainSubWallet() noexcept {}

			/**
			 * Create a withdraw transaction and return the content of transaction in json format. Note that \p amount should greater than sum of \p so that we will leave enough fee for mainchain.
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
			 * @param amount specify amount we want to send.
			 * @param mainChainAddress mainchain address.
 			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo input memo attribute for describing.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateWithdrawTransaction(
					const nlohmann::json &inputs,
					const std::string &amount,
					const std::string &mainChainAddress,
					const std::string &fee,
					const std::string &memo) = 0;

		};

	}
}

#endif //__ELASTOS_SDK_ISIDECHAINSUBWALLET_H__
