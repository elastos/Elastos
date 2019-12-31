// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "WithdrawFromSideChain.h"

#include <Common/Log.h>
#include <Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		WithdrawFromSideChain::WithdrawFromSideChain() :
				_blockHeight(0),
				_genesisBlockAddress("") {

		}

		WithdrawFromSideChain::WithdrawFromSideChain(const WithdrawFromSideChain &payload) {
			operator=(payload);
		}

		WithdrawFromSideChain::WithdrawFromSideChain(uint32_t blockHeight, const std::string &genesisBlockAddress,
		                                           const std::vector<uint256> &sideChainTransactionHash) {
			_blockHeight = blockHeight;
			_genesisBlockAddress = genesisBlockAddress;
			_sideChainTransactionHash = sideChainTransactionHash;
		}

		WithdrawFromSideChain::~WithdrawFromSideChain() {
		}

		void WithdrawFromSideChain::SetBlockHeight(uint32_t blockHeight) {
			_blockHeight = blockHeight;
		}

		uint32_t WithdrawFromSideChain::GetBlockHeight() const {
			return _blockHeight;
		}

		void WithdrawFromSideChain::SetGenesisBlockAddress(const std::string &genesisBlockAddress) {
			_genesisBlockAddress = genesisBlockAddress;
		}

		const std::string &WithdrawFromSideChain::GetGenesisBlockAddress() const {
			return _genesisBlockAddress;
		}

		void WithdrawFromSideChain::SetSideChainTransacitonHash(const std::vector<uint256> &sideChainTransactionHash) {
			_sideChainTransactionHash = sideChainTransactionHash;
		}

		const std::vector<uint256> &WithdrawFromSideChain::GetSideChainTransacitonHash() const {
			return _sideChainTransactionHash;
		}

		size_t WithdrawFromSideChain::EstimateSize(uint8_t version) const {
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

		void WithdrawFromSideChain::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteUint32(_blockHeight);
			ostream.WriteVarString(_genesisBlockAddress);
			ostream.WriteVarUint((uint64_t)_sideChainTransactionHash.size());

			for (size_t i = 0; i < _sideChainTransactionHash.size(); ++i) {
				ostream.WriteBytes(_sideChainTransactionHash[i]);
			}
		}

		bool WithdrawFromSideChain::Deserialize(const ByteStream &istream, uint8_t version) {
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

		nlohmann::json WithdrawFromSideChain::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["BlockHeight"] = _blockHeight;
			j["GenesisBlockAddress"] = _genesisBlockAddress;
			nlohmann::json hashes;
			for (size_t i = 0; i < _sideChainTransactionHash.size(); ++i)
				hashes.push_back(_sideChainTransactionHash[i].GetHex());
			j["SideChainTransactionHash"] = hashes;

			return j;
		}

		void WithdrawFromSideChain::FromJson(const nlohmann::json &j, uint8_t version) {
			_blockHeight = j["BlockHeight"].get<uint32_t>();
			_genesisBlockAddress = j["GenesisBlockAddress"].get<std::string>();

			nlohmann::json hashes = j["SideChainTransactionHash"];
			for (nlohmann::json::iterator it = hashes.begin(); it != hashes.end(); ++it)
				_sideChainTransactionHash.emplace_back((*it).get<std::string>());
		}

		IPayload &WithdrawFromSideChain::operator=(const IPayload &payload) {
			try {
				const WithdrawFromSideChain &payloadWithDrawAsset = dynamic_cast<const WithdrawFromSideChain &>(payload);
				operator=(payloadWithDrawAsset);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of WithdrawFromSideChain");
			}

			return *this;
		}

		WithdrawFromSideChain &WithdrawFromSideChain::operator=(const WithdrawFromSideChain &payload) {
			_blockHeight = payload._blockHeight;
			_genesisBlockAddress = payload._genesisBlockAddress;
			_sideChainTransactionHash = payload._sideChainTransactionHash;

			return *this;
		}

	}
}