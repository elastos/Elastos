// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sys/time.h>
#include <cfloat>
#include <Core/BRTransaction.h>

#include "VersionMessage.h"

#define ENABLED_SERVICES   0ULL  // we don't provide full blocks to remote nodes
#define PROTOCOL_VERSION   70013

namespace Elastos {
	namespace ElaWallet {

		VersionMessage::VersionMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool VersionMessage::Accept(const CMBlock &msg) {
			ByteStream stream(msg);

			uint32_t version = 0;
			if (!stream.readUint32(version)) {
				_peer->Perror("malformed version message, parse version fail");
				return false;
			}
			_peer->SetVersion(version);

			uint64_t services = 0;
			if (!stream.readUint64(services)) {
				_peer->Perror("malformed version message, parse services fail");
				return false;
			}
			_peer->SetServices(services);

			uint32_t timestamp = 0;
			if (!stream.readUint32(timestamp)) {
				_peer->Perror("malformed version message, parse timestamp fail");
				return false;
			}
			_peer->SetTimestamp(timestamp);

			uint16_t port = 0;
			if (!stream.readUint16(port)) {
				_peer->Perror("malformed version message, parse port fail");
				return false;
			}
			_peer->SetPort(port);

			uint64_t nonce = 0;
			if (!stream.readUint64(nonce)) {
				_peer->Perror("malformed version message, parse nonce fail");
				return false;
			}
			_peer->SetNonce(nonce);

			uint64_t height = 0;
			if (!stream.readUint64(height)) {
				_peer->Perror("malformed version message, parse height fail");
				return false;
			}
			_peer->SetLastBlock(uint32_t(height));

			_peer->Pinfo("got version {}, services {}", _peer->GetVersion(), _peer->GetServices());
			SendMessageParameter param;
			_peer->SendMessage(MSG_VERACK, param);
			return true;
		}

		void VersionMessage::Send(const SendMessageParameter &param) {
			ByteStream stream;

			stream.writeUint32(PROTOCOL_VERSION);
			stream.writeUint64(ENABLED_SERVICES);
			stream.writeUint32(uint32_t(time(nullptr)));
			stream.writeUint16(_peer->GetPort());
			_peer->SetNonce(((uint64_t)BRRand(0) << 32) | (uint64_t)BRRand(0));
			stream.writeUint64(_peer->GetNonce());
			stream.writeUint64(0);
			stream.writeUint8(0);

			SendMessage(stream.getBuffer(), Type());
		}

		std::string VersionMessage::Type() const {
			return MSG_VERSION;
		}

	}
}