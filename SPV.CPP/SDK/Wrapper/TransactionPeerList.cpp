// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionPeerList.h"

namespace Elastos {
	namespace ElaWallet {

		const UInt256 &TransactionPeerList::GetTransactionHash() const {
			return _txHash;
		}

		const std::vector<PeerPtr> &TransactionPeerList::GetPeers() const {
			return _peers;
		}
	}
}
