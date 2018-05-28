// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>

#include "PayloadSideMining.h"
#include "Utils.h"

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

		void PayloadSideMining::setSideBlockHash(const UInt256 &sideBlockHash) {
			_sideBlockHash = sideBlockHash;
		}

		void PayloadSideMining::setSideGenesisHash(const UInt256 &sideGensisHash) {
			_sideGenesisHash = sideGensisHash;
		}

		CMBlock PayloadSideMining::getData() const {
			CMBlock buff(uint64_t(sizeof(_sideBlockHash) + sizeof(_sideGenesisHash)));

			memcpy(buff, _sideBlockHash.u8, sizeof(_sideBlockHash));
			memcpy(&buff[(uint64_t)sizeof(_sideBlockHash)], _sideGenesisHash.u8, sizeof(_sideGenesisHash));

			return buff;
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

		nlohmann::json PayloadSideMining::toJson() {
			nlohmann::json jsonData;

			jsonData["sideBlockHash"] = Utils::UInt256ToString(_sideBlockHash);

			jsonData["sideGenesisHash"] = Utils::UInt256ToString(_sideGenesisHash);

			return jsonData;
		}

		void PayloadSideMining::fromJson(const nlohmann::json &jsonData) {
			std::string sideBlockHash = jsonData["sideBlockHash"].get<std::string>();
			_sideBlockHash = Utils::UInt256FromString(sideBlockHash);

			std::string sideGenesisHash = jsonData["sideGenesisHash"].get<std::string>();
			_sideGenesisHash = Utils::UInt256FromString(sideGenesisHash);
		}
	}
}