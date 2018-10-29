// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAPEERMANAGER_H__
#define __ELASTOS_SDK_ELAPEERMANAGER_H__

#include "Plugin/PluginTypes.h"
#include "BRPeerManager.h"

namespace Elastos {
	namespace ElaWallet {

		struct ELAPeerManager {
			BRPeerManager Raw;
			PluginTypes Plugins;
		};

		ELAPeerManager *ELAPeerManagerNew(const BRChainParams *params, BRWallet *wallet, uint32_t earliestKeyTime,
										  uint32_t reconnectSeconds,
										  BRMerkleBlock *blocks[], size_t blocksCount, const BRPeer peers[],
										  size_t peersCount, BRPeerMessages *peerMessages, const PluginTypes &plugins);

		void ELAPeerManagerFree(ELAPeerManager *peerManager);

	}
}

#endif //__ELASTOS_SDK_ELAPEERMANAGER_H__
