// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadTransferAsset.h"
#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		PayloadTransferAsset::PayloadTransferAsset() {

		}

		PayloadTransferAsset::PayloadTransferAsset(const PayloadTransferAsset &payload) {
			operator=(payload);
		}

		PayloadTransferAsset::~PayloadTransferAsset() {

		}

		void PayloadTransferAsset::Serialize(ByteStream &ostream, uint8_t version) const {

		}

		bool PayloadTransferAsset::Deserialize(const ByteStream &istream, uint8_t version) {
			return true;
		}

		nlohmann::json PayloadTransferAsset::ToJson(uint8_t version) const {
			return nlohmann::json ();
		}

		void PayloadTransferAsset::FromJson(const nlohmann::json &j, uint8_t version) {

		}

		IPayload &PayloadTransferAsset::operator=(const IPayload &payload) {
			try {
				const PayloadTransferAsset &payloadTransferAsset = dynamic_cast<const PayloadTransferAsset&>(payload);
				operator=(payloadTransferAsset);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadTransferAsset");
			}

			return *this;
		}

		PayloadTransferAsset &PayloadTransferAsset::operator=(const PayloadTransferAsset &payload) {
			return *this;
		}

	}
}