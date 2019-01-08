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
			if (version == PayloadRechargeToSideChain::V0) {
				ostream.writeVarBytes(_merkeProof);
				ostream.writeVarBytes(_mainChainTransaction);
			} else if (version == PayloadRechargeToSideChain::V1) {
				ostream.writeBytes(&_mainChainTxHash, sizeof(_mainChainTxHash));
			} else {
				Log::error("Serialize: invalid recharge to side chain payload version = {}", version);
			}
		}

		bool PayloadRechargeToSideChain::Deserialize(ByteStream &istream, uint8_t version) {
			if (version == PayloadRechargeToSideChain::V0) {
				if (!istream.readVarBytes(_merkeProof)) {
					Log::error("Deserialize: recharge to side chain payload read merkle proof");
					return false;
				}

				if (!istream.readVarBytes(_mainChainTransaction)) {
					Log::error("Deserialize: recharge to side chain payload read tx");
					return false;
				}
			} else if (version == PayloadRechargeToSideChain::V1) {
				if (!istream.readBytes(&_mainChainTxHash, sizeof(_mainChainTxHash))) {
					Log::error("Deserialize: recharge to side chain payload read tx hash");
					return false;
				}
			} else {
				Log::error("Deserialize: invalid recharge to side chain payload versin = {}", version);
				return false;
			}

			return true;
		}

		nlohmann::json PayloadRechargeToSideChain::toJson(uint8_t version) const {
			nlohmann::json j;

			if (version == PayloadRechargeToSideChain::V0) {
				j["MerkleProof"] = Utils::encodeHex(_merkeProof);
				j["MainChainTransaction"] = Utils::encodeHex(_mainChainTransaction);
			} else if (version == PayloadRechargeToSideChain::V1) {
				j["MainChaianTxHash"] = Utils::UInt256ToString(_mainChainTxHash, true);
			} else {
				Log::error("toJson: invalid recharge to side chain payload version = {}", version);
			}

			return j;
		}

		void PayloadRechargeToSideChain::fromJson(const nlohmann::json &j, uint8_t version) {
			if (version == PayloadRechargeToSideChain::V0) {
				_merkeProof = Utils::decodeHex(j["MerkleProof"].get<std::string>());
				_mainChainTransaction = Utils::decodeHex(j["MainChainTransaction"].get<std::string>());
			} else if (version == PayloadRechargeToSideChain::V1) {
				_mainChainTxHash = Utils::UInt256FromString(j["MainChainTxHash"].get<std::string>(), true);
			} else {
				Log::error("fromJson: invalid recharge to side chain payload version = {}", version);
			}
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
			_mainChainTxHash = payload._mainChainTxHash;

			return *this;
		}

	}
}