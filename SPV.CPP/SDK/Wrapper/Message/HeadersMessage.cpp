// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "HeadersMessage.h"

namespace Elastos {
	namespace ElaWallet {

		HeadersMessage::HeadersMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool HeadersMessage::Accept(const CMBlock &msg) {
			//fixme [refactor]
			return false;
		}

		void HeadersMessage::Send(const SendMessageParameter &param) {
			//fixme [refactor]
		}

		std::string HeadersMessage::Type() const {
			return MSG_HEADERS;
		}
	}
}
