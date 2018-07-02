// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/Log.h>
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
			return stream.getBuffer();
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
			if (_crossChainAddress.size() != _outputIndex.size() || _outputIndex.size() != _crossChainAmount.size()) {
				Log::getLogger()->error("Invalid cross chain asset: len(crossChainAddress)={},"
							" len(outputIndex)={}, len(crossChainAddress)={}", _crossChainAddress.size(),
										_outputIndex.size(), _crossChainAmount.size());
				return ;
			}

			size_t len = _crossChainAddress.size();
			ostream.writeVarUint((uint64_t)len);
			for (size_t i = 0; i < len; ++i) {
				ostream.writeVarString(_crossChainAddress[i]);
				ostream.writeVarUint(_outputIndex[i]);
				ostream.writeUint64(_crossChainAmount[i]);
			}
		}

		bool PayloadTransferCrossChainAsset::Deserialize(ByteStream &istream) {
			uint64_t len = 0;
			if (!istream.readVarUint(len)) {
				Log::error("Payload transfer cross chain asset deserialize fail");
				return false;
			}

			_crossChainAddress.resize(len);
			_outputIndex.resize(len);
			_crossChainAmount.resize(len);

			for (uint64_t i = 0; i < len; ++i) {
				if (!istream.readVarString(_crossChainAddress[i])) {
					Log::error("Payload transfer cross chain asset deserialize cross chain address fail");
					return false;
				}

				if (!istream.readVarUint(_outputIndex[i])) {
					Log::error("Payload transfer cross chain asset deserialize output index fail");
					return false;
				}

				if (!istream.readUint64(_crossChainAmount[i])) {
					Log::error("Payload transfer cross chain asset deserialize cross chain amount fail");
					return false;
				}
			}

			return true;
		}

		nlohmann::json PayloadTransferCrossChainAsset::toJson() const {
			nlohmann::json j;

			j["CrossChainAddress"] = _crossChainAddress;
			j["OutputIndex"] = _outputIndex;
			j["CrossChainAmount"] = _crossChainAmount;

			return j;
		}

		void PayloadTransferCrossChainAsset::fromJson(const nlohmann::json &j) {
			_crossChainAddress = j["CrossChainAddress"].get<std::vector<std::string>>();
			_outputIndex = j["OutputIndex"].get<std::vector<uint64_t>>();
			_crossChainAmount = j["CrossChainAmount"].get<std::vector<uint64_t >>();
		}
	}
}