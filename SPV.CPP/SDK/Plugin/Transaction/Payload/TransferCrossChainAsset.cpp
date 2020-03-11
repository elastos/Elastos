// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransferCrossChainAsset.h"
#include <Common/Log.h>
#include <WalletCore/Base58.h>
#include <WalletCore/Key.h>
#include <WalletCore/Address.h>

namespace Elastos {
	namespace ElaWallet {

		TransferInfo::TransferInfo() :
			_crossChainAddress(""), _outputIndex(0), _crossChainAmount(0) {

		}

		TransferInfo::TransferInfo(const std::string &address, uint16_t index, const BigInt &amount) :
			_crossChainAddress(address), _outputIndex(index), _crossChainAmount(amount) {
		}

		const std::string &TransferInfo::CrossChainAddress() const {
			return _crossChainAddress;
		}

		uint16_t TransferInfo::OutputIndex() const {
			return _outputIndex;
		}

		const BigInt &TransferInfo::CrossChainAmount() const {
			return _crossChainAmount;
		}

		nlohmann::json TransferInfo::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["CrossChainAddress"] = _crossChainAddress;
			j["OutputIndex"] = _outputIndex;
			j["CrossChainAmount"] = _crossChainAmount.getDec();

			return j;
		}

		void TransferInfo::FromJson(const nlohmann::json &j, uint8_t version) {
			_crossChainAddress = j["CrossChainAddress"].get<std::string>();
			_outputIndex = j["OutputIndex"].get<uint16_t>();
			_crossChainAmount.setDec(j["CrossChainAmount"].get<std::string>());
		}

		TransferCrossChainAsset::TransferCrossChainAsset() {

		}

		TransferCrossChainAsset::TransferCrossChainAsset(const TransferCrossChainAsset &payload) {
			this->operator=(payload);
		}

		TransferCrossChainAsset::TransferCrossChainAsset(const std::vector<TransferInfo> &info) :
			_info(info) {
		}

		TransferCrossChainAsset::~TransferCrossChainAsset() {

		}

		bool TransferCrossChainAsset::IsValid(uint8_t version) const {
			if (_info.empty())
				return false;

			for (size_t i = 0; i < _info.size(); ++i) {
				if (!Address(_info[i]._crossChainAddress).Valid())
					return false;

				if (_info[i]._crossChainAmount == 0)
					return false;
			}

			return true;
		}

		const std::vector<TransferInfo> &TransferCrossChainAsset::Info() const {
			return _info;
		}

		size_t TransferCrossChainAsset::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			size += stream.WriteVarUint(_info.size());
			for (size_t i = 0; i < _info.size(); ++i) {
				size += stream.WriteVarUint(_info[i]._crossChainAddress.size());
				size += _info[i]._crossChainAddress.size();
				size += stream.WriteVarUint(_info[i]._outputIndex);
				size += sizeof(_info[i]._outputIndex);
			}

			return size;
		}

		void TransferCrossChainAsset::Serialize(ByteStream &ostream, uint8_t version) const {
			size_t len = _info.size();
			ostream.WriteVarUint((uint64_t)len);
			for (size_t i = 0; i < _info.size(); ++i) {
				ostream.WriteVarString(_info[i]._crossChainAddress);
				ostream.WriteVarUint(_info[i]._outputIndex);
				ostream.WriteUint64(_info[i]._crossChainAmount.getUint64());
			}
		}

		bool TransferCrossChainAsset::Deserialize(const ByteStream &istream, uint8_t version) {
			uint64_t len = 0;
			if (!istream.ReadVarUint(len)) {
				Log::error("Payload transfer cross chain asset deserialize fail");
				return false;
			}

			TransferInfo info;
			for (uint64_t i = 0; i < len; ++i) {
				if (!istream.ReadVarString(info._crossChainAddress)) {
					Log::error("Payload transfer cross chain asset deserialize cross chain address fail");
					return false;
				}

				uint64_t index;
				if (!istream.ReadVarUint(index)) {
					Log::error("Payload transfer cross chain asset deserialize output index fail");
					return false;
				}
				info._outputIndex = (uint16_t) index;

				uint64_t amount;
				if (!istream.ReadUint64(amount)) {
					Log::error("Payload transfer cross chain asset deserialize cross chain amount fail");
					return false;
				}
				info._crossChainAmount.setUint64(amount);

				_info.push_back(info);
			}

			return true;
		}

		nlohmann::json TransferCrossChainAsset::ToJson(uint8_t version) const {
			nlohmann::json j;

			for (size_t i = 0; i < _info.size(); ++i) {
				j.push_back(_info[i].ToJson(version));
			}

			return j;
		}

		void TransferCrossChainAsset::FromJson(const nlohmann::json &j, uint8_t version) {
			if (!j.is_array()) {
				Log::error("cross chain info json should be array");
				return;
			}

			for (nlohmann::json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
				TransferInfo info;
				info.FromJson(*it, version);
				_info.push_back(info);
			}
		}

		IPayload &TransferCrossChainAsset::operator=(const IPayload &payload) {
			try {
				const TransferCrossChainAsset &payloadTransferCrossChainAsset = dynamic_cast<const TransferCrossChainAsset&>(payload);
				this->operator=(payloadTransferCrossChainAsset);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of TransferCrossChainAsset");
			}

			return *this;
		}

		TransferCrossChainAsset &TransferCrossChainAsset::operator=(const TransferCrossChainAsset &payload) {
			_info = payload._info;

			return *this;
		}

	}
}