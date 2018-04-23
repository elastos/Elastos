// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_WALLETMANAGER_H__
#define __ELASTOS_SDK_WALLETMANAGER_H__

#include "CoreWalletManager.h"
#include "DatabaseManager.h"
#include "BackgroundExecutor.h"

namespace Elastos {
	namespace SDK {

		class WalletManager :
				public CoreWalletManager {
		public:
			WalletManager(const MasterPubKeyPtr &masterPubKey,
						  const ChainParams &chainParams,
						  uint32_t earliestPeerTime,
						  const boost::filesystem::path &path);

			~WalletManager();

		public:
			//todo override Wallet listener
			// func balanceChanged(_ balance: UInt64)
			virtual void balanceChanged(uint64_t balance);

			// func txAdded(_ tx: BRTxRef)
			virtual void onTxAdded(Transaction *transaction);
		public:
			//todo override PeerManager listener

		protected:
			virtual const PeerManagerListenerPtr &createPeerManagerListener();

			virtual const WalletListenerPtr &createWalletListener();

			//todo override other protected methods

		private:
			std::string getChainDescriptiveName() const;

		private:
			DatabaseManager _databaseManager;
			BackgroundExecutor _executor;
		};

	}
}

#endif //__ELASTOS_SDK_WALLETMANAGER_H__
