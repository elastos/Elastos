// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONPEERLIST_H__
#define __ELASTOS_SDK_TRANSACTIONPEERLIST_H__

#include "Peer.h"

namespace Elastos {
	namespace ElaWallet {

		class TransactionPeerList {
		public:
			TransactionPeerList(const uint256 &txHash, const std::vector<PeerPtr> &peers);

			const uint256 &GetTransactionHash() const;

			const std::vector<PeerPtr> &GetPeers() const;

			void AddPeer(const PeerPtr &peer);

			bool RemovePeer(const PeerPtr &peer);

		private:
			uint256 _txHash;
			std::vector<PeerPtr> _peers;
		};

	}
}


#endif //__ELASTOS_SDK_TRANSACTIONPEERLIST_H__
