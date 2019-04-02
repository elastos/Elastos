// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadTransferCrossChainAsset.h"
#include <SDK/Common/Log.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/WalletCore/BIPs/Key.h>
#include <SDK/WalletCore/BIPs/Address.h>

namespace Elastos {
	namespace ElaWallet {

		PayloadTransferCrossChainAsset::PayloadTransferCrossChainAsset() {

		}

		PayloadTransferCrossChainAsset::PayloadTransferCrossChainAsset(const PayloadTransferCrossChainAsset &payload) {
			operator=(payload);
		}

		PayloadTransferCrossChainAsset::PayloadTransferCrossChainAsset(
			const std::vector<std::string> &crossChainAddress,
			const std::vector<uint64_t> &outputIndex,
			const std::vector<uint64_t> &crossChainAmount) {
			SetCrossChainData(crossChainAddress, outputIndex, crossChainAmount);
		}

		PayloadTransferCrossChainAsset::~PayloadTransferCrossChainAsset() {

		}

		void PayloadTransferCrossChainAsset::SetCrossChainData(
			const std::vector<std::string> &crossChainAddress,
			const std::vector<uint64_t> &outputIndex,
			const std::vector<uint64_t> &crossChainAmount) {
			_crossChainAddress = crossChainAddress;
			_outputIndex = outputIndex;
			_crossChainAmount = crossChainAmount;
		}

		bool PayloadTransferCrossChainAsset::IsValid() const {
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

		const std::vector<std::string> &PayloadTransferCrossChainAsset::GetCrossChainAddress() const {
			return _crossChainAddress;
		}

		const std::vector<uint64_t> &PayloadTransferCrossChainAsset::GetOutputIndex() const {
			return _outputIndex;
		}

		const std::vector<uint64_t> &PayloadTransferCrossChainAsset::GetCrossChainAmout() const {
			return _crossChainAmount;
		}

		void PayloadTransferCrossChainAsset::Serialize(ByteStream &ostream, uint8_t version) const {
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

		bool PayloadTransferCrossChainAsset::Deserialize(const ByteStream &istream, uint8_t version) {
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

		nlohmann::json PayloadTransferCrossChainAsset::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["CrossChainAddress"] = _crossChainAddress;
			j["OutputIndex"] = _outputIndex;
			j["CrossChainAmount"] = _crossChainAmount;

			return j;
		}

		void PayloadTransferCrossChainAsset::FromJson(const nlohmann::json &j, uint8_t version) {
			_crossChainAddress = j["CrossChainAddress"].get<std::vector<std::string>>();
			_outputIndex = j["OutputIndex"].get<std::vector<uint64_t>>();
			_crossChainAmount = j["CrossChainAmount"].get<std::vector<uint64_t >>();
		}

		IPayload &PayloadTransferCrossChainAsset::operator=(const IPayload &payload) {
			try {
				const PayloadTransferCrossChainAsset &payloadTransferCrossChainAsset = dynamic_cast<const PayloadTransferCrossChainAsset&>(payload);
				operator=(payloadTransferCrossChainAsset);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadTransferCrossChainAsset");
			}

			return *this;
		}

		PayloadTransferCrossChainAsset &PayloadTransferCrossChainAsset::operator=(const PayloadTransferCrossChainAsset &payload) {
			_crossChainAddress = payload._crossChainAddress;
			_outputIndex = payload._outputIndex;
			_crossChainAmount = payload._crossChainAmount;

			return *this;
		}

	}
}