// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PongMessage.h"

#include <Common/Log.h>
#include <P2P/Peer.h>

#include <sys/time.h>

namespace Elastos {
	namespace ElaWallet {

		PongMessage::PongMessage(const MessagePeerPtr &peer) :
				Message(peer) {
		}

		bool PongMessage::Accept(const bytes_t &msg) {
			struct timeval tv;
			double pingTime;
			bool r = true;

			if (sizeof(uint64_t) > msg.size()) {
				_peer->warn("malformed pong message, length is {}, should be {}", msg.size(), sizeof(uint64_t));
				r = false;
			} else if (_peer->GetPongCallbacks().empty()) {
				_peer->warn("got unexpected pong");
				r = false;
			} else {
				if (_peer->GetStartTime() > 1) {
					gettimeofday(&tv, nullptr);
					pingTime = tv.tv_sec + (double) tv.tv_usec / 1000000 - _peer->GetStartTime();

					// 50% low pass filter on current ping time
					_peer->SetPingTime(_peer->GetPingTime() * 0.5 + pingTime * 0.5);
					_peer->SetStartTime(0);
					_peer->info("got pong in {}", pingTime);
				} else {
					_peer->info("got pong");
				}

				if (_peer->GetPongCallbacks().size() > 0) {
					Peer::PeerCallback callback = _peer->PopPongCallback();
					if (!callback.empty()) callback(1);
				}
			}

			return r;
		}

		void PongMessage::Send(const SendMessageParameter &param) {
			const PongParameter &pongParameter = dynamic_cast<const PongParameter &>(param);
			ByteStream stream;
			stream.WriteUint64(pongParameter.lastBlockHeight);
			_peer->SendMessage(stream.GetBytes(), Type());
		}

		std::string PongMessage::Type() const {
			return MSG_PONG;
		}

	}
}