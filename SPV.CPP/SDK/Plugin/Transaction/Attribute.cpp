// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Attribute.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

#include <Core/BRInt.h>

namespace Elastos {
	namespace ElaWallet {

		Attribute::Attribute() :
			_usage(Nonce) {
		}

		Attribute::Attribute(const Attribute &attr) {
			this->_usage = attr._usage;
			this->_data.Resize(attr._data.GetSize());
			memcpy(this->_data, attr._data, attr._data.GetSize());
		}

		Attribute::Attribute(Attribute::Usage usage, const CMBlock &data) :
			_usage(usage),
			_data(data) {

		}

		Attribute::~Attribute() {
		}

		Attribute::Usage Attribute::GetUsage() const {
			return _usage;
		}

		const CMBlock &Attribute::GetData() const {
			return _data;
		}

		bool Attribute::isValid() const {
			return (_usage == Attribute::Usage::Description ||
					_usage == Attribute::Usage::DescriptionUrl ||
					_usage == Attribute::Usage::Memo ||
					_usage == Attribute::Usage::Script ||
					_usage == Attribute::Usage::Nonce ||
					_usage == Attribute::Usage::Confirmations);
		}

		void Attribute::Serialize(ByteStream &ostream) const {
			ostream.writeUint8(_usage);
			ostream.writeVarBytes(_data);
		}

		bool Attribute::Deserialize(ByteStream &istream) {
			if (!istream.readBytes(&_usage, 1)) {
				Log::error("Attribute deserialize usage fail");
				return false;
			}

//			if (!isValid()) {
//				Log::error("invalid attribute usage: {}", (uint8_t)_usage);
//				return false;
//			}

			if (!istream.readVarBytes(_data)) {
				Log::error("Attribute deserialize data fail");
				return false;
			}

			return true;
		}

		nlohmann::json Attribute::toJson() const {
			nlohmann::json jsonData;
			jsonData["Usage"] = _usage;
			jsonData["Data"] = Utils::encodeHex(_data);

			return jsonData;
		}

		void Attribute::fromJson(const nlohmann::json &jsonData) {
			_usage = jsonData["Usage"].get<Usage>();
			_data = Utils::decodeHex(jsonData["Data"].get<std::string>());
		}
	}
}