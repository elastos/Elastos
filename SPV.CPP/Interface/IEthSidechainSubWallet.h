/*
 * Copyright (c) 2020 Elastos Foundation
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

#ifndef __ELASTOS_SDK_IETHSIDECHAINSUBWALLET_H__
#define __ELASTOS_SDK_IETHSIDECHAINSUBWALLET_H__

namespace Elastos {
	namespace ElaWallet {

		enum EthereumAmountUnit {
			TOKEN_DECIMAL = 0,
			TOKEN_INTEGER = 1,

			ETHER_WEI = 0,
			ETHER_GWEI = 3,
			ETHER_ETHER = 6,
		};

		class IEthSidechainSubWallet : public virtual ISubWallet {
		public:
			virtual ~IEthSidechainSubWallet() noexcept {}

			/**
			 *
			 * @param targetAddress
			 * @param amount
			 * @param amountUnit
			 * @return
			 */
			virtual nlohmann::json CreateTransfer(const std::string &targetAddress,
												  const std::string &amount,
												  EthereumAmountUnit amountUnit) const = 0;

			/**
			 *
			 * @param targetAddress
			 * @param amount
			 * @param amountUnit
			 * @param gasPrice
			 * @param gasPriceUnit
			 * @param gasLimit
			 * @param data
			 * @return
			 */
			virtual nlohmann::json CreateTransferGeneric(const std::string &targetAddress,
														 const std::string &amount,
														 EthereumAmountUnit amountUnit,
														 const std::string &gasPrice,
														 EthereumAmountUnit gasPriceUnit,
														 const std::string &gasLimit,
														 const std::string &data) const = 0;

		};

	}
}

#endif
