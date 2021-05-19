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
#ifndef __ELASTOS_SDK_TOKENCHAINSUBWALLET_H__
#define __ELASTOS_SDK_TOKENCHAINSUBWALLET_H__

#include "SidechainSubWallet.h"
#include <ITokenchainSubWallet.h>

namespace Elastos {
	namespace ElaWallet {

		class TokenchainSubWallet : public SidechainSubWallet, public ITokenchainSubWallet {
		public:
			virtual ~TokenchainSubWallet();

			virtual nlohmann::json GetBalanceInfo(const std::string &assetID) const;

			virtual std::string GetBalance(const std::string &assetID) const;

			virtual std::string GetBalanceWithAddress(const std::string &assetID, const std::string &address) const;

			virtual nlohmann::json CreateRegisterAssetTransaction(
				const std::string &name,
				const std::string &description,
				const std::string &registerToAddress,
				const std::string &registerAmount,
				uint8_t precision,
				const std::string &memo);

			virtual nlohmann::json CreateTransaction(
				const std::string &fromAddress,
				const std::string &toAddress,
				const std::string &amount,
				const std::string &assetID,
				const std::string &memo);

			virtual nlohmann::json CreateConsolidateTransaction(
				const std::string &assetID,
				const std::string &memo);

			virtual nlohmann::json GetAllAssets() const;

		protected:
			friend class MasterWallet;

			TokenchainSubWallet(const CoinInfoPtr &info,
							   const ChainConfigPtr &config,
							   MasterWallet *parent,
							   const std::string &netType);

		};

	}
}

#endif //__ELASTOS_SDK_TOKENCHAINSUBWALLET_H__
