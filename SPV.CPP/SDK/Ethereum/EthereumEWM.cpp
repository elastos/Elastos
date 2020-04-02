/*
 * EthereumEWM
 *
 * Created by Ed Gamble <ed@breadwallet.com> on 3/7/18.
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

#include "EthereumEWM.h"
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		// Client Announcers
		void EthereumEWM::announceBalance(BREthereumWallet wid, const std::string &balance, int rid) {
			ewmAnnounceWalletBalance(_ewm, wid, balance.c_str(), rid);
		}

		void EthereumEWM::announceGasPrice(BREthereumWallet wid, const std::string &gasPrice, int rid) {
			ewmAnnounceGasPrice(_ewm, wid, gasPrice.c_str(), rid);
		}

		void
		EthereumEWM::announceGasEstimate(BREthereumWallet wid, BREthereumTransfer tid, const std::string &gasEstimate,
										 int rid) {
			ewmAnnounceGasEstimate(_ewm, wid, tid, gasEstimate.c_str(), rid);
		}

		void
		EthereumEWM::announceSubmitTransaction(BREthereumWallet wid, BREthereumTransfer tid, const std::string &hash,
											   int errorCode,
											   const std::string &errorMessage, int rid) {
			ewmAnnounceSubmitTransfer(_ewm, wid, tid, hash.c_str(), errorCode, errorMessage.c_str(), rid);
		}

		void EthereumEWM::announceTransaction(int id,
											  const std::string &hash,
											  const std::string &from,
											  const std::string &to,
											  const std::string &contract,
											  const std::string &amount, // value
											  const std::string &gasLimit,
											  const std::string &gasPrice,
											  const std::string &data,
											  const std::string &nonce,
											  const std::string &gasUsed,
											  const std::string &blockNumber,
											  const std::string &blockHash,
											  const std::string &blockConfirmations,
											  const std::string &blockTransactionIndex,
											  const std::string &blockTimestamp,
			// cumulative gas used,
			// confirmations
			// txreceipt_status
											  const std::string &isError) {
			ewmAnnounceTransaction(_ewm, id, hash.c_str(), from.c_str(), to.c_str(), contract.c_str(), amount.c_str(),
								   gasLimit.c_str(), gasPrice.c_str(), data.c_str(), nonce.c_str(), gasUsed.c_str(),
								   blockNumber.c_str(), blockHash.c_str(), blockConfirmations.c_str(),
								   blockTransactionIndex.c_str(), blockTimestamp.c_str(), isError.c_str());
		}

		void EthereumEWM::announceTransactionComplete(int id, bool success) {
			ewmAnnounceTransactionComplete(_ewm, id, AS_ETHEREUM_BOOLEAN(success));
		}

		void EthereumEWM::announceLog(int id,
									  const std::string &hash,
									  const std::string &contract,
									  const std::vector<std::string> &topics,
									  const std::string &data,
									  const std::string &gasPrice,
									  const std::string &gasUsed,
									  const std::string &logIndex,
									  const std::string &blockNumber,
									  const std::string &blockTransactionIndex,
									  const std::string &blockTimestamp) {
			size_t topicsCount = topics.size();
			const char *ctopics[topicsCount];
			for (size_t i = 0; i < topicsCount; ++i)
				ctopics[i] = topics[i].data();

			ewmAnnounceLog(_ewm, id, hash.data(), contract.data(), topics.size(), ctopics, data.data(),
						   gasPrice.data(), gasUsed.data(), logIndex.data(), blockNumber.data(),
						   blockTransactionIndex.data(), blockTimestamp.data());
		}

		void EthereumEWM::announceLogComplete(int id, bool success) {
			ewmAnnounceLogComplete(_ewm, id, AS_ETHEREUM_BOOLEAN(success));
		}

		void EthereumEWM::announceBlockNumber(const std::string &blockNumber, int rid) {
			ewmAnnounceBlockNumber(_ewm, blockNumber.data(), rid);
		}

		void EthereumEWM::announceNonce(const std::string &address, const std::string &nonce, int rid) {
			ewmAnnounceNonce(_ewm, address.data(), nonce.data(), rid);
		}

		void EthereumEWM::announceToken(const std::string &address,
										const std::string &symbol,
										const std::string &name,
										const std::string &description,
										int decimals,
										const std::string &defaultGasLimit,
										const std::string &defaultGasPrice,
										int rid) {
			ewmAnnounceToken(_ewm, address.data(), symbol.data(), name.data(), description.data(), decimals,
							 defaultGasLimit.data(), defaultGasPrice.data(), rid);
		}

		void EthereumEWM::announceTokenComplete(int rid, bool success) {
			ewmAnnounceTokenComplete(_ewm, AS_ETHEREUM_BOOLEAN(success), rid);
		}

		BREthereumEWM EthereumEWM::getRaw() const {
			return _ewm;
		}

	}
}