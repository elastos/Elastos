// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MerkleBlockMessage.h"

#include <SDK/P2P/PeerManager.h>
#include <SDK/P2P/Peer.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Plugin/Block/AuxPow.h>
#include <SDK/Plugin/Block/ELAMerkleBlock.h>
#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Block/MerkleBlock.h>

#include <Core/BRMerkleBlock.h>
#include <Core/BRArray.h>
#include <Core/BRMerkleBlock.h>

#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		MerkleBlockMessage::MerkleBlockMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool MerkleBlockMessage::Accept(const CMBlock &msg) {
			ByteStream stream(msg);

			PeerManager *manager = _peer->getPeerManager();
			MerkleBlockPtr block(Registry::Instance()->CreateMerkleBlock(manager->GetPluginType()));

			if (block == nullptr) {
				_peer->error("create merkle block pointer with type {} fail", manager->GetPluginType());
				return false;
			}

			if (!block->Deserialize(stream)) {
				_peer->debug("merkle block orignal data: {}", Utils::encodeHex(msg));
				_peer->error("merkle block deserialize with type {} fail", manager->GetPluginType());
				return false;
			}

			if (!block->isValid((uint32_t) time(nullptr))) {
				_peer->error("invalid merkleblock: {}", Utils::UInt256ToString(block->getHash()));
				return false;
			} else if (!_peer->SentFilter() && !_peer->SentGetdata()) {
				_peer->error("got merkleblock message before loading a filter");
				return false;
			} else {
				std::vector<UInt256> txHashes = block->MerkleBlockTxHashes();

				for (size_t i = txHashes.size(); i > 0; i--) { // reverse order for more efficient removal as tx arrive
					if (_peer->KnownTxHashSet().Contains(txHashes[i - 1])) continue;
					_peer->AddCurrentBlockTxHash(txHashes[i - 1]);
				}
			}

			if (_peer->CurrentBlockTxHashes().size() > 0) { // wait til we get all tx messages before processing the block
				_peer->SetCurrentBlock(block);
			} else {
				FireRelayedBlock(block);
			}

			return true;
		}

		void MerkleBlockMessage::Send(const SendMessageParameter &param) {
		}

		std::string MerkleBlockMessage::Type() const {
			return MSG_MERKLEBLOCK;
		}
	}
}
