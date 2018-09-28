// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRInt.h>
#include <SDK/Common/Log.h>
#include "Utils.h"
#include "Attribute.h"

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
			ostream.put(_usage);

			ostream.putVarUint(_data.GetSize());
			ostream.putBytes(_data, _data.GetSize());
		}

		bool Attribute::Deserialize(ByteStream &istream) {
			if (!istream.readBytes(&_usage, 1))
				return false;

//			if (!isValid()) {
//				Log::getLogger()->error("invalid attribute usage: {}", (uint8_t)_usage);
//				return false;
//			}

			return istream.readVarBytes(_data);
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