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
#include "Common/ErrorChecker.h"
#include "WalletCore/WordLists/English.h"

namespace Elastos {
	namespace ElaWallet {

		EthereumClient::EthereumClient(EthereumEWM::Client *client,
									   const EthereumNetworkPtr &network,
									   const std::string &storagePath,
									   const bytes_t &pubkey) :
			_client(client),
			_network(network),
			_storagePath(storagePath) {

			ErrorChecker::CheckParam(pubkey[0] != 4 || pubkey.size() != 65, Error::InvalidArgument,
									 "pubkey should be 65 bytes and begin with 0x04");

			_ewm = EthereumEWMPtr(
				new EthereumEWM(this, CRYPTO_SYNC_MODE_P2P_ONLY, _network, _storagePath, pubkey, 0, 6));
		}

		void EthereumClient::getGasPrice(BREthereumWallet wid, int rid) {
			_client->getGasPrice(wid, rid);
		}

		void EthereumClient::getGasEstimate(BREthereumWallet wid,
											BREthereumCookie cookie,
											const std::string &from,
											const std::string &to,
											const std::string &amount,
											const std::string &gasPrice,
											const std::string &data,
											int rid) {
			_client->getGasEstimate(wid, cookie, from, to, amount, gasPrice, data, rid);
		}

		void EthereumClient::getBalance(BREthereumWallet wid, const std::string &address, int rid) {
			_client->getBalance(wid, address, rid);
		}

		void EthereumClient::submitTransaction(BREthereumWallet wid,
											   BREthereumTransfer tid,
											   const std::string &rawTransaction,
											   int rid) {
			_client->submitTransaction(wid, tid, rawTransaction, rid);
		}

		void EthereumClient::getTransactions(const std::string &address,
											 uint64_t begBlockNumber,
											 uint64_t endBlockNumber,
											 int rid) {
			_client->getTransactions(address, begBlockNumber, endBlockNumber, rid);
		}

		void EthereumClient::getLogs(const std::string &contract,
									 const std::string &address,
									 const std::string &event,
									 uint64_t begBlockNumber,
									 uint64_t endBlockNumber,
									 int rid) {
			_client->getLogs(contract, address, event, begBlockNumber, endBlockNumber, rid);
		}

		void EthereumClient::getBlocks(const std::string &address,
									   int interests,
									   uint64_t blockNumberStart,
									   uint64_t blockNumberStop,
									   int rid) {
			_client->getBlocks(address, interests, blockNumberStart, blockNumberStop, rid);
		}

		void EthereumClient::getTokens(int rid) {
			_client->getTokens(rid);
		}

		void EthereumClient::getBlockNumber(int rid) {
			_client->getBlockNumber(rid);
		}

		void EthereumClient::getNonce(const std::string &address, int rid) {
			_client->getNonce(address, rid);
		}

		void EthereumClient::handleEWMEvent(const BREthereumEWMEvent &event) {
			_client->handleEWMEvent(event);
		}

		void EthereumClient::handlePeerEvent(const BREthereumPeerEvent &event) {
			_client->handlePeerEvent(event);
		}

		void EthereumClient::handleWalletEvent(const EthereumWalletPtr &wallet,
											   const BREthereumWalletEvent &event) {
			_client->handleWalletEvent(wallet, event);
		}

		void EthereumClient::handleTokenEvent(const EthereumTokenPtr &token, const BREthereumTokenEvent &event) {
			_client->handleTokenEvent(token, event);
		}

		void EthereumClient::handleTransferEvent(const EthereumWalletPtr &wallet,
												 const EthereumTransferPtr &transaction,
												 const BREthereumTransferEvent &event) {
			_client->handleTransferEvent(wallet, transaction, event);
		}

	}
}