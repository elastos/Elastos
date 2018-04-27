// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadWithDrawAsset.h"
#include "BRInt.h"

namespace Elastos {
	namespace SDK {

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

		ByteData PayloadWithDrawAsset::getData() const {
			//todo implement IPayload getData
			return ByteData(nullptr, 0);
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

		void PayloadWithDrawAsset::Deserialize(ByteStream &istream) {
			_blockHeight = istream.getVarUint();

			uint64_t len = istream.getVarUint();
			char *utfBuffer = new char[len + 1];
			istream.getBytes((uint8_t *) utfBuffer, len);
			utfBuffer[len] = '\0';
			_genesisBlockAddress = utfBuffer;

			len = istream.getVarUint();
			istream.getBytes((uint8_t *) utfBuffer, len);
			utfBuffer[len] = '\0';
			_sideChainTransactionHash = utfBuffer;
			delete[] utfBuffer;
		}
	}
}