// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>
#include <Core/BRMerkleBlock.h>

#include "SDK/P2P/PeerManager.h"
#include "BRArray.h"
#include "BRMerkleBlock.h"

#include "SDK/P2P/Peer.h"
#include "MerkleBlockMessage.h"
#include "Log.h"
#include "Utils.h"
#include "SDK/Plugin/Block/AuxPow.h"
#include "SDK/Plugin/Block/ELAMerkleBlock.h"
#include "Plugin/Registry.h"
#include "SDK/Plugin/Block/MerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		MerkleBlockMessage::MerkleBlockMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool MerkleBlockMessage::Accept(const CMBlock &msg) {
			ByteStream stream(msg);

			PeerManager *manager = _peer->getPeerManager();
			MerkleBlockPtr block(Registry::Instance()->CreateMerkleBlock(manager->GetPluginTypes().BlockType));

			if (block == nullptr) {
				_peer->Perror("create merkle block pointer with type {} fail", manager->GetPluginTypes().BlockType);
				return false;
			}

			if (!block->Deserialize(stream)) {
				_peer->Pdebug("merkle block orignal data: {}", Utils::encodeHex(msg));
				_peer->Perror("merkle block deserialize with type {} fail", manager->GetPluginTypes().BlockType);
				return false;
			}

			if (!block->isValid((uint32_t) time(nullptr))) {
				_peer->Perror("invalid merkleblock: {}", Utils::UInt256ToString(block->getHash()));
				return false;
			} else if (!_peer->SentFilter() && !_peer->SentGetdata()) {
				_peer->Perror("got merkleblock message before loading a filter");
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
