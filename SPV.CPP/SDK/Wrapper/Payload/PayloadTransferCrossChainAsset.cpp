// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadTransferCrossChainAsset.h"

namespace Elastos {
	namespace SDK {

		PayloadTransferCrossChainAsset::PayloadTransferCrossChainAsset() {

		}

		PayloadTransferCrossChainAsset::PayloadTransferCrossChainAsset(
				const std::map<std::string, uint64_t> addressMap) {
			_addressMap = addressMap;
		}

		PayloadTransferCrossChainAsset::~PayloadTransferCrossChainAsset() {

		}

		ByteData PayloadTransferCrossChainAsset::getData() const {
			//todo implement IPayload getData
			return ByteData(nullptr, 0);
		}

		void PayloadTransferCrossChainAsset::Serialize(ByteStream &ostream) const {
			uint64_t len = _addressMap.size();
			ostream.putVarUint(len);

			std::string key = "";
			uint64_t value = 0;
			std::map<std::string, uint64_t>::const_iterator it;
			for (it = _addressMap.begin(); it != _addressMap.end(); it++) {
				key = it->first;
				value = it->second;
				ostream.putVarUint(key.length());
				ostream.putBytes((uint8_t *) key.c_str(), key.length());

				ostream.putVarUint(value);
			}
		}

		void PayloadTransferCrossChainAsset::Deserialize(ByteStream &istream) {
			_addressMap.clear();
			uint64_t len = istream.getVarUint();

			std::string key = "";
			uint64_t value = 0;
			for (uint64_t i = 0; i < len; i++) {
				len = istream.getVarUint();
				char *utfBuffer = new char[len + 1];
				istream.getBytes((uint8_t *) utfBuffer, len);
				utfBuffer[len] = '\0';
				key = utfBuffer;
				delete[] utfBuffer;

				value = istream.getVarUint();
				_addressMap[key] = value;
			}
		}
	}
}