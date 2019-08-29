// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionPeerList.h"

namespace Elastos {
	namespace ElaWallet {

		TransactionPeerList::TransactionPeerList(const uint256 &txHash, const std::vector<PeerPtr> &peers) :
			_peers(peers),
			_txHash(txHash) {
		}

		const uint256 &TransactionPeerList::GetTransactionHash() const {
			return _txHash;
		}

		const std::vector<PeerPtr> &TransactionPeerList::GetPeers() const {
			return _peers;
		}

		void TransactionPeerList::AddPeer(const PeerPtr &peer) {
			_peers.push_back(peer);
		}

		bool TransactionPeerList::RemovePeer(const PeerPtr &peer) {
			bool removed = false;
			for (std::vector<PeerPtr>::iterator p = _peers.begin(); p != _peers.end();) {
				if ((*p) == peer) {
					p = _peers.erase(p);
					removed = true;
				} else {
					++p;
				}
			}
			return removed;
		}
	}
}
