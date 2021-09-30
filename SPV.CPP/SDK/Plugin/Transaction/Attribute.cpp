// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Attribute.h"

#include <Common/Log.h>
#include <Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		Attribute::Attribute() :
			_usage(Nonce) {
		}

		Attribute::Attribute(const Attribute &attr) {
			operator=(attr);
		}

		Attribute::Attribute(Attribute::Usage usage, const bytes_t &data) :
			_usage(usage),
			_data(data) {

		}

		Attribute::~Attribute() {
		}

		Attribute& Attribute::operator=(const Attribute &attr) {
			this->_usage = attr._usage;
			this->_data = attr._data;
			return *this;
		}

		Attribute::Usage Attribute::GetUsage() const {
			return _usage;
		}

		const bytes_t &Attribute::GetData() const {
			return _data;
		}

		bool Attribute::IsValid() const {
			return (_usage == Attribute::Usage::Description ||
					_usage == Attribute::Usage::DescriptionUrl ||
					_usage == Attribute::Usage::Memo ||
					_usage == Attribute::Usage::Script ||
					_usage == Attribute::Usage::Nonce ||
					_usage == Attribute::Usage::Confirmations);
		}

		size_t Attribute::EstimateSize() const {
			size_t size = 0;
			ByteStream stream;

			size += 1;
			size += stream.WriteVarUint(_data.size());
			size += _data.size();

			return size;
		}

		void Attribute::Serialize(ByteStream &stream) const {
			stream.WriteUint8(_usage);
			stream.WriteVarBytes(_data);
		}

		bool Attribute::Deserialize(const ByteStream &stream) {
			if (!stream.ReadBytes(&_usage, 1)) {
				Log::error("Attribute deserialize usage fail");
				return false;
			}

			if (!IsValid()) {
				Log::error("invalid attribute usage: {}", (uint8_t)_usage);
				return false;
			}

			if (!stream.ReadVarBytes(_data)) {
				Log::error("Attribute deserialize data fail");
				return false;
			}

			return true;
		}

		nlohmann::json Attribute::ToJson() const {
			nlohmann::json j;
			j["Usage"] = _usage;
			j["Data"] = _data.getHex();

			return j;
		}

		void Attribute::FromJson(const nlohmann::json &j) {
			_usage = j["Usage"].get<Usage>();
			_data.setHex(j["Data"].get<std::string>());
		}

		bool Attribute::operator==(const Attribute &a) const {
			return _usage == a._usage && _data == a._data;
		}

		bool Attribute::operator!=(const Attribute &a) const {
			return !operator==(a);
		}

	}
}