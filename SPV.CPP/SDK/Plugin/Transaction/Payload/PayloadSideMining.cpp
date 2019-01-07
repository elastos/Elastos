// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadSideMining.h"
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>

#include <cstring>

namespace Elastos {
	namespace ElaWallet {

		PayloadSideMining::PayloadSideMining() :
			_sideBlockHash(UINT256_ZERO),
			_sideGenesisHash(UINT256_ZERO),
			_blockHeight(0) {

		}

		PayloadSideMining::PayloadSideMining(const PayloadSideMining &payload) {
			operator=(payload);
		}


		PayloadSideMining::PayloadSideMining(const UInt256 &sideBlockHash, const UInt256 &sideGensisHash, uint32_t height, const CMBlock &signedData) {
			UInt256Set(&_sideBlockHash, sideBlockHash);
			UInt256Set(&_sideGenesisHash, sideGensisHash);
			_blockHeight = height;
			_signedData = signedData;
		}

		PayloadSideMining::~PayloadSideMining() {

		}

		void PayloadSideMining::SetSideBlockHash(const UInt256 &sideBlockHash) {
			_sideBlockHash = sideBlockHash;
		}

		void PayloadSideMining::SetSideGenesisHash(const UInt256 &sideGensisHash) {
			_sideGenesisHash = sideGensisHash;
		}

		void PayloadSideMining::SetBlockHeight(uint32_t height) {
			_blockHeight = height;
		}

		void PayloadSideMining::SetSignedData(const CMBlock &signedData) {
			_signedData = signedData;
		}

		const UInt256 &PayloadSideMining::GetSideBlockHash() const {
			return _sideBlockHash;
		}

		const UInt256 &PayloadSideMining::GetSideGenesisHash() const {
			return _sideGenesisHash;
		}

		const uint32_t &PayloadSideMining::GetBlockHeight() const {
			return _blockHeight;
		}

		const CMBlock &PayloadSideMining::GetSignedData() const {
			return _signedData;
		}

		void PayloadSideMining::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.writeBytes(_sideBlockHash.u8, sizeof(UInt256));
			ostream.writeBytes(_sideGenesisHash.u8, sizeof(UInt256));
			ostream.writeUint32(_blockHeight);
			ostream.writeVarBytes(_signedData);
		}

		bool PayloadSideMining::Deserialize(ByteStream &istream, uint8_t version) {
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

		IPayload &PayloadSideMining::operator=(const IPayload &payload) {
			try {
				const PayloadSideMining &payloadSideMining = dynamic_cast<const PayloadSideMining &>(payload);
				operator=(payloadSideMining);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadSideMining");
			}

			return *this;
		}

		PayloadSideMining &PayloadSideMining::operator=(const PayloadSideMining &payload) {
			_sideBlockHash = payload._sideBlockHash;
			_sideGenesisHash = payload._sideGenesisHash;
			_blockHeight = payload._blockHeight;
			_signedData.Memcpy(payload._signedData);

			return *this;
		}

	}
}