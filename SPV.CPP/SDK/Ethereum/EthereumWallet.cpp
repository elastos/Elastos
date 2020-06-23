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

#include "EthereumWallet.h"
#include "EthereumEWM.h"

namespace Elastos {
	namespace ElaWallet {

		EthereumWallet::EthereumWallet(EthereumEWM *ewm,
									   BREthereumWallet wallet,
									   const EthereumAccountPtr &account,
									   const EthereumNetworkPtr &network) :
			ReferenceWithDefaultUnit(ewm, wallet, EthereumAmount::Unit::ETHER_ETHER),
			_account(account),
			_network(network),
			_token(nullptr) {

		}

		EthereumWallet::EthereumWallet(EthereumEWM *ewm,
									   BREthereumWallet wallet,
									   const EthereumAccountPtr &account,
									   const EthereumNetworkPtr &network,
									   const EthereumTokenPtr &token) :
			EthereumWallet(ewm, wallet, account, network) {
			_token = token;
			_defaultUnit = EthereumAmount::Unit::TOKEN_DECIMAL;
			_defaultUnitUsesToken = true;
		}

		EthereumWallet::~EthereumWallet() {

		}

		BREthereumWallet EthereumWallet::getRaw() const {
			return (BREthereumWallet) _identifier;
		}

		// Account
		EthereumAccountPtr EthereumWallet::getAccount() const {
			return _account;
		}

		// Network
		EthereumNetworkPtr EthereumWallet::getNetwork() const {
			return _network;
		}

		// Token
		EthereumTokenPtr EthereumWallet::getToken() const {
			return _token;
		}

		bool EthereumWallet::walletHoldsEther() const {
			return nullptr == _token;
		}

		std::string EthereumWallet::getSymbol() const {
			return nullptr == _token ? "ETH" : _token->getSymbol();
		}

		// Default Gas Price (ETH in WEI)
		uint64_t EthereumWallet::getDefaultGasPrice() const {
			BREthereumGasPrice price = ewmWalletGetDefaultGasPrice(_ewm->getRaw(), getRaw());
			return price.etherPerGas.valueInWEI.u64[0];
		}

		void EthereumWallet::setDefaultGasPrice(uint64_t gasPrice) {
			BREthereumGasPrice price = gasPriceCreate(etherCreateNumber(gasPrice, WEI));

			ewmWalletSetDefaultGasPrice(_ewm->getRaw(), getRaw(), price);
		}

		// Default Gas Limit (in 'gas')
		uint64_t EthereumWallet::getDefaultGasLimit() const {
			BREthereumGas limit = ewmWalletGetDefaultGasLimit(_ewm->getRaw(), getRaw());
			return limit.amountOfGas;
		}

		void EthereumWallet::setDefaultGasLimit(uint64_t gasLimit) {
			BREthereumGas limit = gasCreate(gasLimit);
			ewmWalletSetDefaultGasLimit(_ewm->getRaw(), getRaw(), limit);
		}

		// Balance
		std::string EthereumWallet::getBalance() const {
			return getBalance(_defaultUnit);
		}

		std::string EthereumWallet::getBalance(EthereumAmount::Unit unit) const {
			validUnitOrException(unit);
			BREthereumAmount balance = ewmWalletGetBalance(_ewm->getRaw(), getRaw());
			char *number = (AMOUNT_ETHER == amountGetType(balance)
							? etherGetValueString(balance.u.ether, (BREthereumEtherUnit) unit)
							: tokenQuantityGetValueString(balance.u.tokenQuantity, (BREthereumTokenQuantityUnit) unit));

			return GetCString(number);
		}

		// Estimate GasPrice and Gas
		void EthereumWallet::estimateGasPrice() {
			ewmUpdateGasPrice(_ewm->getRaw(), getRaw());
		}

		void EthereumWallet::estimateGas(const EthereumTransferPtr &transaction) {
			ewmUpdateGasEstimate(_ewm->getRaw(), getRaw(), transaction->getRaw());
		}

		// Transactions
		std::string EthereumWallet::transferEstimatedFee(const std::string &amount, EthereumAmount::Unit amountUnit,
														 EthereumAmount::Unit resultUnit) const {
			BREthereumEWM node = _ewm->getRaw();
			BREthereumWallet wallet = getRaw();
			int overflow;
			BRCoreParseStatus status;


			// Get the `amount` as ETHER or TOKEN QUANTITY
			BREthereumToken token = ewmWalletGetToken(node, wallet);
			BREthereumAmount a = (NULL == token
								  ? ewmCreateEtherAmountString(node, amount.data(), (BREthereumEtherUnit) amountUnit,
															   &status)
								  : ewmCreateTokenAmountString(node, token, amount.data(),
															   (BREthereumTokenQuantityUnit) amountUnit, &status));

			// Get the estimated FEE
			BREthereumEther fee = ewmWalletEstimateTransferFee(node, wallet, a, &overflow);

			std::string result;
			// Return the FEE in `resultUnit`
			if (status == CORE_PARSE_OK && 0 == overflow) {
				char *str = ewmCoerceEtherAmountToString(node, fee, (BREthereumEtherUnit) resultUnit);
				result = GetCString(str);
			}

			return result;
		}

		std::string EthereumWallet::transferEstimatedFee(const std::string &amount) const {
			return transferEstimatedFee(amount, _defaultUnit, _defaultUnit);
		}

		EthereumTransferPtr EthereumWallet::createTransfer(const std::string &targetAddress, const std::string &amount,
														   EthereumAmount::Unit amountUnit) const {
			BREthereumTransfer transfer = createRawTransaction(targetAddress, amount, amountUnit);
			return EthereumTransferPtr(new EthereumTransfer(_ewm, transfer, amountUnit));
		}

		EthereumTransferPtr EthereumWallet::createTransferGeneric(const std::string &targetAddress,
																  const std::string &amount,
																  EthereumAmount::Unit amountUnit,
																  const std::string &gasPrice,
																  EthereumAmount::Unit gasPriceUnit,
																  const std::string &gasLimit,
																  const std::string &data) const {
			_ewm->ensureValidAddress(targetAddress);
			assert(!EthereumAmount::isTokenUnit(amountUnit) && !EthereumAmount::isTokenUnit(gasPriceUnit));

			BREthereumTransfer transfer = createRawTransactionGeneric(targetAddress, amount,
																	  amountUnit, gasPrice,
																	  gasPriceUnit, gasLimit,
																	  data);

			return EthereumTransferPtr(new EthereumTransfer(_ewm, transfer, amountUnit));
		}

		void EthereumWallet::sign(EthereumTransferPtr &transaction, const std::string &paperKey) const {
			signRawTransaction(transaction->getRaw(), paperKey);
		}

		void EthereumWallet::signWithPrivateKey(EthereumTransferPtr &transaction, const BRKey &key) const {
			signRawTransactionWithPrivateKey(transaction->getRaw(), key);
		}

		void EthereumWallet::submit(const EthereumTransferPtr &transaction) {
			submitRawTransaction(transaction->getRaw());
		}

		std::vector<EthereumTransferPtr> EthereumWallet::getTransfers() const {
			return getRawTransactions();
		}

		BREthereumTransfer EthereumWallet::createRawTransaction(const std::string &targetAddress,
																const std::string &amount,
																EthereumAmount::Unit unit) const {
			BREthereumEWM node = _ewm->getRaw();
			BREthereumWallet wallet = getRaw();
			BREthereumToken token = ewmWalletGetToken(node, wallet);

			// Get an actual Amount
			BRCoreParseStatus status = CORE_PARSE_OK;
			BREthereumAmount a = NULL == token
								 ? amountCreateEtherString(amount.data(), (BREthereumEtherUnit) unit, &status)
								 : amountCreateTokenQuantityString(token, amount.data(),
																   (BREthereumTokenQuantityUnit) unit, &status);
			return ewmWalletCreateTransfer(node, wallet, targetAddress.data(), a);
		}

		BREthereumTransfer EthereumWallet::createRawTransactionGeneric(const std::string &to,
																	   const std::string &amount,
																	   EthereumAmount::Unit amountUnit,
																	   const std::string &gasPrice,
																	   EthereumAmount::Unit gasPriceUnit,
																	   const std::string &gasLimit,
																	   const std::string &data) const {
			BREthereumEWM node = _ewm->getRaw();
			BRCoreParseStatus status = CORE_PARSE_OK;

			// Get an actual Amount
			BREthereumEther brAmount = etherCreateString(amount.data(), (BREthereumEtherUnit) amountUnit, &status);

			BREthereumGasPrice brGasPrice = gasPriceCreate(
				etherCreateString(gasPrice.data(), (BREthereumEtherUnit) gasPriceUnit, &status));

			BREthereumGas brGasLimit = gasCreate(strtoull(gasLimit.data(), NULL, 0));

			return ewmWalletCreateTransferGeneric(node, getRaw(), to.data(), brAmount,
												  brGasPrice, brGasLimit, data.data());
		}

		void EthereumWallet::signRawTransaction(BREthereumTransfer transaction, const std::string &paperKey) const {
			BREthereumEWM node = _ewm->getRaw();

			ewmWalletSignTransferWithPaperKey(node, getRaw(), (BREthereumTransfer) transaction, paperKey.data());
		}

		void EthereumWallet::signRawTransactionWithPrivateKey(BREthereumTransfer transaction,
															  const BRKey &key) const {
			BREthereumEWM node = _ewm->getRaw();

			ewmWalletSignTransfer(node, getRaw(), (BREthereumTransfer) transaction, key);
		}

		void EthereumWallet::submitRawTransaction(BREthereumTransfer transaction) const {
			ewmWalletSubmitTransfer(_ewm->getRaw(), getRaw(), transaction);
		}

		std::vector<EthereumTransferPtr> EthereumWallet::getRawTransactions() const {
			std::vector<EthereumTransferPtr> transactions;
			int count = ewmWalletGetTransferCount(_ewm->getRaw(), getRaw());
			assert (-1 != count);
			// uint32_t array - need a long
			BREthereumTransfer *transactionIds = ewmWalletGetTransfers(_ewm->getRaw(), getRaw());
			for (int i = 0; i < count; i++) {
				EthereumTransferPtr t(new EthereumTransfer(_ewm, transactionIds[i], _defaultUnit));
				transactions.push_back(t);
			}

			return transactions;
		}

	}
}