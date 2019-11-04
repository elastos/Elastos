// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NotFoundMessage.h"

#include <Common/Log.h>
#include <Common/Utils.h>
#include <P2P/Peer.h>

#include <cfloat>

namespace Elastos {
	namespace ElaWallet {

		NotFoundMessage::NotFoundMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool NotFoundMessage::Accept(const bytes_t &msg) {
			ByteStream stream(msg);
			uint32_t count = 0;

			if (!stream.ReadUint32(count)) {
				_peer->error("malformed notfound message");
				return false;
			}

			if (count > MAX_GETDATA_HASHES) {
				_peer->warn("dropping notfound message, {} is too many items, max is {}", count, MAX_GETDATA_HASHES);
				return true;
			}

			uint32_t type;
			uint256 hash;
			std::vector<uint256> txHashes, blockHashes;

			_peer->info("got notfound with {} item(s)", count);

			for (size_t i = 0; i < count; i++) {
				if (!stream.ReadUint32(type)) {
					_peer->error("notfound msg read type fail");
					return false;
				}

				if (!stream.ReadBytes(hash)) {
					_peer->error("notfound msg read hash fail");
					return false;
				}

				_peer->info("not found type = {}, hash = {}", type, hash.GetHex());

				switch (type) {
					case inv_tx: txHashes.push_back(hash); break;
					case inv_filtered_block: // drop through
					case inv_block: blockHashes.push_back(hash); break;
					default: break;
				}
			}

			_peer->RemoveKnownTxHashes(txHashes);
			FireNotfound(txHashes, blockHashes);

			return true;
		}

		void NotFoundMessage::Send(const SendMessageParameter &param) {

		}

		std::string NotFoundMessage::Type() const {
			return MSG_NOTFOUND;
		}

	}
}