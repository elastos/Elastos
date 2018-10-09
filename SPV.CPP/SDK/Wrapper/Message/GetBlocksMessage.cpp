// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "GetBlocksMessage.h"
#include "Log.h"
#include "Utils.h"

namespace Elastos {
	namespace ElaWallet {

		GetBlocksMessage::GetBlocksMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool GetBlocksMessage::Accept(const CMBlock &msg) {
			return false;
		}

		void GetBlocksMessage::Send(const SendMessageParameter &param) {
			const GetBlocksParameter &getBlocksParameter = static_cast<const GetBlocksParameter &>(param);

			size_t i, off = 0;
			size_t msgLen = sizeof(uint32_t) + sizeof(UInt256) * getBlocksParameter.locators.size() + sizeof(getBlocksParameter.hashStop);
			uint8_t msg[msgLen];

			UInt32SetLE(&msg[off], uint32_t(getBlocksParameter.locators.size()));
			off += sizeof(uint32_t);

			for (i = 0; i < getBlocksParameter.locators.size(); i++) {
				UInt256Set(&msg[off], getBlocksParameter.locators[i]);
				off += sizeof(UInt256);
			}

			UInt256Set(&msg[off], getBlocksParameter.hashStop);
			off += sizeof(UInt256);

			if (getBlocksParameter.locators.size() > 0) {
//				peer_dbg(peer, "calling getblocks with %zu locators: [%s,%s %s]",
//						 locatorsCount,
//						 Utils::UInt256ToString(locators[0]).c_str(),
//						 (locatorsCount > 2 ? " ...," : ""),
//						 (locatorsCount > 1 ? Utils::UInt256ToString(locators[locatorsCount - 1]).c_str() : ""));
//				BRPeerSendMessage(peer, msg, off, MSG_GETBLOCKS);
			}
		}

		std::string GetBlocksMessage::Type() const {
			return MSG_GETBLOCKS;
		}
	}
}