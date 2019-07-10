// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransferCrossChainAsset.h"
#include <SDK/Common/Log.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/WalletCore/BIPs/Key.h>
#include <SDK/WalletCore/BIPs/Address.h>

namespace Elastos {
	namespace ElaWallet {

		TransferCrossChainAsset::TransferCrossChainAsset() {

		}

		TransferCrossChainAsset::TransferCrossChainAsset(const TransferCrossChainAsset &payload) {
			operator=(payload);
		}

		TransferCrossChainAsset::TransferCrossChainAsset(
			const std::vector<std::string> &crossChainAddress,
			const std::vector<uint64_t> &outputIndex,
			const std::vector<uint64_t> &crossChainAmount) {
			SetCrossChainData(crossChainAddress, outputIndex, crossChainAmount);
		}

		TransferCrossChainAsset::~TransferCrossChainAsset() {

		}

		void TransferCrossChainAsset::SetCrossChainData(
			const std::vector<std::string> &crossChainAddress,
			const std::vector<uint64_t> &outputIndex,
			const std::vector<uint64_t> &crossChainAmount) {
			_crossChainAddress = crossChainAddress;
			_outputIndex = outputIndex;
			_crossChainAmount = crossChainAmount;
		}

		bool TransferCrossChainAsset::IsValid() const {
			size_t len = _crossChainAddress.size();
			if (len <= 0 || len != _outputIndex.size() || len != _crossChainAmount.size()) {
				return false;
			}

			for (size_t i = 0; i < len; ++i) {
				if (!Address(_crossChainAddress[i]).Valid())
					return false;

				if (_crossChainAmount[i] <= 0) {
					return false;
				}
			}

			return true;
		}

		const std::vector<std::string> &TransferCrossChainAsset::GetCrossChainAddress() const {
			return _crossChainAddress;
		}

		const std::vector<uint64_t> &TransferCrossChainAsset::GetOutputIndex() const {
			return _outputIndex;
		}

		const std::vector<uint64_t> &TransferCrossChainAsset::GetCrossChainAmout() const {
			return _crossChainAmount;
		}

		size_t TransferCrossChainAsset::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			size += stream.WriteVarUint(_crossChainAddress.size());
			for (size_t i = 0; i < _crossChainAddress.size(); ++i) {
				size += stream.WriteVarUint(_crossChainAddress[i].size());
				size += _crossChainAddress[i].size();
				size += stream.WriteVarUint(_outputIndex[i]);
				size += sizeof(uint64_t);
			}

			return size;
		}

		void TransferCrossChainAsset::Serialize(ByteStream &ostream, uint8_t version) const {
			if (_crossChainAddress.size() != _outputIndex.size() || _outputIndex.size() != _crossChainAmount.size()) {
				Log::error("Invalid cross chain asset: len(crossChainAddress)={},"
							" len(outputIndex)={}, len(crossChainAddress)={}", _crossChainAddress.size(),
										_outputIndex.size(), _crossChainAmount.size());
				return ;
			}

			size_t len = _crossChainAddress.size();
			ostream.WriteVarUint((uint64_t)len);
			for (size_t i = 0; i < len; ++i) {
				ostream.WriteVarString(_crossChainAddress[i]);
				ostream.WriteVarUint(_outputIndex[i]);
				ostream.WriteUint64(_crossChainAmount[i]);
			}
		}

		bool TransferCrossChainAsset::Deserialize(const ByteStream &istream, uint8_t version) {
			uint64_t len = 0;
			if (!istream.ReadVarUint(len)) {
				Log::error("Payload transfer cross chain asset deserialize fail");
				return false;
			}

			_crossChainAddress.resize(len);
			_outputIndex.resize(len);
			_crossChainAmount.resize(len);

			for (uint64_t i = 0; i < len; ++i) {
				if (!istream.ReadVarString(_crossChainAddress[i])) {
					Log::error("Payload transfer cross chain asset deserialize cross chain address fail");
					return false;
				}

				if (!istream.ReadVarUint(_outputIndex[i])) {
					Log::error("Payload transfer cross chain asset deserialize output index fail");
					return false;
				}

				if (!istream.ReadUint64(_crossChainAmount[i])) {
					Log::error("Payload transfer cross chain asset deserialize cross chain amount fail");
					return false;
				}
			}

			return true;
		}

		nlohmann::json TransferCrossChainAsset::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["CrossChainAddress"] = _crossChainAddress;
			j["OutputIndex"] = _outputIndex;
			j["CrossChainAmount"] = _crossChainAmount;

			return j;
		}

		void TransferCrossChainAsset::FromJson(const nlohmann::json &j, uint8_t version) {
			_crossChainAddress = j["CrossChainAddress"].get<std::vector<std::string>>();
			_outputIndex = j["OutputIndex"].get<std::vector<uint64_t>>();
			_crossChainAmount = j["CrossChainAmount"].get<std::vector<uint64_t >>();
		}

		IPayload &TransferCrossChainAsset::operator=(const IPayload &payload) {
			try {
				const TransferCrossChainAsset &payloadTransferCrossChainAsset = dynamic_cast<const TransferCrossChainAsset&>(payload);
				operator=(payloadTransferCrossChainAsset);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of TransferCrossChainAsset");
			}

			return *this;
		}

		TransferCrossChainAsset &TransferCrossChainAsset::operator=(const TransferCrossChainAsset &payload) {
			_crossChainAddress = payload._crossChainAddress;
			_outputIndex = payload._outputIndex;
			_crossChainAmount = payload._crossChainAmount;

			return *this;
		}

	}
}