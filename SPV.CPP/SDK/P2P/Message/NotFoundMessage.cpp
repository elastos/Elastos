// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NotFoundMessage.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/P2P/Peer.h>

#include <Core/BRInt.h>
#include <Core/BRArray.h>
#include <Core/BRAddress.h>

#include <cfloat>

namespace Elastos {
	namespace ElaWallet {

		NotFoundMessage::NotFoundMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool NotFoundMessage::Accept(const CMBlock &msg) {
			size_t off = 0, count = 0;

			count = UInt32GetLE(&msg[off]);
			off += sizeof(uint32_t);

			if (off == 0 || off + 36*count > msg.GetSize()) {
				_peer->error("malformed notfound message, length is {}, should be {} for {} item(s)", msg.GetSize(),
							 BRVarIntSize(count) + 36 * count, count);
				return false;
			} else if (count > MAX_GETDATA_HASHES) {
				_peer->warn("dropping notfound message, {} is too many items, max is {}", count, MAX_GETDATA_HASHES);
				return true;
			} else {
				inv_type type;
				UInt256 hash;
				std::vector<UInt256> txHashes, blockHashes;

				_peer->info("got notfound with {} item(s)", count);

				for (size_t i = 0; i < count; i++) {
					type = (inv_type)UInt32GetLE(&msg[off]);
					UInt256Get(&hash, &msg[off + sizeof(uint32_t)]);

					_peer->info("not found type = {}, hash = {}", type, Utils::UInt256ToString(hash));

					switch (type) {
						case inv_tx: txHashes.push_back(hash); break;
						case inv_filtered_block: // drop through
						case inv_block: blockHashes.push_back(hash); break;
						default: break;
					}

					off += 36;
				}

				FireNotfound(txHashes, blockHashes);
			}

			return true;
		}

		void NotFoundMessage::Send(const SendMessageParameter &param) {

		}

		std::string NotFoundMessage::Type() const {
			return MSG_NOTFOUND;
		}

	}
}