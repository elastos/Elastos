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

#ifndef __ELASTOS_SPVSDK_CLIENT_H__
#define __ELASTOS_SPVSDK_CLIENT_H__

#include "EthereumEWM.h"

namespace Elastos {
	namespace ElaWallet {

		class EthereumClient : public EthereumEWM::Client {
		public:
			EthereumClient(EthereumEWM::Client *listener,
						   const EthereumNetworkPtr &network,
						   const std::string &storagePath,
						   const bytes_t &pubkey);
		public:
			virtual void getGasPrice(BREthereumWallet wid, int rid);

			virtual void getGasEstimate(BREthereumWallet wid,
										BREthereumCookie cookie,
										const std::string &from,
										const std::string &to,
										const std::string &amount,
										const std::string &gasPrice,
										const std::string &data,
										int rid);

			virtual void getBalance(BREthereumWallet wid, const std::string &address, int rid);

			virtual void
			submitTransaction(BREthereumWallet wid, BREthereumTransfer tid, const std::string &rawTransaction,
							  int rid);

			virtual void
			getTransactions(const std::string &address, uint64_t begBlockNumber, uint64_t endBlockNumber,
							int rid);

			virtual void getLogs(const std::string &contract,
								 const std::string &address,
								 const std::string &event,
								 uint64_t begBlockNumber,
								 uint64_t endBlockNumber,
								 int rid);

			virtual void getBlocks(const std::string &address,
								   int interests,
								   uint64_t blockNumberStart,
								   uint64_t blockNumberStop,
								   int rid);

			virtual void getTokens(int rid);

			//        typedef void (*BREthereumClientHandlerGetBlockNumber) (BREthereumClientContext context,
			//                                                    BREthereumEWM ewm,
			//                                                    int rid);
			virtual void getBlockNumber(int rid);

			//        typedef void (*BREthereumClientHandlerGetNonce) (BREthereumClientContext context,
			//                                                        BREthereumEWM ewm,
			//                                                        const char *address,
			//                                                        int rid);
			virtual void getNonce(const std::string &address, int rid);

			virtual void handleEWMEvent(const BREthereumEWMEvent &event);

			virtual void handlePeerEvent(const BREthereumPeerEvent &event);

			virtual void handleWalletEvent(const EthereumWalletPtr &wallet,
										   const BREthereumWalletEvent &event);

			virtual void handleTokenEvent(const EthereumTokenPtr &token, const BREthereumTokenEvent &event);

			virtual void handleTransferEvent(const EthereumWalletPtr &wallet,
											 const EthereumTransferPtr &transaction,
											 const BREthereumTransferEvent &event);

		protected:
			friend class EthSidechainSubWallet;
			EthereumNetworkPtr _network;
			EthereumEWMPtr _ewm;
			std::string _storagePath;
			EthereumEWM::Client *_client;
		};

	}
}

#endif
