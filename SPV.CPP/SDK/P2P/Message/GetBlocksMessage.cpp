// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "GetBlocksMessage.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/P2P/Peer.h>

namespace Elastos {
	namespace ElaWallet {

		GetBlocksMessage::GetBlocksMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool GetBlocksMessage::Accept(const CMBlock &msg) {
			_peer->error("dropping {} message", Type());
			return false;
		}

		void GetBlocksMessage::Send(const SendMessageParameter &param) {
			const GetBlocksParameter &getBlocksParameter = static_cast<const GetBlocksParameter &>(param);

			size_t i, locatorsCount;
			ByteStream msg;

			locatorsCount = getBlocksParameter.locators.size();
			msg.WriteUint32(uint32_t(locatorsCount));

			for (i = 0; i < locatorsCount; i++) {
				msg.WriteBytes(&getBlocksParameter.locators[i], sizeof(UInt256));
			}

			msg.WriteBytes(&getBlocksParameter.hashStop, sizeof(UInt256));

			if (locatorsCount > 0) {
				_peer->debug("calling getblocks with {} locators: [{},{} {}]",
							 locatorsCount,
							 Utils::UInt256ToString(getBlocksParameter.locators[0], true),
							 (locatorsCount > 2 ? " ...," : ""),
							 (locatorsCount > 1 ? Utils::UInt256ToString(getBlocksParameter.locators[locatorsCount - 1], true)
												: ""));
				SendMessage(msg.GetBuffer(), Type());
			}
		}

		std::string GetBlocksMessage::Type() const {
			return MSG_GETBLOCKS;
		}
	}
}