// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ReturnDepositCoin.h"
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
		ReturnDepositCoin::ReturnDepositCoin() {
		}

		ReturnDepositCoin::ReturnDepositCoin(const ReturnDepositCoin &payload) {
			operator=(payload);
		}

		ReturnDepositCoin::~ReturnDepositCoin() {
		}

		size_t ReturnDepositCoin::EstimateSize(uint8_t version) const {
			return 0;
		}

		void ReturnDepositCoin::Serialize(ByteStream &ostream, uint8_t version) const {

		}

		bool ReturnDepositCoin::Deserialize(const ByteStream &istream, uint8_t version) {
			return true;
		}

		nlohmann::json ReturnDepositCoin::ToJson(uint8_t version) const {
			return nlohmann::json();
		}

		void ReturnDepositCoin::FromJson(const nlohmann::json &j, uint8_t version) {

		}

		IPayload &ReturnDepositCoin::operator=(const IPayload &payload) {
			try {
				const ReturnDepositCoin &payloadReturnDepositCoin = dynamic_cast<const ReturnDepositCoin &>(payload);
				operator=(payloadReturnDepositCoin);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of ReturnDepositCoin");
			}

			return *this;
		}

		ReturnDepositCoin &ReturnDepositCoin::operator=(const ReturnDepositCoin &payload) {
			return *this;
		}

	}
}