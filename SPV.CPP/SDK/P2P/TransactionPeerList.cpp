// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionPeerList.h"

namespace Elastos {
	namespace ElaWallet {

		TransactionPeerList::TransactionPeerList(const UInt256 &txHash, const std::vector<PeerPtr> &peers) :
			_peers(peers) {
			memcpy(_txHash.u8, txHash.u8, sizeof(txHash));
		}

		const UInt256 &TransactionPeerList::GetTransactionHash() const {
			return _txHash;
		}

		const std::vector<PeerPtr> &TransactionPeerList::GetPeers() const {
			return _peers;
		}

		void TransactionPeerList::AddPeer(const PeerPtr &peer) {
			_peers.push_back(peer);
		}

		void TransactionPeerList::RemovePeerAt(size_t index) {
			_peers.erase(_peers.begin() + index);
		}
	}
}
