// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "RejectMessage.h"

#include <SDK/P2P/Peer.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

#include <Core/BRInt.h>

namespace Elastos {
	namespace ElaWallet {
		RejectMessage::RejectMessage(const MessagePeerPtr &peer) :
			Message(peer) {
		}

		bool RejectMessage::Accept(const CMBlock &msg) {
			size_t hashLen = 0;

			ByteStream stream(msg);

			std::string type;
			if (!stream.readVarString(type)) {
				_peer->error("malformed reject message, read var string 'type' error");
				return false;
			}

			if (type == MSG_TX) {
				hashLen = sizeof(UInt256);
			}

			uint8_t code;
			if (!stream.readByte(code)) {
				_peer->error("malformed reject message, read code error");
				return false;
			}

			std::string reason;
			if (!stream.readVarString(reason)) {
				_peer->error("malformed reject message, read reason error");
				return false;
			}

			UInt256 txHash = UINT256_ZERO;
			if (hashLen == sizeof(UInt256)) {
				if (!stream.readBytes(txHash.u8, sizeof(UInt256))) {
					_peer->error("malformed reject message, read tx hash error");
					return false;
				}
			}

			if (!UInt256IsZero(&txHash)) {
				_peer->info("rejected {} code: {:x} reason: {} txid: {}", type, code,
							reason, Utils::UInt256ToString(txHash, true));
				FireRejectedTx(txHash, code, reason);
			} else {
				_peer->info("rejected {} code: {:x} reason: {}", type, code, reason);
			}

			return true;
		}

		void RejectMessage::Send(const SendMessageParameter &param) {
			_peer->error("should not send reject message");
		}

		std::string RejectMessage::Type() const {
			return MSG_REJECT;
		}
	}
}
