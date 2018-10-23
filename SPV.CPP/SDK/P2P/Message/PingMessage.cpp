// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sys/time.h>
#include <BRMerkleBlock.h>
#include <SDK/Common/Utils.h>

#include "BRArray.h"

#include "SDK/P2P/Peer.h"
#include "PingMessage.h"
#include "SDK/P2P/PeerManager.h"

namespace Elastos {
	namespace ElaWallet {

		PingMessage::PingMessage(const MessagePeerPtr &peer) :
				Message(peer) {

		}

		bool PingMessage::Accept(const CMBlock &msg) {
			bool r = true;

			if (sizeof(uint64_t) > msg.GetSize()) {
				_peer->Pwarn("malformed ping message, length is {}, should be {}", msg.GetSize(), sizeof(uint64_t));
				r = false;
			} else {
				_peer->Pinfo("got ping");
				_peer->SendMessage(msg, MSG_PONG);

				if (_peer->SentVerack() && _peer->GotVerack() && _peer->SentFilter() && _peer->SentMempool()) {
					PeerManager *manager = _peer->getPeerManager();
					int haveTxPending = 0;

					manager->Lock();
					for (size_t i = manager->getPublishedTransaction().size(); i > 0; i--) {
						if (manager->getPublishedTransaction()[i - 1].HasCallback()) {
							_peer->Pinfo("publish pending tx hash = {}",
										 Utils::UInt256ToString(manager->getPublishedTransactionHashes()[i - 1]));
							haveTxPending++;
						}
					}
					manager->Unlock();

					if (manager->GetLastBlockHeight() >= *(uint64_t *)(uint8_t *)msg && !haveTxPending) {
						manager->Lock();
						if (manager->reconnectTaskCount() == 0) {
							manager->reconnectTaskCount()++;
							manager->Unlock();

							FireRelayedPingMsg();
						} else {
							manager->Unlock();
						}
					}
				}
			}

			return r;
		}

		void PingMessage::Send(const SendMessageParameter &param) {
			uint8_t msg[sizeof(uint64_t)];
			struct timeval tv;

			gettimeofday(&tv, nullptr);

			const PingParameter &pingParameter = dynamic_cast<const PingParameter &>(param);
			_peer->SetStartTime(tv.tv_sec + (double) tv.tv_usec / 1000000);

			_peer->addPongCallback(pingParameter.callback);
			UInt64SetLE(msg, _peer->getPeerManager()->GetLastBlockHeight());

			CMBlock msgBlock(sizeof(msg));
			memcpy(msgBlock, msg, sizeof(msg));
			_peer->SendMessage(msgBlock, MSG_PING);
		}

		std::string PingMessage::Type() const {
			return MSG_PING;
		}
	}
}