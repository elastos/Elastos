// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "GetDataMessage.h"
#include "TransactionMessage.h"

#include <SDK/P2P/Peer.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

#include <Core/BRArray.h>

#define TX_MAX_SIZE          100000      // no tx can be larger than this size in bytes

namespace Elastos {
	namespace ElaWallet {

		GetDataMessage::GetDataMessage(const Elastos::ElaWallet::MessagePeerPtr &peer) : Message(peer) {

		}

		bool GetDataMessage::Accept(const CMBlock &msg) {

			size_t off = 0;
			uint32_t count = 0;

			count = UInt32GetLE(&msg[off]);
			off += sizeof(count);

			if (off == 0 || off + 36 * count > msg.GetSize()) {
				_peer->error("malformed getdata message, length is {}, should {} for {} item(s)",
							 msg.GetSize(), sizeof(count) + 36 * count, count);
				return false;
			} else if (count > MAX_GETDATA_HASHES) {
				_peer->error("dropping getdata message, {} is too many items, max is {}",
							 count, MAX_GETDATA_HASHES);
				return false;
			} else {
				struct inv_item {
					uint8_t item[36];
				};

				std::vector<inv_item> notfound;

				TransactionPtr tx;

				_peer->info("got getdata with {} item(s)", count);
				for (size_t i = 0; i < count; i++) {
					inv_type type = (inv_type) UInt32GetLE(&msg[off]);
					UInt256 hash;
					UInt256Get(&hash, &msg[off + sizeof(uint32_t)]);
					switch (type) {
						case inv_tx:
							tx = FireRequestedTx(hash);

							if (tx != nullptr && tx->getSize() < TX_MAX_SIZE) {
								TransactionParameter txParam;
								txParam.tx = tx;
								_peer->SendMessage(MSG_TX, txParam);
								break;
							}

							// fall through
						default:
							_peer->info("not found with type = {}, hash = {}", type, Utils::UInt256ToString(hash, true));
							notfound.push_back(*(struct inv_item *) &msg[off]);
							break;
					}

					off += 36;
				}
				if (notfound.size() > 0) {
					ByteStream stream;

					stream.writeUint32(uint32_t(notfound.size()));
					for (size_t i = 0; i < notfound.size(); ++i) {
						stream.writeBytes(notfound[i].item, sizeof(inv_item));
					}

					SendMessage(stream.getBuffer(), MSG_NOTFOUND);
				}
			}
			return true;
		}

		void GetDataMessage::Send(const SendMessageParameter &param) {
			const GetDataParameter &getDataParameter = static_cast<const GetDataParameter &>(param);

			size_t i;
			size_t txCount = getDataParameter.txHashes.size();
			size_t blockCount = getDataParameter.blockHashes.size();
			uint32_t count = uint32_t(txCount + blockCount);

			if (count > 1000) {
				_peer->warn("couldn't send getdata, {} is too many items, max is {}", count, 1000);
				count = 1000;
			}

			if (count > 0) {
				ByteStream stream;
				stream.writeUint32(count);

				for (i = 0; i < txCount && i < count; i++) {
					stream.writeUint32(uint32_t(inv_tx));
					stream.writeBytes(&getDataParameter.txHashes[i], sizeof(UInt256));
				}

				for (i = 0; i < blockCount && txCount + i < count; i++) {
					stream.writeUint32(uint32_t(inv_filtered_block));
					stream.writeBytes(&getDataParameter.blockHashes[i], sizeof(UInt256));
				}

				_peer->SetSentGetdata(true);
				SendMessage(stream.getBuffer(), Type());
			}
		}

		std::string GetDataMessage::Type() const {
			return MSG_GETDATA;
		}

	}
}
