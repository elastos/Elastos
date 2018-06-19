// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadTransferCrossChainAsset.h"
#include "Address.h"

namespace Elastos {
	namespace ElaWallet {

		PayloadTransferCrossChainAsset::PayloadTransferCrossChainAsset() {

		}

		PayloadTransferCrossChainAsset::PayloadTransferCrossChainAsset(const std::vector<std::string> crossChainAddress,
		                                                               const std::vector<uint64_t> outputIndex,
		                                                               const std::vector<uint64_t> crossChainAmount)
		{
			setCrossChainData(crossChainAddress, outputIndex, crossChainAmount);
		}

		PayloadTransferCrossChainAsset::~PayloadTransferCrossChainAsset() {

		}

		void PayloadTransferCrossChainAsset::setCrossChainData(const std::vector<std::string> crossChainAddress,
		                                                       const std::vector<uint64_t> outputIndex,
		                                                       const std::vector<uint64_t> crossChainAmount) {
			_crossChainAddress = crossChainAddress;
			_outputIndex = outputIndex;
			_crossChainAmount = crossChainAmount;
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

		bool PayloadTransferCrossChainAsset::isValid() const {
			size_t len = _crossChainAddress.size();
			if (len <= 0 || len != _outputIndex.size() || len != _crossChainAmount.size()) {
				return false;
			}

			for (size_t i = 0; i < len; ++i) {
				Address address(_crossChainAddress[i]);
				if (!address.isValid()) {
					return false;
				}

				if (_crossChainAmount[i] <= 0) {
					return false;
				}
			}

			return true;
		}

		const std::vector<std::string> &PayloadTransferCrossChainAsset::getCrossChainAddress() const {
			return _crossChainAddress;
		}

		const std::vector<uint64_t> &PayloadTransferCrossChainAsset::getOutputIndex() const {
			return _outputIndex;
		}

		const std::vector<uint64_t> &PayloadTransferCrossChainAsset::getCrossChainAmout() const {
			return _crossChainAmount;
		}

		void PayloadTransferCrossChainAsset::Serialize(ByteStream &ostream) const {
			size_t len = _crossChainAddress.size();
			ostream.putVarUint(len);
			for (size_t i = 0; i < len; ++i) {
				ostream.putVarUint(_crossChainAddress[i].length());
				ostream.putBytes((uint8_t *) _crossChainAddress[i].c_str(), _crossChainAddress[i].length());

				ostream.putVarUint(_outputIndex[i]);

				ostream.putUint64(_crossChainAmount[i]);
			}
		}

		bool PayloadTransferCrossChainAsset::Deserialize(ByteStream &istream) {
			_crossChainAddress.clear();
			_outputIndex.clear();
			_crossChainAmount.clear();
			uint64_t len = istream.getVarUint();

			for (size_t i = 0; i < len; ++i) {
				size_t size = istream.getVarUint();
				char *buff = new char[size + 1];
				if (buff) {
					istream.getBytes((uint8_t *)buff, size);
					buff[size] = '\0';
					std::string address = buff;
					_crossChainAddress.push_back(address);
					delete[] buff;
				}

				uint64_t index = istream.getVarUint();
				_outputIndex.push_back(index);

				uint64_t amount = istream.getUint64();
				_crossChainAmount.push_back(amount);

			}

			return true;
		}

		nlohmann::json PayloadTransferCrossChainAsset::toJson() {
			nlohmann::json jsonData;

			jsonData["crossChainAddress"] = _crossChainAddress;

			jsonData["outputIndex"] = _outputIndex;

			jsonData["crossChainAmount"] = _crossChainAmount;

			return jsonData;
		}

		void PayloadTransferCrossChainAsset::fromJson(const nlohmann::json &jsonData) {
			_crossChainAddress = jsonData["crossChainAddress"].get<std::vector<std::string>>();

			_outputIndex = jsonData["outputIndex"].get<std::vector<uint64_t>>();

			_crossChainAmount = jsonData["crossChainAmount"].get<std::vector<uint64_t >>();
		}
	}
}