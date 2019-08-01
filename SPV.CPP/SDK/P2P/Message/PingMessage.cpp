// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PingMessage.h"
#include "PongMessage.h"
#include "MempoolMessage.h"

#include <SDK/Common/Utils.h>
#include <SDK/P2P/Peer.h>
#include <SDK/P2P/PeerManager.h>

#include <sys/time.h>

namespace Elastos {
	namespace ElaWallet {

		PingMessage::PingMessage(const MessagePeerPtr &peer) :
				Message(peer) {

		}

		bool PingMessage::Accept(const bytes_t &msg) {
			bool r = true;

			if (sizeof(uint64_t) > msg.size()) {
				_peer->warn("malformed ping message, length is {}, should be {}", msg.size(), sizeof(uint64_t));
				r = false;
			} else {
				_peer->info("got ping");
				bool needRelayPing = false, hasPendingTx = false;
				PeerManager *manager = _peer->GetPeerManager();

				PongParameter pongParameter;
				pongParameter.lastBlockHeight = manager->GetLastBlockHeight();
				_peer->SendMessage(MSG_PONG, pongParameter);

				if (manager->GetConnectStatus() == Peer::Connected && manager->SyncSucceeded() &&
					time_after(time(nullptr), manager->GetKeepAliveTimestamp() + 30)) {
					needRelayPing = true;
				}

				std::vector<PublishedTransaction> publishedTx = manager->GetPublishedTransaction();
				for (size_t i = publishedTx.size(); i > 0; i--) {
					if (publishedTx[i - 1].HasCallback()) {
						_peer->info("publish pending tx hash = {}, do not disconnect",
									publishedTx[i - 1].GetTransaction()->GetHash().GetHex());
						hasPendingTx = true;
						break;
					}
				}

				if (needRelayPing && !hasPendingTx) {
					FireRelayedPingMsg();
//				} else if (hasPendingTx) {
//					MempoolParameter mempoolParameter;
//					mempoolParameter.CompletionCallback = boost::function<void(int)>();
//					_peer->SendMessage(MSG_MEMPOOL, mempoolParameter);
				}
			}

			return r;
		}

		void PingMessage::Send(const SendMessageParameter &param) {
			ByteStream stream;
			struct timeval tv;

			gettimeofday(&tv, nullptr);

			const PingParameter &pingParameter = dynamic_cast<const PingParameter &>(param);
			_peer->SetStartTime(tv.tv_sec + (double) tv.tv_usec / 1000000);

			_peer->AddPongCallback(pingParameter.callback);

			stream.WriteUint64(pingParameter.lastBlockHeight);

			_peer->SendMessage(stream.GetBytes(), MSG_PING);
		}

		std::string PingMessage::Type() const {
			return MSG_PING;
		}
	}
}