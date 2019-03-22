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

		PayloadRechargeToSideChain::PayloadRechargeToSideChain(const bytes_t &merkeProff, const bytes_t &mainChainTransaction) {
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
				ostream.WriteVarBytes(_merkeProof);
				ostream.WriteVarBytes(_mainChainTransaction);
			} else if (version == PayloadRechargeToSideChain::V1) {
				ostream.WriteBytes(_mainChainTxHash);
			} else {
				Log::error("Serialize: invalid recharge to side chain payload version = {}", version);
			}
		}

		bool PayloadRechargeToSideChain::Deserialize(const ByteStream &istream, uint8_t version) {
			if (version == PayloadRechargeToSideChain::V0) {
				if (!istream.ReadVarBytes(_merkeProof)) {
					Log::error("Deserialize: recharge to side chain payload read merkle proof");
					return false;
				}

				if (!istream.ReadVarBytes(_mainChainTransaction)) {
					Log::error("Deserialize: recharge to side chain payload read tx");
					return false;
				}
			} else if (version == PayloadRechargeToSideChain::V1) {
				if (!istream.ReadBytes(_mainChainTxHash)) {
					Log::error("Deserialize: recharge to side chain payload read tx hash");
					return false;
				}
			} else {
				Log::error("Deserialize: invalid recharge to side chain payload versin = {}", version);
				return false;
			}

			return true;
		}

		nlohmann::json PayloadRechargeToSideChain::ToJson(uint8_t version) const {
			nlohmann::json j;

			if (version == PayloadRechargeToSideChain::V0) {
				j["MerkleProof"] = _merkeProof.getHex();
				j["MainChainTransaction"] = _mainChainTransaction.getHex();
			} else if (version == PayloadRechargeToSideChain::V1) {
				j["MainChaianTxHash"] = _mainChainTxHash.GetHex();
			} else {
				Log::error("toJson: invalid recharge to side chain payload version = {}", version);
			}

			return j;
		}

		void PayloadRechargeToSideChain::FromJson(const nlohmann::json &j, uint8_t version) {
			if (version == PayloadRechargeToSideChain::V0) {
				_merkeProof.setHex(j["MerkleProof"].get<std::string>());
				_mainChainTransaction.setHex(j["MainChainTransaction"].get<std::string>());
			} else if (version == PayloadRechargeToSideChain::V1) {
				_mainChainTxHash.SetHex(j["MainChainTxHash"].get<std::string>());
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
			_merkeProof = payload._merkeProof;
			_mainChainTransaction = payload._mainChainTransaction;
			_mainChainTxHash = payload._mainChainTxHash;

			return *this;
		}

	}
}