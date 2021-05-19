// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MerkleBlockMessage.h"

#include <P2P/PeerManager.h>
#include <P2P/Peer.h>
#include <Common/Log.h>
#include <Common/Utils.h>
#include <Plugin/Block/AuxPow.h>
#include <Plugin/Block/ELAMerkleBlock.h>
#include <Plugin/Registry.h>
#include <Plugin/Block/MerkleBlock.h>

#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		MerkleBlockMessage::MerkleBlockMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool MerkleBlockMessage::Accept(const bytes_t &msg) {
			std::vector<uint256> txHashes;
			int version;
			ByteStream stream(msg);

			PeerManager *manager = _peer->GetPeerManager();
			MerkleBlockPtr block(Registry::Instance()->CreateMerkleBlock(manager->GetChainID()));

			if (block == nullptr) {
				_peer->error("create merkle block pointer with type fail");
				return false;
			}

#ifdef SUPPORT_NEW_INV_TYPE
			version = MERKLEBLOCK_VERSION_1;
#else
			version = MERKLEBLOCK_VERSION_0;
#endif

			if (!block->Deserialize(stream, version)) {
				_peer->debug("merkle block orignal data: {}", msg.getHex());
				_peer->error("merkle block deserialize with type fail");
				return false;
			}

			if (!block->IsValid((uint32_t) time(nullptr))) {
				_peer->error("invalid merkleblock: {}", block->GetHash().GetHex());
				return false;
			} else if (!_peer->SentFilter() && !_peer->SentGetdata()) {
				_peer->error("got merkleblock message before loading a filter");
				return false;
			} else {
				_peer->SetWaitingBlocks(false);
				block->MerkleBlockTxHashes(txHashes);

				for (size_t i = txHashes.size(); i > 0; i--) { // reverse order for more efficient removal as tx arrive
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
