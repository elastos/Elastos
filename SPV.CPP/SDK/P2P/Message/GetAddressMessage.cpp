// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "GetAddressMessage.h"

#include <P2P/Peer.h>
#include <P2P/PeerManager.h>

namespace Elastos {
	namespace ElaWallet {

		GetAddressMessage::GetAddressMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool GetAddressMessage::Accept(const bytes_t &msg) {
			_peer->info("got getaddr");
			_peer->SendMessage(MSG_ADDR, Message::DefaultParam);
			return true;
		}

		void GetAddressMessage::Send(const SendMessageParameter &param) {
			_peer->SetSentGetaddr(true);
			_peer->GetPeerManager()->SetKeepAliveTimestamp(time(nullptr));
			SendMessage(bytes_t(), Type());
		}

		std::string GetAddressMessage::Type() const {
			return MSG_GETADDR;
		}
	}
}
