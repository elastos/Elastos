// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SDK/P2P/Peer.h"
#include "SDK/Base/BloomFilter.h"
#include "BloomFilterMessage.h"
#include "ByteStream.h"

namespace Elastos {
	namespace ElaWallet {

		BloomFilterMessage::BloomFilterMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool BloomFilterMessage::Accept(const CMBlock &msg) {
			_peer->Perror("dropping {} message", Type());
			return false;
		}

		void BloomFilterMessage::Send(const SendMessageParameter &param) {
			const BloomFilterParameter &bloomFilterParameter = static_cast<const BloomFilterParameter &>(param);
			ByteStream stream;
			bloomFilterParameter.Filter->Serialize(stream);
			_peer->SetSentFilter(true);
			_peer->SetSentMempool(false);
			SendMessage(stream.getBuffer(), Type());
		}

		std::string BloomFilterMessage::Type() const {
			return MSG_FILTERLOAD;
		}
	}
}