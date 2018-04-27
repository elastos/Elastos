// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadSideMining.h"

namespace Elastos {
	namespace SDK {

		PayloadSideMining::PayloadSideMining() :
				_sideBlockHash(UINT256_ZERO),
				_sideGenesisHash(UINT256_ZERO) {

		}

		PayloadSideMining::PayloadSideMining(const UInt256 &sideBlockHash, const UInt256 &sideGensisHash) {
			UInt256Set(&_sideBlockHash, sideBlockHash);
			UInt256Set(&_sideGenesisHash, sideGensisHash);
		}

		PayloadSideMining::~PayloadSideMining() {

		}

		ByteData PayloadSideMining::getData() const {
			uint8_t buff[sizeof(_sideBlockHash) + sizeof(_sideGenesisHash)];

			memcpy(buff, _sideBlockHash.u8, sizeof(_sideBlockHash));
			memcpy(buff, &buff[sizeof(_sideBlockHash)], sizeof(_sideGenesisHash));

			return ByteData(buff, sizeof(buff));
		}

		void PayloadSideMining::Serialize(ByteStream &ostream) const {
			uint8_t blockData[sizeof(_sideBlockHash)];
			UInt256Set(blockData, _sideBlockHash);
			ostream.putBytes(blockData, sizeof(blockData));

			uint8_t genesisData[sizeof(_sideGenesisHash)];
			UInt256Set(genesisData, _sideGenesisHash);
			ostream.putBytes(genesisData, sizeof(_sideGenesisHash));
		}

		void PayloadSideMining::Deserialize(ByteStream &istream) {
			uint8_t blockData[sizeof(_sideBlockHash)];
			istream.getBytes(blockData, sizeof(blockData));
			UInt256Get(&_sideBlockHash, blockData);

			uint8_t genesisData[sizeof(_sideGenesisHash)];
			istream.getBytes(genesisData, sizeof(_sideGenesisHash));
			UInt256Get(&_sideGenesisHash, genesisData);
		}
	}
}