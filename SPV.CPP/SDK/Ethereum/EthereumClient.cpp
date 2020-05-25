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

#include "EthereumClient.h"
#include "Common/Log.h"
#include "WalletCore/WordLists/English.h"

namespace Elastos {
	namespace ElaWallet {

		EthereumClient::EthereumClient(const EthereumNetworkPtr &network,
									   const std::string &storagePath,
									   const std::string &payperKey) :
			_network(network),
			_storagePath(storagePath) {

			_ewm = EthereumEWMPtr(
				new EthereumEWM(this, EthereumEWM::Mode::API_ONLY, network, _storagePath, payperKey, EnglishWordLists));

		}

		void EthereumClient::getGasPrice(BREthereumWallet wid, int rid) {
			ArgInfo("{}", GetFunName());
		}

		void EthereumClient::getGasEstimate(BREthereumWallet wid,
											BREthereumTransfer tid,
											const std::string &from,
											const std::string &to,
											const std::string &amount,
											const std::string &data,
											int rid) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::getBalance(BREthereumWallet wid, const std::string &address, int rid) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::submitTransaction(BREthereumWallet wid, BREthereumTransfer tid, const std::string &rawTransaction,
											   int rid) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::getTransactions(const std::string &address, uint64_t begBlockNumber, uint64_t endBlockNumber,
											 int rid) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::getLogs(const std::string &contract,
									 const std::string &address,
									 const std::string &event,
									 uint64_t begBlockNumber,
									 uint64_t endBlockNumber,
									 int rid) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::getBlocks(const std::string &address,
									   int interests,
									   uint64_t blockNumberStart,
									   uint64_t blockNumberStop,
									   int rid) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::getTokens(int rid) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::getBlockNumber(int rid) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::getNonce(const std::string &address, int rid) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::handleEWMEvent(EthereumEWM::EWMEvent event, EthereumEWM::Status status,
											const std::string &errorDescription) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::handlePeerEvent(EthereumEWM::PeerEvent event, EthereumEWM::Status status,
											 const std::string &errorDescription) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::handleWalletEvent(const EthereumWalletPtr &wallet,
											   EthereumEWM::WalletEvent event, EthereumEWM::Status status,
											   const std::string &errorDescription) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::handleTokenEvent(const EthereumTokenPtr &token, EthereumEWM::TokenEvent event) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::handleBlockEvent(const EthereumBlockPtr &block,
											  EthereumEWM::BlockEvent event, EthereumEWM::Status status,
											  const std::string &errorDescription) {
			ArgInfo("{}", GetFunName());

		}

		void EthereumClient::handleTransferEvent(const EthereumWalletPtr &wallet,
												 const EthereumTransferPtr &transaction,
												 EthereumEWM::TransactionEvent event, EthereumEWM::Status status,
												 const std::string &errorDescription) {
			ArgInfo("{}", GetFunName());

		}

	}
}