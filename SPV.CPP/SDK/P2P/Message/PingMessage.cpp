// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PingMessage.h"
#include "PongMessage.h"

#include <SDK/Common/Utils.h>
#include <SDK/P2P/Peer.h>
#include <SDK/P2P/PeerManager.h>

#include <Core/BRMerkleBlock.h>
#include <Core/BRArray.h>

#include <sys/time.h>

namespace Elastos {
	namespace ElaWallet {

		PingMessage::PingMessage(const MessagePeerPtr &peer) :
				Message(peer) {

		}

#define time_after(a,b)  ((long)(b) - (long)(a) < 0)

		bool PingMessage::Accept(const CMBlock &msg) {
			bool r = true;

			if (sizeof(uint64_t) > msg.GetSize()) {
				_peer->warn("malformed ping message, length is {}, should be {}", msg.GetSize(), sizeof(uint64_t));
				r = false;
			} else {
				_peer->info("got ping");
				bool needRelayPing = false;
				PeerManager *manager = _peer->getPeerManager();

				PongParameter pongParameter;
				pongParameter.lastBlockHeight = manager->GetLastBlockHeight();
				_peer->SendMessage(MSG_PONG, pongParameter);

				if (manager->getConnectStatus() == Peer::Connected && manager->SyncSucceeded() &&
					time_after(time(nullptr), manager->getKeepAliveTimestamp() + 30)) {

					needRelayPing = true;
					for (size_t i = manager->getPublishedTransaction().size(); i > 0; i--) {
						if (manager->getPublishedTransaction()[i - 1].HasCallback()) {
							_peer->info("publish pending tx hash = {}, do not disconnect",
										Utils::UInt256ToString(manager->getPublishedTransactionHashes()[i - 1], true));
							needRelayPing = false;
							break;
						}
					}
				}

				if (needRelayPing)
					FireRelayedPingMsg();
			}

			return r;
		}

		void PingMessage::Send(const SendMessageParameter &param) {
			ByteStream stream;
			struct timeval tv;

			gettimeofday(&tv, nullptr);

			const PingParameter &pingParameter = dynamic_cast<const PingParameter &>(param);
			_peer->SetStartTime(tv.tv_sec + (double) tv.tv_usec / 1000000);

			_peer->addPongCallback(pingParameter.callback);

			stream.writeUint64(pingParameter.lastBlockHeight);

			_peer->SendMessage(stream.getBuffer(), MSG_PING);
		}

		std::string PingMessage::Type() const {
			return MSG_PING;
		}
	}
}