/*
 * Copyright (c) 2021 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Common/Log.h>
#include "PayloadCrossChain.h"

namespace Elastos {
	namespace ElaWallet {

		PayloadCrossChain::PayloadCrossChain() {

		}

		PayloadCrossChain::PayloadCrossChain(uint8_t version, const std::string &addr, const BigInt &amount, const bytes_t &data) :
			_version(version),
			_targetAddress(addr),
			_targetAmount(amount),
			_targetData(data) {
		}

		PayloadCrossChain::~PayloadCrossChain() {

		}

		uint8_t PayloadCrossChain::Version() const {
			return _version;
		}

		const std::string &PayloadCrossChain::TargetAddress() const {
			return _targetAddress;
		}

		const BigInt &PayloadCrossChain::TargetAmount() const {
			return _targetAmount;
		}

		const bytes_t &PayloadCrossChain::TargetData() const {
			return _targetData;
		}

		size_t PayloadCrossChain::EstimateSize() const {
			size_t size = 0;
			ByteStream stream;

			size += sizeof(_version);
			size += stream.WriteVarUint(_targetAddress.size());
			size += _targetAddress.size();
			size += sizeof(uint64_t);
			size += stream.WriteVarUint(_targetData.size());
			size += _targetData.size();

			return size;
		}

		void PayloadCrossChain::Serialize(ByteStream &stream) const {
			stream.WriteUint8(_version);
			stream.WriteVarString(_targetAddress);
			stream.WriteUint64(_targetAmount.getUint64());
			stream.WriteVarBytes(_targetData);
		}

		bool PayloadCrossChain::Deserialize(const ByteStream &stream) {
			if (!stream.ReadUint8(_version)) {
				Log::error("deser op version");
				return false;
			}

			if (!stream.ReadVarString(_targetAddress)) {
				Log::error("deser op address");
				return false;
			}

			uint64_t amount = 0;
			if (!stream.ReadUint64(amount)) {
				Log::error("deser op amount");
				return false;
			}
			_targetAmount.setUint64(amount);

			if (!stream.ReadVarBytes(_targetData)) {
				Log::error("deser op data");
				return false;
			}

			return true;
		}

		nlohmann::json PayloadCrossChain::ToJson() const {
			nlohmann::json j;
			j["Version"] = _version;
			j["TargetAddress"] = _targetAddress;
			j["TargetAmount"] = _targetAmount.getDec();
			j["TargetData"] = _targetData.getHex();
			return j;
		}

		void PayloadCrossChain::FromJson(const nlohmann::json &j) {
			_version = j["Version"];
			_targetAddress = j["TargetAddress"];
			_targetAmount.setDec(j["TargetAmount"].get<std::string>());
			_targetData.setHex(j["TargetData"].get<std::string>());
		}

		IOutputPayload &PayloadCrossChain::operator=(const IOutputPayload &payload) {
			try {
				const PayloadCrossChain &payloadCrossChain = dynamic_cast<const PayloadCrossChain &>(payload);
				operator=(payloadCrossChain);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadVote");
			}

			return *this;
		}

		PayloadCrossChain &PayloadCrossChain::operator=(const PayloadCrossChain &payload) {
			_version = payload._version;
			_targetAddress = payload._targetAddress;
			_targetAmount = payload._targetAmount;
			_targetData = payload._targetData;
			return *this;
		}

		bool PayloadCrossChain::operator==(const IOutputPayload &payload) const {
			try {
				const PayloadCrossChain &p = dynamic_cast<const PayloadCrossChain &>(payload);
				return _version == p._version &&
					   _targetAddress == p._targetAddress &&
					   _targetAmount == p._targetAmount &&
					   _targetData == p._targetData;
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadCrossChain");
			}

			return false;
		}
	}
}