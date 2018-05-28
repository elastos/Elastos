// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadTransferCrossChainAsset.h"

namespace Elastos {
	namespace SDK {

		PayloadTransferCrossChainAsset::PayloadTransferCrossChainAsset() {

		}

		PayloadTransferCrossChainAsset::PayloadTransferCrossChainAsset(
				const std::map<std::string, uint64_t> &addressMap) {
			_addressMap = addressMap;
		}

		PayloadTransferCrossChainAsset::~PayloadTransferCrossChainAsset() {

		}

		void PayloadTransferCrossChainAsset::setAddressMap(const std::map<std::string, uint64_t> &addressMap) {
			_addressMap = addressMap;

		}

		CMBlock PayloadTransferCrossChainAsset::getData() const {
			//todo implement IPayload getData
			ByteStream stream;
			Serialize(stream);
			uint8_t* buf = stream.getBuf();
			uint64_t len = stream.length();
			CMBlock db(len);
			memcpy(db, buf, len);

			return db;
		}

		void PayloadTransferCrossChainAsset::Serialize(ByteStream &ostream) const {
			uint64_t len = _addressMap.size();
			ostream.putVarUint(len);

			std::string key = "";
			uint64_t value = 0;
			std::map<std::string, uint64_t>::const_iterator it;
			for (it = _addressMap.begin(); it != _addressMap.end(); it++) {
				key = 	it->first;
				value = it->second;
				ostream.putVarUint(key.length());
				ostream.putBytes((uint8_t *) key.c_str(), key.length());

				ostream.putVarUint(value);
			}
		}

		void PayloadTransferCrossChainAsset::Deserialize(ByteStream &istream) {
			_addressMap.clear();
			uint64_t len = istream.getVarUint(), _len;

			if (0 < len) {
                std::string key = "";
                uint64_t value = 0;
                for (uint64_t i = 0; i < len; i++) {
                    _len = istream.getVarUint();
                    if (0 < _len) {
                        char *utfBuffer = new char[_len + 1];
                        if (utfBuffer) {
                            istream.getBytes((uint8_t *) utfBuffer, _len);
                            utfBuffer[_len] = '\0';
                            key = utfBuffer;
                            delete[] utfBuffer;
                        }
                    }

                    value = istream.getVarUint();
                    _addressMap[key] = value;
                }
            }
		}

		nlohmann::json PayloadTransferCrossChainAsset::toJson() {
			nlohmann::json jsonData(_addressMap);
			return jsonData;
		}

		void PayloadTransferCrossChainAsset::fromJson(const nlohmann::json jsonData) {
			for (nlohmann::json::const_iterator it = jsonData.cbegin(); it != jsonData.cend(); ++it) {
				_addressMap[it.key()] = it.value();
			}
		}
	}
}