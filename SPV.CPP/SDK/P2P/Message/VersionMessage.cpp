// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "VersionMessage.h"

#include <sys/time.h>
#include <support/BRKey.h>

#define ENABLED_SERVICES   0ULL  // we don't provide full blocks to remote nodes
#define PROTOCOL_VERSION   70013

namespace Elastos {
	namespace ElaWallet {

		VersionMessage::VersionMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool VersionMessage::Accept(const bytes_t &msg) {
			ByteStream stream(msg);

			uint32_t version = 0;
			if (!stream.ReadUint32(version)) {
				_peer->error("malformed version message, parse version fail");
				return false;
			}
			_peer->SetVersion(version);

			uint64_t services = 0;
			if (!stream.ReadUint64(services)) {
				_peer->error("malformed version message, parse services fail");
				return false;
			}
			_peer->SetServices(services);

			uint32_t timestamp = 0;
			if (!stream.ReadUint32(timestamp)) {
				_peer->error("malformed version message, parse timestamp fail");
				return false;
			}
//			_peer->SetTimestamp(timestamp);

			uint16_t port = 0;
			if (!stream.ReadUint16(port)) {
				_peer->error("malformed version message, parse port fail");
				return false;
			}
//			_peer->SetPort(port);

			uint64_t nonce = 0;
			if (!stream.ReadUint64(nonce)) {
				_peer->error("malformed version message, parse nonce fail");
				return false;
			}
			_peer->SetNonce(nonce);

			uint64_t height = 0;
			if (!stream.ReadUint64(height)) {
				_peer->error("malformed version message, parse height fail");
				return false;
			}
			_peer->SetLastBlock(uint32_t(height));

			_peer->info("got version {}, services {}", _peer->GetVersion(), _peer->GetServices());
			SendMessageParameter param;
			_peer->SendMessage(MSG_VERACK, param);
			return true;
		}

		void VersionMessage::Send(const SendMessageParameter &param) {
			ByteStream stream;

			stream.WriteUint32(PROTOCOL_VERSION);
			stream.WriteUint64(ENABLED_SERVICES);
			stream.WriteUint32(uint32_t(time(nullptr)));
			stream.WriteUint16(_peer->GetPort());
			_peer->SetNonce(((uint64_t)BRRand(0) << 32) | (uint64_t)BRRand(0));
			stream.WriteUint64(_peer->GetNonce());
			stream.WriteUint64(0);
			stream.WriteUint8(0);

			SendMessage(stream.GetBytes(), Type());
		}

		std::string VersionMessage::Type() const {
			return MSG_VERSION;
		}

	}
}