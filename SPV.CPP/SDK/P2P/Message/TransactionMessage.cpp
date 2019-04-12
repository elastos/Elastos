// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionMessage.h"

#include <P2P/Peer.h>
#include <P2P/PeerManager.h>
#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/IDTransaction.h>
#include <Common/Log.h>
#include <Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		TransactionMessage::TransactionMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool TransactionMessage::Accept(const bytes_t &msg) {
			std::string chainID = _peer->GetPeerManager()->GetChainID();

			ByteStream stream(msg);

			TransactionPtr tx;
			if (chainID == CHAINID_MAINCHAIN) {
				tx = TransactionPtr(new Transaction());
			} else if (chainID == CHAINID_IDCHAIN || chainID == CHAINID_TOKENCHAIN) {
				tx = TransactionPtr(new IDTransaction());
			}

			if (!tx->Deserialize(stream)) {
				_peer->error("malformed tx message with length: {}", msg.size());
				return false;
			} else if (!_peer->SentFilter() && !_peer->SentGetdata()) {
				_peer->error("got tx message before loading filter");
				return false;
			} else {
				uint256 txHash = tx->GetHash();

				PEER_INFO(_peer, "got tx {}", txHash.GetHex());

				FireRelayedTx(tx);

				if (_peer->CurrentBlock() != nullptr) { // we're collecting tx messages for a merkleblock
					_peer->CurrentBlockTxHashesRemove(txHash);

					if (_peer->CurrentBlockTxHashes().size() == 0) { // we received the entire block including all matched tx
						const MerkleBlockPtr block = _peer->CurrentBlock();

						_peer->SetCurrentBlock(nullptr);
						FireRelayedBlock(block);
					}
				}
			}

			return true;
		}

		void TransactionMessage::Send(const SendMessageParameter &param) {
			const TransactionParameter &txParam = static_cast<const TransactionParameter &>(param);

			PEER_INFO(_peer, "sending tx {}", txParam.tx->GetHash().GetHex());

			ByteStream stream;
			txParam.tx->Serialize(stream);
			SendMessage(stream.GetBytes(), Type());
		}

		std::string TransactionMessage::Type() const {
			return MSG_TX;
		}

	}
}
