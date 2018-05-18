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
			//todo implement IPayload getData
			ByteStream stream;
			Serialize(stream);
			uint8_t *buf = stream.getBuf();
			uint64_t len = stream.length();
			CMBlock bd(len);
			memcpy(bd, buf, len);

			return bd;
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
			//blockHeight = istream.getVarUint();
			uint8_t heightData[32 / 8];
			istream.getBytes(heightData, (uint64_t)sizeof(heightData));
			_blockHeight = UInt32GetLE(heightData);

			uint64_t len = istream.getVarUint();
			if (0 < len) {
                char *utfBuffer = new char[len + 1];
                if (utfBuffer) {
                    istream.getBytes((uint8_t *) utfBuffer, len);
                    utfBuffer[len] = '\0';
                    _genesisBlockAddress = utfBuffer;
                    delete[] utfBuffer;
                }
            }

			len = istream.getVarUint();
			if (0 < len) {
                char *utfBuffer = new char[len + 1];
                if (utfBuffer) {
                    istream.getBytes((uint8_t *) utfBuffer, len);
                    utfBuffer[len] = '\0';
                    _sideChainTransactionHash = utfBuffer;
                    delete[] utfBuffer;
                }
            }
		}
	}
}