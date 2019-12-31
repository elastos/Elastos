// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "FilterLoadMessage.h"

#include <P2P/Peer.h>
#include <P2P/PeerManager.h>
#include <WalletCore/BloomFilter.h>
#include <Common/ByteStream.h>

namespace Elastos {
	namespace ElaWallet {

		FilterLoadMessage::FilterLoadMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool FilterLoadMessage::Accept(const bytes_t &msg) {
			_peer->error("dropping {} message", Type());
			return false;
		}

		void FilterLoadMessage::Send(const SendMessageParameter &param) {
			const FilterLoadParameter &filterLoadParameter = static_cast<const FilterLoadParameter &>(param);
			ByteStream stream;
			filterLoadParameter.Filter->Serialize(stream);
			_peer->SetSentFilter(true);
			_peer->SetSentMempool(false);
			SendMessage(stream.GetBytes(), Type());
		}

		std::string FilterLoadMessage::Type() const {
			return MSG_FILTERLOAD;
		}
	}
}