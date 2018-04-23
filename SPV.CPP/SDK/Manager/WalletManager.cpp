// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "WalletManager.h"
#include "Log.h"

#define BACKGROUND_THREAD_COUNT 1

namespace Elastos {
	namespace SDK {

		WalletManager::WalletManager(const MasterPubKeyPtr &masterPubKey, const ChainParams &chainParams,
									 uint32_t earliestPeerTime, const boost::filesystem::path &path) :
			CoreWalletManager(masterPubKey, chainParams, earliestPeerTime),
			_executor(BACKGROUND_THREAD_COUNT),
			_databaseManager(path) {
		}

		WalletManager::~WalletManager() {

		}

		void WalletManager::balanceChanged(uint64_t balance) {

		}

		void WalletManager::onTxAdded(Transaction *transaction) {
			if (!SHOW_CALLBACK) {
				return;
			}

			Log::info("");

		}

		const CoreWalletManager::PeerManagerListenerPtr &WalletManager::createPeerManagerListener() {
			if (_peerManagerListener != nullptr) {
				_peerManagerListener = PeerManagerListenerPtr(new WrappedExecutorPeerManagerListener(this, &_executor));
			}
			return _peerManagerListener;
		}

		const CoreWalletManager::WalletListenerPtr &WalletManager::createWalletListener() {
			if (_walletListener == nullptr) {
				_walletListener = WalletListenerPtr(new WrappedExecutorWalletListener(this, &_executor));
			}
			return _walletListener;
		}

		std::string WalletManager::getChainDescriptiveName() const {
			return "";
		}
	}
}
