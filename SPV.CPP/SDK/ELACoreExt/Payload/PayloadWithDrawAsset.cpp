// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadWithDrawAsset.h"
#include "BRInt.h"

namespace Elastos {
	namespace ElaWallet {

		PayloadWithDrawAsset::PayloadWithDrawAsset() :
				_blockHeight(0),
				_genesisBlockAddress(""),
				_sideChainTransactionHash("") {

		}

		PayloadWithDrawAsset::PayloadWithDrawAsset(const uint32_t blockHeight, const std::string genesisBlockAddress,
		                                           const std::string sideChainTransactionHash) {
			_blockHeight = blockHeight;
			_genesisBlockAddress = genesisBlockAddress;
			_sideChainTransactionHash = sideChainTransactionHash;
		}

		PayloadWithDrawAsset::~PayloadWithDrawAsset() {
		}

		void PayloadWithDrawAsset::setBlockHeight(const uint32_t blockHeight) {
			_blockHeight = blockHeight;
		}

		void PayloadWithDrawAsset::setGenesisBlockAddress(const std::string genesisBlockAddress) {
			_genesisBlockAddress = genesisBlockAddress;
		}

		void PayloadWithDrawAsset::setSideChainTransacitonHash(const std::string sideChainTransactionHash) {
			_sideChainTransactionHash = sideChainTransactionHash;
		}

		CMBlock PayloadWithDrawAsset::getData() const {
			ByteStream stream;
			Serialize(stream);
			return stream.getBuffer();
		}

		void PayloadWithDrawAsset::Serialize(ByteStream &ostream) const {
			uint8_t heightData[32 / 8];
			UInt32SetLE(heightData, _blockHeight);
			ostream.putBytes(heightData, sizeof(heightData));

			uint64_t len = _genesisBlockAddress.length();
			ostream.putVarUint(len);
			ostream.putBytes((uint8_t *) _genesisBlockAddress.c_str(), len);

			len = _sideChainTransactionHash.length();
			ostream.putVarUint(len);
			ostream.putBytes((uint8_t *) _sideChainTransactionHash.c_str(), len);
		}

		bool PayloadWithDrawAsset::Deserialize(ByteStream &istream) {
			//blockHeight = istream.getVarUint();
			uint8_t heightData[32 / 8];
			istream.getBytes(heightData, (uint64_t)sizeof(heightData));
			_blockHeight = UInt32GetLE(heightData);

			uint64_t len = istream.getVarUint();
			char addr[len + 1];
			istream.getBytes((uint8_t *) addr, len);
			addr[len] = '\0';
			_genesisBlockAddress = std::string(addr);

			len = istream.getVarUint();
			char hash[len + 1];
			istream.getBytes((uint8_t *) hash, len);
			hash[len] = '\0';
			_sideChainTransactionHash = std::string(hash);

			return true;
		}

		nlohmann::json PayloadWithDrawAsset::toJson() {
			nlohmann::json jsonData;

			jsonData["BlockHeight"] = _blockHeight;

			jsonData["GenesisBlockAddress"] = _genesisBlockAddress;

			jsonData["SideChainTransactionHash"] = _sideChainTransactionHash;

			return jsonData;
		}

		void PayloadWithDrawAsset::fromJson(const nlohmann::json &jsonData) {
			_blockHeight = jsonData["BlockHeight"];

			_genesisBlockAddress = jsonData["GenesisBlockAddress"];

			_sideChainTransactionHash = jsonData["SideChainTransactionHash"];
		}
	}
}