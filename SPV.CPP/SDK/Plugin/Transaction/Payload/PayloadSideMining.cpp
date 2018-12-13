// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadSideMining.h"
#include <SDK/Common/Utils.h>

#include <cstring>

namespace Elastos {
	namespace ElaWallet {

		PayloadSideMining::PayloadSideMining() :
			_sideBlockHash(UINT256_ZERO),
			_sideGenesisHash(UINT256_ZERO),
			_blockHeight(0) {

		}

		PayloadSideMining::PayloadSideMining(const UInt256 &sideBlockHash, const UInt256 &sideGensisHash, uint32_t height, const CMBlock &signedData) {
			UInt256Set(&_sideBlockHash, sideBlockHash);
			UInt256Set(&_sideGenesisHash, sideGensisHash);
			_blockHeight = height;
			_signedData = signedData;
		}

		PayloadSideMining::~PayloadSideMining() {

		}

		void PayloadSideMining::setSideBlockHash(const UInt256 &sideBlockHash) {
			_sideBlockHash = sideBlockHash;
		}

		void PayloadSideMining::setSideGenesisHash(const UInt256 &sideGensisHash) {
			_sideGenesisHash = sideGensisHash;
		}

		void PayloadSideMining::setBlockHeight(uint32_t height) {
			_blockHeight = height;
		}

		void PayloadSideMining::setSignedData(const CMBlock &signedData) {
			_signedData = signedData;
		}

		CMBlock PayloadSideMining::getData() const {
			ByteStream stream;
			this->Serialize(stream);
			return stream.getBuffer();
		}

		void PayloadSideMining::Serialize(ByteStream &ostream) const {
			ostream.writeBytes(_sideBlockHash.u8, sizeof(UInt256));
			ostream.writeBytes(_sideGenesisHash.u8, sizeof(UInt256));
			ostream.writeUint32(_blockHeight);
			ostream.writeVarBytes(_signedData);
		}

		bool PayloadSideMining::Deserialize(ByteStream &istream) {
			if (!istream.readBytes(_sideBlockHash.u8, sizeof(UInt256)))
				return false;

			if (!istream.readBytes(_sideGenesisHash.u8, sizeof(UInt256)))
				return false;

			if (!istream.readUint32(_blockHeight))
				return false;

			return istream.readVarBytes(_signedData);

		}

		nlohmann::json PayloadSideMining::toJson() const {
			nlohmann::json j;

			j["SideBlockHash"] = Utils::UInt256ToString(_sideBlockHash, true);
			j["SideGenesisHash"] = Utils::UInt256ToString(_sideGenesisHash, true);
			j["BlockHeight"] = _blockHeight;
			j["SignedData"] = Utils::encodeHex(_signedData);

			return j;
		}

		void PayloadSideMining::fromJson(const nlohmann::json &j) {
			_sideBlockHash = Utils::UInt256FromString(j["SideBlockHash"].get<std::string>(), true);
			_sideGenesisHash = Utils::UInt256FromString(j["SideGenesisHash"].get<std::string>(), true);
			_blockHeight = j["BlockHeight"].get<uint32_t>();
			_signedData = Utils::decodeHex(j["SignedData"].get<std::string>());
		}
	}
}