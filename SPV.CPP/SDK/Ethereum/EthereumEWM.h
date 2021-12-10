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

#ifndef __ELASTOS_SDK_ETHEREUMEWM_H__
#define __ELASTOS_SDK_ETHEREUMEWM_H__

#include "EthereumNetwork.h"
#include "EthereumAccount.h"
#include "EthereumTransfer.h"
#include "EthereumAmount.h"
#include "EthereumToken.h"
#include "EthereumWallet.h"

#include <ethereum/ewm/BREthereumEWM.h>

#include <string>
#include <boost/weak_ptr.hpp>
#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

		class EthereumEWM {
		public:
			// Account
			EthereumNetworkPtr getNetwork() const;

			EthereumAccountPtr getAccount() const;

			std::string getAddress() const;

			bytes_t getAddressPublicKey() const;

			// Wallet
		protected:
			EthereumWalletPtr walletLookupOrCreate(BREthereumWallet wid, const EthereumTokenPtr &token);

		public:
			std::vector<EthereumWalletPtr> getWallets();

			EthereumWalletPtr getWallet();

			EthereumWalletPtr getWallet(const EthereumTokenPtr &token);

			EthereumWalletPtr getWalletByIdentifier(BREthereumWallet wid);

			// Transaction
		protected:
			EthereumTransferPtr transactionLookupOrCreate(BREthereumTransfer tid);

		public:
			void transferDelete(const EthereumTransferPtr &transfer);

		public:
			uint64_t getBlockHeight() const;

			// Tokens
		public:
			std::vector<EthereumTokenPtr> getTokens() const;

		protected:
			EthereumTokenPtr lookupTokenByReference(BREthereumToken reference) const;

			EthereumTokenPtr addTokenByReference(BREthereumToken reference);

		public:
			EthereumTokenPtr lookupToken(const std::string &address) const;

			// Constructor
		public:
			EthereumEWM(BRCryptoSyncMode mode, const EthereumNetworkPtr &network,
						const std::string &storagePath, const std::string &paperKey,
						const std::vector<std::string> &wordList,
						uint64_t blockHeight,
						uint64_t confirmationsUntilFinal);

			EthereumEWM(BRCryptoSyncMode mode, const EthereumNetworkPtr &network,
						const std::string &storagePath, const bytes_t &publicKey,
						uint64_t blockHeight,
						uint64_t confirmationsUntilFinal);

			~EthereumEWM();

		private:
			EthereumEWM(BREthereumEWM identifier, const EthereumNetworkPtr &network);

			BREthereumEWM createRawEWM(BRCryptoSyncMode mode, BREthereumNetwork network,
									   const std::string &storagePath, BREthereumAccount account,
									   uint64_t blockHeight,
									   uint64_t confirmationsUntilFinal);

			BREthereumEWM createRawEWM(BRCryptoSyncMode mode, BREthereumNetwork network,
									   const std::string &storagePath, const std::string &paperKey,
									   const std::vector<std::string> &wordList,
									   uint64_t blockHeight,
									   uint64_t confirmationsUntilFinal);

			BREthereumEWM createRawEWMPublicKey(BRCryptoSyncMode mode, BREthereumNetwork network,
												const std::string &storagePath, const bytes_t &pubkey,
												uint64_t blockHeight,
												uint64_t confirmationsUntilFinal);

			// Connect / Disconnect
		public:

			static bool addressIsValid(const std::string &address);

			static void ensureValidAddress(const std::string &address);

		public:
			BREthereumEWM getRaw() const;

		private:
			typedef std::map<BREthereumWallet, EthereumWalletPtr> WalletMap;
			typedef std::map<BREthereumTransfer, EthereumTransferPtr> TransferMap;
			typedef std::map<std::string, EthereumTokenPtr> TokenAddressMap;
			typedef std::map<BREthereumToken, EthereumTokenPtr> TokenReferenceMap;
			WalletMap _wallets;
			TransferMap _transactions;
			TokenAddressMap _tokensByAddress;
			TokenReferenceMap _tokensByReference;

		private:
			BREthereumEWM _ewm;
			EthereumNetworkPtr _network;
			EthereumAccountPtr _account;
		};

		typedef boost::shared_ptr<EthereumEWM> EthereumEWMPtr;

	}
}

#endif //ELASTOS_SPVSDK_ETHEREUMEWM_H
