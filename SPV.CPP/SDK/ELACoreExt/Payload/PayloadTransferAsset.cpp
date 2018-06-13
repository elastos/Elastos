// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadTransferAsset.h"

namespace Elastos {
	namespace SDK {

		PayloadTransferAsset::PayloadTransferAsset() {

		}

		PayloadTransferAsset::~PayloadTransferAsset() {

		}

		CMBlock PayloadTransferAsset::getData() const {
			//todo implement IPayload getData
			return CMBlock();
		}

		void PayloadTransferAsset::Serialize(ByteStream &ostream) const {

		}

		bool PayloadTransferAsset::Deserialize(ByteStream &istream) {
			return true;
		}

		nlohmann::json PayloadTransferAsset::toJson() {
			nlohmann::json jsonData;
			return jsonData;
		}

		void PayloadTransferAsset::fromJson(const nlohmann::json &jsonData) {

		}
	}
}