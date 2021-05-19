// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "VerackMessage.h"
#include <P2P/Peer.h>

#include <float.h>
#include <sys/time.h>

namespace Elastos {
	namespace ElaWallet {

		VerackMessage::VerackMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool VerackMessage::Accept(const bytes_t &msg) {
			if (_peer->GotVerack()) {
				_peer->error("got unexcepted verack");
			} else {
				struct timeval tv;
				gettimeofday(&tv, nullptr);
				_peer->SetPingTime(tv.tv_sec + (double)tv.tv_usec / 1000000 - _peer->GetStartTime());
				_peer->SetStartTime(0);
				_peer->info("got verack in {}", _peer->GetPingTime());
				_peer->SetGotVerack(true);

				if (_peer->GetConnectStatus() == Peer::ConnectStatus::Connecting &&
					_peer->SentVerack() && _peer->GotVerack()) {
					_peer->info("handshake completed");
					_peer->SetDisconnectTime(DBL_MAX);
					_peer->SetConnectStatus(Peer::ConnectStatus::Connected);
					_peer->info("connected with last block height: {}", _peer->GetLastBlock());
					FireConnected();
				}
			}

			return true;
		}

		void VerackMessage::Send(const SendMessageParameter &param) {
			SendMessage(bytes_t(), Type());
			_peer->SetSentVerack(true);
		}

		std::string VerackMessage::Type() const {
			return MSG_VERACK;
		}
	}
}
