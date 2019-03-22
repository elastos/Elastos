// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "RejectMessage.h"

#include <SDK/P2P/Peer.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {
		RejectMessage::RejectMessage(const MessagePeerPtr &peer) :
			Message(peer) {
		}

		bool RejectMessage::Accept(const bytes_t &msg) {
			ByteStream stream(msg);

			std::string type;
			if (!stream.ReadVarString(type)) {
				_peer->error("malformed reject message, read var string 'type' error");
				return false;
			}

			uint8_t code;
			if (!stream.ReadByte(code)) {
				_peer->error("malformed reject message, read code error");
				return false;
			}

			std::string reason;
			if (!stream.ReadVarString(reason)) {
				_peer->error("malformed reject message, read reason error");
				return false;
			}

			if (type == MSG_TX) {
				uint256 txHash;

				if (!stream.ReadBytes(txHash)) {
					_peer->error("malformed reject message, read tx hash error");
					return false;
				}

				_peer->info("rejected {} code: 0x{:x} reason: {} txid: {}", type, code,
							reason, txHash.GetHex());
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
