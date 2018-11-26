// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadWithDrawAsset.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <Core/BRInt.h>

namespace Elastos {
	namespace ElaWallet {

		PayloadWithDrawAsset::PayloadWithDrawAsset() :
				_blockHeight(0),
				_genesisBlockAddress("") {

		}

		PayloadWithDrawAsset::PayloadWithDrawAsset(uint32_t blockHeight, const std::string &genesisBlockAddress,
		                                           const std::vector<UInt256> &sideChainTransactionHash) {
			_blockHeight = blockHeight;
			_genesisBlockAddress = genesisBlockAddress;
			_sideChainTransactionHash = sideChainTransactionHash;
		}

		PayloadWithDrawAsset::~PayloadWithDrawAsset() {
		}

		void PayloadWithDrawAsset::setBlockHeight(uint32_t blockHeight) {
			_blockHeight = blockHeight;
		}

		uint32_t PayloadWithDrawAsset::getBlockHeight() const {
			return _blockHeight;
		}

		void PayloadWithDrawAsset::setGenesisBlockAddress(const std::string &genesisBlockAddress) {
			_genesisBlockAddress = genesisBlockAddress;
		}

		const std::string &PayloadWithDrawAsset::getGenesisBlockAddress() const {
			return _genesisBlockAddress;
		}

		void PayloadWithDrawAsset::setSideChainTransacitonHash(const std::vector<UInt256> &sideChainTransactionHash) {
			_sideChainTransactionHash = sideChainTransactionHash;
		}

		const std::vector<UInt256> &PayloadWithDrawAsset::getSideChainTransacitonHash() const {
			return _sideChainTransactionHash;
		}

		CMBlock PayloadWithDrawAsset::getData() const {
			ByteStream stream;
			Serialize(stream);
			return stream.getBuffer();
		}

		void PayloadWithDrawAsset::Serialize(ByteStream &ostream) const {
			ostream.writeUint32(_blockHeight);
			ostream.writeVarString(_genesisBlockAddress);
			ostream.writeVarUint((uint64_t)_sideChainTransactionHash.size());

			for (size_t i = 0; i < _sideChainTransactionHash.size(); ++i) {
				ostream.writeBytes(_sideChainTransactionHash[i].u8, sizeof(UInt256));
			}
		}

		bool PayloadWithDrawAsset::Deserialize(ByteStream &istream) {
			if (!istream.readUint32(_blockHeight)) {
				Log::error("Payload with draw asset deserialize block height fail");
				return false;
			}

			if (!istream.readVarString(_genesisBlockAddress)) {
				Log::error("Payload with draw asset deserialize genesis block address fail");
				return false;
			}

			uint64_t len = 0;
			if (!istream.readVarUint(len)) {
				Log::error("Payload with draw asset deserialize side chain tx hash len fail");
				return false;
			}

			_sideChainTransactionHash.resize(len);
			for (uint64_t i = 0; i < len; ++i) {
				if (!istream.readBytes(_sideChainTransactionHash[i].u8, sizeof(UInt256))) {
					Log::error("Payload with draw asset deserialize side chain tx hash[{}] fail", i);
					return false;
				}
			}

			return true;
		}

		nlohmann::json PayloadWithDrawAsset::toJson() const {
			nlohmann::json j;

			j["BlockHeight"] = _blockHeight;
			j["GenesisBlockAddress"] = _genesisBlockAddress;
			std::vector<std::string> hashes;
			for (size_t i = 0; i < _sideChainTransactionHash.size(); ++i) {
				std::string str = Utils::UInt256ToString(_sideChainTransactionHash[i]);
				hashes.push_back(str);
			}
			j["SideChainTransactionHash"] = hashes;

			return j;
		}

		void PayloadWithDrawAsset::fromJson(const nlohmann::json &j) {
			_blockHeight = j["BlockHeight"].get<uint32_t>();
			_genesisBlockAddress = j["GenesisBlockAddress"].get<std::string>();

			std::vector<std::string> hashes = j["SideChainTransactionHash"].get<std::vector<std::string>>();
			_sideChainTransactionHash.resize(hashes.size());
			for (size_t i = 0; i < hashes.size(); ++i) {
				_sideChainTransactionHash[i] = Utils::UInt256FromString(hashes[i]);
			}
		}
	}
}