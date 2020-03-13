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

		class ISidechainSubWallet : public virtual ISubWallet {
		public:
			/**
			 * Virtual destructor.
			 */
			virtual ~ISidechainSubWallet() noexcept {}

			/**
			 * Create a withdraw transaction and return the content of transaction in json format. Note that \p amount should greater than sum of \p so that we will leave enough fee for mainchain.
			 * @param fromAddress specify which address we want to spend, or just input empty string to let wallet choose UTXOs automatically.
			 * @param amount specify amount we want to send.
			 * @param mainChainAddress mainchain address.
			 * @param memo input memo attribute for describing.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateWithdrawTransaction(
					const std::string &fromAddress,
					const std::string &amount,
					const std::string &mainChainAddress,
					const std::string &memo) = 0;

			/**
			 * Get genesis address of the side chain, the address is a special address will be set to toAddress in CreateDepositTransaction.
			 * @return genesis address of the side chain.
			 */
			virtual std::string GetGenesisAddress() const = 0;

		};

	}
}

#endif //__ELASTOS_SDK_ISIDECHAINSUBWALLET_H__
