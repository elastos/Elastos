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
			_blockHeight(0) {

		}

		PayloadSideMining::PayloadSideMining(const PayloadSideMining &payload) {
			operator=(payload);
		}


		PayloadSideMining::PayloadSideMining(const uint256 &sideBlockHash, const uint256 &sideGensisHash, uint32_t height, const bytes_t &signedData) {
			_sideBlockHash = sideBlockHash;
			_sideGenesisHash = sideGensisHash;
			_blockHeight = height;
			_signedData = signedData;
		}

		PayloadSideMining::~PayloadSideMining() {

		}

		void PayloadSideMining::SetSideBlockHash(const uint256 &sideBlockHash) {
			_sideBlockHash = sideBlockHash;
		}

		void PayloadSideMining::SetSideGenesisHash(const uint256 &sideGensisHash) {
			_sideGenesisHash = sideGensisHash;
		}

		void PayloadSideMining::SetBlockHeight(uint32_t height) {
			_blockHeight = height;
		}

		void PayloadSideMining::SetSignedData(const bytes_t &signedData) {
			_signedData = signedData;
		}

		const uint256 &PayloadSideMining::GetSideBlockHash() const {
			return _sideBlockHash;
		}

		const uint256 &PayloadSideMining::GetSideGenesisHash() const {
			return _sideGenesisHash;
		}

		const uint32_t &PayloadSideMining::GetBlockHeight() const {
			return _blockHeight;
		}

		const bytes_t &PayloadSideMining::GetSignedData() const {
			return _signedData;
		}

		void PayloadSideMining::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteBytes(_sideBlockHash);
			ostream.WriteBytes(_sideGenesisHash);
			ostream.WriteUint32(_blockHeight);
			ostream.WriteVarBytes(_signedData);
		}

		bool PayloadSideMining::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadBytes(_sideBlockHash))
				return false;

			if (!istream.ReadBytes(_sideGenesisHash))
				return false;

			if (!istream.ReadUint32(_blockHeight))
				return false;

			return istream.ReadVarBytes(_signedData);

		}

		nlohmann::json PayloadSideMining::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["SideBlockHash"] = _sideBlockHash.GetHex();
			j["SideGenesisHash"] = _sideGenesisHash.GetHex();
			j["BlockHeight"] = _blockHeight;
			j["SignedData"] = _signedData.getHex();

			return j;
		}

		void PayloadSideMining::FromJson(const nlohmann::json &j, uint8_t version) {
			_sideBlockHash.SetHex(j["SideBlockHash"].get<std::string>());
			_sideGenesisHash.SetHex(j["SideGenesisHash"].get<std::string>());
			_blockHeight = j["BlockHeight"].get<uint32_t>();
			_signedData.setHex(j["SignedData"].get<std::string>());
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
			_signedData = payload._signedData;

			return *this;
		}

	}
}