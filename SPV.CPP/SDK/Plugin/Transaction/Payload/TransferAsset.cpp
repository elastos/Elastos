// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransferAsset.h"
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		TransferAsset::TransferAsset() {

		}

		TransferAsset::TransferAsset(const TransferAsset &payload) {
			operator=(payload);
		}

		TransferAsset::~TransferAsset() {

		}

		size_t TransferAsset::EstimateSize(uint8_t version) const {
			return 0;
		}

		void TransferAsset::Serialize(ByteStream &ostream, uint8_t version) const {

		}

		bool TransferAsset::Deserialize(const ByteStream &istream, uint8_t version) {
			return true;
		}

		nlohmann::json TransferAsset::ToJson(uint8_t version) const {
			return nlohmann::json ();
		}

		void TransferAsset::FromJson(const nlohmann::json &j, uint8_t version) {

		}

		IPayload &TransferAsset::operator=(const IPayload &payload) {
			try {
				const TransferAsset &payloadTransferAsset = dynamic_cast<const TransferAsset&>(payload);
				operator=(payloadTransferAsset);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of TransferAsset");
			}

			return *this;
		}

		TransferAsset &TransferAsset::operator=(const TransferAsset &payload) {
			return *this;
		}

	}
}