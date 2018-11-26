// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "GetHeadersMessage.h"

#include <SDK/Common/Utils.h>
#include <SDK/P2P/Peer.h>

namespace Elastos {
	namespace ElaWallet {

		GetHeadersMessage::GetHeadersMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool GetHeadersMessage::Accept(const CMBlock &msg) {
			_peer->Perror("dropping {} message", Type());
			return false;
		}

		void GetHeadersMessage::Send(const SendMessageParameter &param) {
			const GetHeadersParameter &getHeadersParameter = static_cast<const GetHeadersParameter &>(param);
			size_t locatorsCount = getHeadersParameter.locators.size();
			ByteStream msg;

			msg.writeUint32(uint32_t(locatorsCount));
			for (size_t i = 0; i < locatorsCount; ++i) {
				msg.writeBytes(&getHeadersParameter.locators[i], sizeof(UInt256));
			}
			msg.writeBytes(&getHeadersParameter.hashStop, sizeof(UInt256));

			if (locatorsCount > 0) {
				_peer->Pdebug("calling getheaders with {} locators: [{},{} {}]",
							  locatorsCount,
							  Utils::UInt256ToString(getHeadersParameter.locators[0]),
							  (locatorsCount > 2 ? " ...," : ""),
							  (locatorsCount > 1 ? Utils::UInt256ToString(getHeadersParameter.locators[locatorsCount - 1]) : ""));
				SendMessage(msg.getBuffer(), Type());
			}
		}

		std::string GetHeadersMessage::Type() const {
			return MSG_GETHEADERS;
		}
	}
}
