// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRArray.h>
#include <sys/time.h>

#include "PongMessage.h"
#include "Log.h"
#include "Peer.h"

namespace Elastos {
	namespace ElaWallet {

		PongMessage::PongMessage(const MessagePeerPtr &peer) :
				Message(peer) {
		}

		bool PongMessage::Accept(const CMBlock &msg) {
			struct timeval tv;
			double pingTime;
			bool r = true;

			if (sizeof(uint64_t) > msg.GetSize()) {
				_peer->Pwarn("malformed pong message, length is {}, should be {}", msg.GetSize(), sizeof(uint64_t));
				r = false;
			} else if (_peer->getPongCallbacks().empty()) {
				_peer->Pwarn("got unexpected pong");
				r = false;
			} else {
				if (_peer->GetStartTime() > 1) {
					gettimeofday(&tv, nullptr);
					pingTime = tv.tv_sec + (double) tv.tv_usec / 1000000 - _peer->GetStartTime();

					// 50% low pass filter on current ping time
					_peer->SetPingTime(_peer->GetPingTime() * 0.5 + pingTime * 0.5);
					_peer->SetStartTime(0);
					_peer->Pinfo("got pong in {}", pingTime);
				} else {
					_peer->Pinfo("got pong");
				}

				if (_peer->getPongCallbacks().size() > 0) {
					Peer::PeerCallback callback = _peer->popPongCallback();
					if (!callback.empty()) callback(1);
				}
			}

			return r;
		}

		void PongMessage::Send(const SendMessageParameter &param) {

		}

		std::string PongMessage::Type() const {
			return MSG_PONG;
		}

	}
}