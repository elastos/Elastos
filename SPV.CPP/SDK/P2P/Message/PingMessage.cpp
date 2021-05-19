// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PingMessage.h"
#include "PongMessage.h"
#include "MempoolMessage.h"

#include <Common/Utils.h>
#include <P2P/Peer.h>
#include <P2P/PeerManager.h>

#include <sys/time.h>

namespace Elastos {
	namespace ElaWallet {

		PingMessage::PingMessage(const MessagePeerPtr &peer) :
				Message(peer) {

		}

		bool PingMessage::Accept(const bytes_t &msg) {
			ByteStream stream(msg);
			uint64_t height;

			if (!stream.ReadUint64(height)) {
				_peer->error("malformed ping message, length is {}, should be 8", msg.size());
				return false;
			}

			_peer->info("got ping");
			PongParameter pongParam(height);
			_peer->SendMessage(MSG_PONG, pongParam);

			FireRelayedPing();

			return true;
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