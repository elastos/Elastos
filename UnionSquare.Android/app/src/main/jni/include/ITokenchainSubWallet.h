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
#include "ISidechainSubWallet.h"

#ifndef __ELASTOS_SDK_ITOKENCHAINSUBWALLET_H__
#define __ELASTOS_SDK_ITOKENCHAINSUBWALLET_H__

namespace Elastos {
	namespace ElaWallet {

		class ITokenchainSubWallet: public virtual ISidechainSubWallet {
		public:
			/**
			 * Get balances of all addresses in json format.
			 * @param assetID asset hex code from asset hash.
			 * @return balances of all addresses in json format.
			 */
			virtual nlohmann::json GetBalanceInfo(const std::string &assetID) const = 0;

			/**
			 * Get sum of balances of all addresses.
			 * @param assetID asset hex code from asset hash.
			 * @return sum of balances in big int string. 1 Token = "1.0000"
			 */
			virtual std::string GetBalance(const std::string &assetID) const = 0;

			/**
			 * Get balance of only the specified address.
			 * @param assetID asset hex code from asset hash.
			 * @param address is one of addresses created by current sub wallet.
			 * @return balance of specified address in big int string.
			 */
			virtual std::string GetBalanceWithAddress(const std::string &assetID, const std::string &address) const = 0;

			/**
			 * Register asset.
			 * @param name Asset name such as ELA, IDChain.
			 * @param description
			 * @param registerToAddress Address where to receive registered asset amount.
			 * @param registerAmount Integer part amount.
			 * @param precision Range: [0, 18].
			 * @param memo
			 * @return Transaction in json format.
			 */
			virtual nlohmann::json CreateRegisterAssetTransaction(
				const std::string &name,
				const std::string &description,
				const std::string &registerToAddress,
				const std::string &registerAmount,
				uint8_t precision,
				const std::string &memo) = 0;

			/**
			 * Create a normal transaction and return the content of transaction in json format.
			 * @param fromAddress specify which address we want to spend, or just input empty string to let wallet choose UTXOs automatically.
			 * @param toAddress specify which address we want to send.
			 * @param amount specify amount(big int string) we want to send.
			 * @param assetID specify asset ID
			 * @param memo input memo attribute for describing.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateTransaction(
				const std::string &fromAddress,
				const std::string &toAddress,
				const std::string &amount,
				const std::string &assetID,
				const std::string &memo) = 0;

			/**
			 * Create a transaction to combine as many UTXOs as possible until transaction size reaches the max size.
			 * @param assetID specify asset ID
			 * @param memo input memo attribute for describing.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateConsolidateTransaction(
				const std::string &assetID,
				const std::string &memo) = 0;

			/**
			 * Get all assets in json format. Note this is a sub set of supported assets.
			 * @return assets list in json format
			 */
			virtual nlohmann::json GetAllAssets() const = 0;

		};

	}
}

#endif //__ELASTOS_SDK_ITOKENCHAINSUBWALLET_H__
