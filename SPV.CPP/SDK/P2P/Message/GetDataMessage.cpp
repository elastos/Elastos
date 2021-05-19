// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "GetDataMessage.h"
#include "TransactionMessage.h"

#include <P2P/PeerManager.h>
#include <Common/Log.h>
#include <Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		GetDataMessage::GetDataMessage(const Elastos::ElaWallet::MessagePeerPtr &peer) : Message(peer) {

		}

		bool GetDataMessage::Accept(const bytes_t &msg) {
			ByteStream stream(msg);
			uint32_t count = 0;

			if (!stream.ReadUint32(count)) {
				_peer->error("malformed getdata message, read count fail");
				return false;
			}

			if (count > MAX_GETDATA_HASHES || 36 * count + 4 > msg.size()) {
				_peer->error("dropping getdata message, invalid count = {}", count);
				return false;
			}

			ByteStream notfound;
			TransactionPtr tx;

			_peer->info("got getdata with {} item(s)", count);
			for (size_t i = 0; i < count; i++) {
				uint32_t type;
				if (!stream.ReadUint32(type)) {
					_peer->error("read inv type fail");
					return false;
				}

				uint256 hash;
				if (!stream.ReadBytes(hash)) {
					_peer->error("read inv hash fail");
					return false;
				}

				switch (type) {
					case inv_tx:
						tx = FireRequestedTx(hash);

						if (tx != nullptr/* && tx->EstimateSize() < TX_MAX_SIZE*/) {
							TransactionParameter txParam;
							txParam.tx = tx;
							_peer->SendMessage(MSG_TX, txParam);
							break;
						}

						// fall through
					default:
						notfound.WriteUint32(type);
						notfound.WriteBytes(hash);
						_peer->info("not found with type = {}, hash = {}", type, hash.GetHex());
						break;
				}
			}

			if (notfound.GetBytes().size() > 0) {
				SendMessage(notfound.GetBytes(), MSG_NOTFOUND);
			}
			return true;
		}

		void GetDataMessage::Send(const SendMessageParameter &param) {
			const GetDataParameter &getDataParameter = static_cast<const GetDataParameter &>(param);

			bool containBlocks = false;
			size_t i;
			size_t txCount = getDataParameter.txHashes.size();
			size_t blockCount = getDataParameter.blockHashes.size();
			uint32_t count = uint32_t(txCount + blockCount);

			if (count > 1000) {
				_peer->warn("couldn't send getdata, {} is too many items, max is {}", count, 1000);
				count = 1000;
			}

			if (count > 0) {
				bool isMainchain = _peer->GetPeerManager()->GetID().find(CHAINID_MAINCHAIN) != std::string::npos;
				ByteStream stream;
				stream.WriteUint32(count);

				for (i = 0; i < txCount && i < count; i++) {
					stream.WriteUint32(uint32_t(inv_tx));
					stream.WriteBytes(getDataParameter.txHashes[i]);
				}

				for (i = 0; i < blockCount && txCount + i < count; i++) {
#ifdef SUPPORT_NEW_INV_TYPE
					if (!isMainchain)
						stream.WriteUint32(uint32_t(inv_filtered_sidechain_block));
					else
#endif
						stream.WriteUint32(uint32_t(inv_filtered_block));
					stream.WriteBytes(getDataParameter.blockHashes[i]);
					containBlocks = true;
				}

				if (containBlocks && blockCount > 1)
					_peer->SetWaitingBlocks(true);
				_peer->SetSentGetdata(true);
				SendMessage(stream.GetBytes(), Type());
			}
		}

		std::string GetDataMessage::Type() const {
			return MSG_GETDATA;
		}

	}
}
