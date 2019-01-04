// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadRechargeToSideChain.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

#include <cstring>

namespace Elastos {
	namespace ElaWallet {

		PayloadRechargeToSideChain::PayloadRechargeToSideChain() {

		}

		PayloadRechargeToSideChain::PayloadRechargeToSideChain(const CMBlock &merkeProff, const CMBlock &mainChainTransaction) {
			_merkeProof = merkeProff;
			_mainChainTransaction = mainChainTransaction;
		}

		PayloadRechargeToSideChain::PayloadRechargeToSideChain(const PayloadRechargeToSideChain &payload) {
			operator=(payload);
		}

		PayloadRechargeToSideChain::~PayloadRechargeToSideChain() {
		}

		void PayloadRechargeToSideChain::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.writeVarBytes(_merkeProof);
			ostream.writeVarBytes(_mainChainTransaction);
		}

		bool PayloadRechargeToSideChain::Deserialize(ByteStream &istream, uint8_t version) {
			if (!istream.readVarBytes(_merkeProof)) {
				Log::error("PayloadRechargeToSideChain deserialize merke proof error");
				return false;
			}

			if (!istream.readVarBytes(_mainChainTransaction)) {
				Log::error("PayloadRechargeToSideChain deserialize main chain transaction error");
				return false;
			}

			return true;
		}

		nlohmann::json PayloadRechargeToSideChain::toJson() const {
			nlohmann::json j;

			j["MerkleProof"] = Utils::encodeHex(_merkeProof);
			j["MainChainTransaction"] = Utils::encodeHex(_mainChainTransaction);

			return j;
		}

		void PayloadRechargeToSideChain::fromJson(const nlohmann::json &j) {
			_merkeProof = Utils::decodeHex(j["MerkleProof"].get<std::string>());
			_mainChainTransaction = Utils::decodeHex(j["MainChainTransaction"].get<std::string>());
		}

		IPayload &PayloadRechargeToSideChain::operator=(const IPayload &payload) {
			try {
				const PayloadRechargeToSideChain &payloadRecharge = dynamic_cast<const PayloadRechargeToSideChain &>(payload);
				operator=(payloadRecharge);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadRechargeToSideChain");
			}

			return *this;
		}

		PayloadRechargeToSideChain &PayloadRechargeToSideChain::operator=(const PayloadRechargeToSideChain &payload) {
			_merkeProof.Memcpy(payload._merkeProof);
			_mainChainTransaction.Memcpy(payload._mainChainTransaction);

			return *this;
		}

	}
}