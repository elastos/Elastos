// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadWithDrawAsset.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		PayloadWithDrawAsset::PayloadWithDrawAsset() :
				_blockHeight(0),
				_genesisBlockAddress("") {

		}

		PayloadWithDrawAsset::PayloadWithDrawAsset(const PayloadWithDrawAsset &payload) {
			operator=(payload);
		}

		PayloadWithDrawAsset::PayloadWithDrawAsset(uint32_t blockHeight, const std::string &genesisBlockAddress,
		                                           const std::vector<uint256> &sideChainTransactionHash) {
			_blockHeight = blockHeight;
			_genesisBlockAddress = genesisBlockAddress;
			_sideChainTransactionHash = sideChainTransactionHash;
		}

		PayloadWithDrawAsset::~PayloadWithDrawAsset() {
		}

		void PayloadWithDrawAsset::SetBlockHeight(uint32_t blockHeight) {
			_blockHeight = blockHeight;
		}

		uint32_t PayloadWithDrawAsset::GetBlockHeight() const {
			return _blockHeight;
		}

		void PayloadWithDrawAsset::SetGenesisBlockAddress(const std::string &genesisBlockAddress) {
			_genesisBlockAddress = genesisBlockAddress;
		}

		const std::string &PayloadWithDrawAsset::GetGenesisBlockAddress() const {
			return _genesisBlockAddress;
		}

		void PayloadWithDrawAsset::SetSideChainTransacitonHash(const std::vector<uint256> &sideChainTransactionHash) {
			_sideChainTransactionHash = sideChainTransactionHash;
		}

		const std::vector<uint256> &PayloadWithDrawAsset::GetSideChainTransacitonHash() const {
			return _sideChainTransactionHash;
		}

		size_t PayloadWithDrawAsset::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			size += sizeof(_blockHeight);
			size += stream.WriteVarUint(_genesisBlockAddress.size());
			size += _genesisBlockAddress.size();
			size += stream.WriteVarUint(_sideChainTransactionHash.size());

			for (size_t i = 0; i < _sideChainTransactionHash.size(); ++i)
				size += _sideChainTransactionHash[i].size();

			return size;
		}

		void PayloadWithDrawAsset::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteUint32(_blockHeight);
			ostream.WriteVarString(_genesisBlockAddress);
			ostream.WriteVarUint((uint64_t)_sideChainTransactionHash.size());

			for (size_t i = 0; i < _sideChainTransactionHash.size(); ++i) {
				ostream.WriteBytes(_sideChainTransactionHash[i]);
			}
		}

		bool PayloadWithDrawAsset::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadUint32(_blockHeight)) {
				Log::error("Payload with draw asset deserialize block height fail");
				return false;
			}

			if (!istream.ReadVarString(_genesisBlockAddress)) {
				Log::error("Payload with draw asset deserialize genesis block address fail");
				return false;
			}

			uint64_t len = 0;
			if (!istream.ReadVarUint(len)) {
				Log::error("Payload with draw asset deserialize side chain tx hash len fail");
				return false;
			}

			_sideChainTransactionHash.resize(len);
			for (uint64_t i = 0; i < len; ++i) {
				if (!istream.ReadBytes(_sideChainTransactionHash[i])) {
					Log::error("Payload with draw asset deserialize side chain tx hash[{}] fail", i);
					return false;
				}
			}

			return true;
		}

		nlohmann::json PayloadWithDrawAsset::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["BlockHeight"] = _blockHeight;
			j["GenesisBlockAddress"] = _genesisBlockAddress;
			std::vector<std::string> hashes;
			for (size_t i = 0; i < _sideChainTransactionHash.size(); ++i) {
				hashes.push_back(_sideChainTransactionHash[i].GetHex());
			}
			j["SideChainTransactionHash"] = hashes;

			return j;
		}

		void PayloadWithDrawAsset::FromJson(const nlohmann::json &j, uint8_t version) {
			_blockHeight = j["BlockHeight"].get<uint32_t>();
			_genesisBlockAddress = j["GenesisBlockAddress"].get<std::string>();

			std::vector<std::string> hashes = j["SideChainTransactionHash"].get<std::vector<std::string>>();
			_sideChainTransactionHash.resize(hashes.size());
			for (size_t i = 0; i < hashes.size(); ++i) {
				_sideChainTransactionHash[i].SetHex(hashes[i]);
			}
		}

		IPayload &PayloadWithDrawAsset::operator=(const IPayload &payload) {
			try {
				const PayloadWithDrawAsset &payloadWithDrawAsset = dynamic_cast<const PayloadWithDrawAsset &>(payload);
				operator=(payloadWithDrawAsset);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadWithDrawAsset");
			}

			return *this;
		}

		PayloadWithDrawAsset &PayloadWithDrawAsset::operator=(const PayloadWithDrawAsset &payload) {
			_blockHeight = payload._blockHeight;
			_genesisBlockAddress = payload._genesisBlockAddress;
			_sideChainTransactionHash = payload._sideChainTransactionHash;

			return *this;
		}

	}
}