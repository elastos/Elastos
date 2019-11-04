// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "RechargeToSideChain.h"

#include <Common/Log.h>
#include <Common/Utils.h>

#include <cstring>

namespace Elastos {
	namespace ElaWallet {

		RechargeToSideChain::RechargeToSideChain() {

		}

		RechargeToSideChain::RechargeToSideChain(const bytes_t &merkeProff, const bytes_t &mainChainTransaction) {
			_merkeProof = merkeProff;
			_mainChainTransaction = mainChainTransaction;
		}

		RechargeToSideChain::RechargeToSideChain(const RechargeToSideChain &payload) {
			operator=(payload);
		}

		RechargeToSideChain::~RechargeToSideChain() {
		}

		size_t RechargeToSideChain::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			if (version == RechargeToSideChain::V0) {
				size += stream.WriteVarUint(_merkeProof.size());
				size += _merkeProof.size();
				size += stream.WriteVarUint(_mainChainTransaction.size());
				size += _mainChainTransaction.size();
			} else if (version == RechargeToSideChain::V1) {
				size += _mainChainTxHash.size();
			}

			return size;
		}

		void RechargeToSideChain::Serialize(ByteStream &ostream, uint8_t version) const {
			if (version == RechargeToSideChain::V0) {
				ostream.WriteVarBytes(_merkeProof);
				ostream.WriteVarBytes(_mainChainTransaction);
			} else if (version == RechargeToSideChain::V1) {
				ostream.WriteBytes(_mainChainTxHash);
			} else {
				Log::error("Serialize: invalid recharge to side chain payload version = {}", version);
			}
		}

		bool RechargeToSideChain::Deserialize(const ByteStream &istream, uint8_t version) {
			if (version == RechargeToSideChain::V0) {
				if (!istream.ReadVarBytes(_merkeProof)) {
					Log::error("Deserialize: recharge to side chain payload read merkle proof");
					return false;
				}

				if (!istream.ReadVarBytes(_mainChainTransaction)) {
					Log::error("Deserialize: recharge to side chain payload read tx");
					return false;
				}
			} else if (version == RechargeToSideChain::V1) {
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

		nlohmann::json RechargeToSideChain::ToJson(uint8_t version) const {
			nlohmann::json j;

			if (version == RechargeToSideChain::V0) {
				j["MerkleProof"] = _merkeProof.getHex();
				j["MainChainTransaction"] = _mainChainTransaction.getHex();
			} else if (version == RechargeToSideChain::V1) {
				j["MainChaianTxHash"] = _mainChainTxHash.GetHex();
			} else {
				Log::error("toJson: invalid recharge to side chain payload version = {}", version);
			}

			return j;
		}

		void RechargeToSideChain::FromJson(const nlohmann::json &j, uint8_t version) {
			if (version == RechargeToSideChain::V0) {
				_merkeProof.setHex(j["MerkleProof"].get<std::string>());
				_mainChainTransaction.setHex(j["MainChainTransaction"].get<std::string>());
			} else if (version == RechargeToSideChain::V1) {
				_mainChainTxHash.SetHex(j["MainChainTxHash"].get<std::string>());
			} else {
				Log::error("fromJson: invalid recharge to side chain payload version = {}", version);
			}
		}

		IPayload &RechargeToSideChain::operator=(const IPayload &payload) {
			try {
				const RechargeToSideChain &payloadRecharge = dynamic_cast<const RechargeToSideChain &>(payload);
				operator=(payloadRecharge);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of RechargeToSideChain");
			}

			return *this;
		}

		RechargeToSideChain &RechargeToSideChain::operator=(const RechargeToSideChain &payload) {
			_merkeProof = payload._merkeProof;
			_mainChainTransaction = payload._mainChainTransaction;
			_mainChainTxHash = payload._mainChainTxHash;

			return *this;
		}

	}
}