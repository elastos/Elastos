// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SideChainPow.h"
#include <Common/Utils.h>
#include <Common/Log.h>

#include <cstring>

namespace Elastos {
	namespace ElaWallet {

		SideChainPow::SideChainPow() :
			_blockHeight(0) {

		}

		SideChainPow::SideChainPow(const SideChainPow &payload) {
			operator=(payload);
		}


		SideChainPow::SideChainPow(const uint256 &sideBlockHash, const uint256 &sideGensisHash, uint32_t height, const bytes_t &signedData) {
			_sideBlockHash = sideBlockHash;
			_sideGenesisHash = sideGensisHash;
			_blockHeight = height;
			_signedData = signedData;
		}

		SideChainPow::~SideChainPow() {

		}

		void SideChainPow::SetSideBlockHash(const uint256 &sideBlockHash) {
			_sideBlockHash = sideBlockHash;
		}

		void SideChainPow::SetSideGenesisHash(const uint256 &sideGensisHash) {
			_sideGenesisHash = sideGensisHash;
		}

		void SideChainPow::SetBlockHeight(uint32_t height) {
			_blockHeight = height;
		}

		void SideChainPow::SetSignedData(const bytes_t &signedData) {
			_signedData = signedData;
		}

		const uint256 &SideChainPow::GetSideBlockHash() const {
			return _sideBlockHash;
		}

		const uint256 &SideChainPow::GetSideGenesisHash() const {
			return _sideGenesisHash;
		}

		const uint32_t &SideChainPow::GetBlockHeight() const {
			return _blockHeight;
		}

		const bytes_t &SideChainPow::GetSignedData() const {
			return _signedData;
		}

		size_t SideChainPow::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			size += _sideBlockHash.size();
			size += _sideGenesisHash.size();
			size += sizeof(_blockHeight);
			size += stream.WriteVarUint(_signedData.size());
			size += _signedData.size();

			return size;
		}

		void SideChainPow::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteBytes(_sideBlockHash);
			ostream.WriteBytes(_sideGenesisHash);
			ostream.WriteUint32(_blockHeight);
			ostream.WriteVarBytes(_signedData);
		}

		bool SideChainPow::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadBytes(_sideBlockHash))
				return false;

			if (!istream.ReadBytes(_sideGenesisHash))
				return false;

			if (!istream.ReadUint32(_blockHeight))
				return false;

			return istream.ReadVarBytes(_signedData);

		}

		nlohmann::json SideChainPow::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["SideBlockHash"] = _sideBlockHash.GetHex();
			j["SideGenesisHash"] = _sideGenesisHash.GetHex();
			j["BlockHeight"] = _blockHeight;
			j["SignedData"] = _signedData.getHex();

			return j;
		}

		void SideChainPow::FromJson(const nlohmann::json &j, uint8_t version) {
			_sideBlockHash.SetHex(j["SideBlockHash"].get<std::string>());
			_sideGenesisHash.SetHex(j["SideGenesisHash"].get<std::string>());
			_blockHeight = j["BlockHeight"].get<uint32_t>();
			_signedData.setHex(j["SignedData"].get<std::string>());
		}

		IPayload &SideChainPow::operator=(const IPayload &payload) {
			try {
				const SideChainPow &payloadSideMining = dynamic_cast<const SideChainPow &>(payload);
				operator=(payloadSideMining);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of SideChainPow");
			}

			return *this;
		}

		SideChainPow &SideChainPow::operator=(const SideChainPow &payload) {
			_sideBlockHash = payload._sideBlockHash;
			_sideGenesisHash = payload._sideGenesisHash;
			_blockHeight = payload._blockHeight;
			_signedData = payload._signedData;

			return *this;
		}

	}
}