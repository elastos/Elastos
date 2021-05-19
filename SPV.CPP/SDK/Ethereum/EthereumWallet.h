/*
 * EthereumWallet
 *
 * Created by Ed Gamble <ed@breadwallet.com> on 3/20/18.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
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

#ifndef __ELASTOS_SPVSDK_ETHEREUMWALLET_H__
#define __ELASTOS_SPVSDK_ETHEREUMWALLET_H__

#include "ReferenceWithDefaultUnit.h"
#include "EthereumAccount.h"
#include "EthereumNetwork.h"
#include "EthereumToken.h"
#include "EthereumTransfer.h"
#include <ethereum/ewm/BREthereumWallet.h>

namespace Elastos {
	namespace ElaWallet {

// Default Gas Price (ETH in WEI)
#define MAXIMUM_DEFAULT_GAS_PRICE 100000000000000L // 100 GWEI

#define GAS_PRICE_1_GWEI    1000000000000L
#define GAS_PRICE_2_GWEI    2000000000000L
#define GAS_PRICE_4_GWEI    4000000000000L
#define GAS_PRICE_10_GWEI  10000000000000L
#define GAS_PRICE_20_GWEI  20000000000000L

		class EthereumWallet : public ReferenceWithDefaultUnit {
		public:
			// Constructors
			EthereumWallet(EthereumEWM *ewm, BREthereumWallet wallet, const EthereumAccountPtr &account,
						   const EthereumNetworkPtr &network);

			EthereumWallet(EthereumEWM *ewm, BREthereumWallet wallet, const EthereumAccountPtr &account,
						   const EthereumNetworkPtr &network, const EthereumTokenPtr &token);

			~EthereumWallet() override;

		private:
			BREthereumWallet getRaw() const;

		public:
			// Account
			EthereumAccountPtr getAccount() const;

			// Network
			EthereumNetworkPtr getNetwork() const;

			// Token
			EthereumTokenPtr getToken() const;

			bool walletHoldsEther() const;

			std::string getSymbol() const;

			// Default Gas Price (ETH in WEI)
			uint64_t getDefaultGasPrice() const;

			void setDefaultGasPrice(uint64_t gasPrice);

			// Default Gas Limit (in 'gas')
			uint64_t getDefaultGasLimit() const;

			void setDefaultGasLimit(uint64_t gasLimit);

			// Balance
			std::string getBalance() const;

			std::string getBalance(EthereumAmount::Unit unit) const;

			// Estimate GasPrice and Gas
			void estimateGasPrice();

			void estimateGas(const EthereumTransferPtr &transaction);

			// Transactions
			std::string transferEstimatedFee(const std::string &amount, EthereumAmount::Unit amountUnit,
											 EthereumAmount::Unit resultUnit) const;

			std::string transferEstimatedFee(const std::string &amount) const;

			EthereumTransferPtr createTransfer(const std::string &targetAddress, const std::string &amount,
											   EthereumAmount::Unit amountUnit) const;

			EthereumTransferPtr createTransferGeneric(const std::string &targetAddress,
													  const std::string &amount, EthereumAmount::Unit amountUnit,
													  const std::string &gasPrice, EthereumAmount::Unit gasPriceUnit,
													  const std::string &gasLimit, const std::string &data) const;

			void sign(EthereumTransferPtr &transaction, const std::string &paperKey) const;

			void signWithPrivateKey(EthereumTransferPtr &transaction, const BRKey &key) const;

			void submit(const EthereumTransferPtr &transaction);

			std::vector<EthereumTransferPtr> getTransfers() const;

		private:
			BREthereumTransfer createRawTransaction(const std::string &targetAddress, const std::string &amount,
													EthereumAmount::Unit unit) const;

			BREthereumTransfer createRawTransactionGeneric(const std::string &to,
														   const std::string &amount,
														   EthereumAmount::Unit amountUnit,
														   const std::string &gasPrice,
														   EthereumAmount::Unit gasPriceUnit,
														   const std::string &gasLimit,
														   const std::string &data) const;

			void signRawTransaction(BREthereumTransfer transaction, const std::string &paperKey) const;

			void signRawTransactionWithPrivateKey(BREthereumTransfer transaction, const BRKey &key) const;

			void submitRawTransaction(BREthereumTransfer transaction) const;

			std::vector<EthereumTransferPtr> getRawTransactions() const;

		private:
			EthereumAccountPtr _account;
			EthereumNetworkPtr _network;
			EthereumTokenPtr _token;
		};

		typedef boost::shared_ptr<EthereumWallet> EthereumWalletPtr;

	}
}

#endif
