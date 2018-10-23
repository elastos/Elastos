// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <Core/BRTransaction.h>
#include "BRArray.h"

#include "SDK/P2P/Peer.h"
#include "TransactionMessage.h"
#include "Plugin/Transaction/Transaction.h"
#include "Log.h"
#include "Utils.h"

namespace Elastos {
	namespace ElaWallet {

		TransactionMessage::TransactionMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool TransactionMessage::Accept(const CMBlock &msg) {
			ByteStream stream(msg);
			TransactionPtr tx = TransactionPtr(new Transaction());

			UInt256 txHash;

			if (!tx->Deserialize(stream)) {
				_peer->Perror("Malformed tx message with length: {}", msg.GetSize());
				return false;
			} else if (!_peer->SentFilter() && !_peer->SentGetdata()) {
				_peer->Perror("Got tx message before loading filter");
				return false;
			} else {
				txHash = tx->getHash();
				_peer->Pdebug("Got tx: %s", Utils::UInt256ToString(txHash));

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

			return false;
		}

		void TransactionMessage::Send(const SendMessageParameter &param) {
			const TransactionParameter &txParam = static_cast<const TransactionParameter &>(param);

			ByteStream stream;
			txParam.tx->Serialize(stream);
			CMBlock buf = stream.getBuffer();
			_peer->Pinfo("Sending tx: tx hash = %s", Utils::UInt256ToString(txParam.tx->getHash()));
			SendMessage(stream.getBuffer(), Type());
		}

		std::string TransactionMessage::Type() const {
			return MSG_TX;
		}

	}
}
