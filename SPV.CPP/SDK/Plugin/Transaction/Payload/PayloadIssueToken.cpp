// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadIssueToken.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

#include <cstring>

namespace Elastos {
	namespace ElaWallet {

		PayloadIssueToken::PayloadIssueToken() {

		}

		PayloadIssueToken::PayloadIssueToken(const CMBlock &merkeProff, const CMBlock &mainChainTransaction) {
			_merkeProof = merkeProff;
			_mainChainTransaction = mainChainTransaction;
		}

		PayloadIssueToken::~PayloadIssueToken() {
		}

		CMBlock PayloadIssueToken::getData() const {
			ByteStream stream;
			Serialize(stream);
			return stream.getBuffer();
		}

		void PayloadIssueToken::Serialize(ByteStream &ostream) const {
			ostream.writeVarBytes(_merkeProof);
			ostream.writeVarBytes(_mainChainTransaction);
		}

		bool PayloadIssueToken::Deserialize(ByteStream &istream) {
			if (!istream.readVarBytes(_merkeProof)) {
				Log::error("PayloadIssueToken deserialize merke proof error");
				return false;
			}

			if (!istream.readVarBytes(_mainChainTransaction)) {
				Log::error("PayloadIssueToken deserialize main chain transaction error");
				return false;
			}

			return true;
		}

		nlohmann::json PayloadIssueToken::toJson() const {
			nlohmann::json j;

			j["MerkleProof"] = Utils::encodeHex(_merkeProof);
			j["MainChainTransaction"] = Utils::encodeHex(_mainChainTransaction);

			return j;
		}

		void PayloadIssueToken::fromJson(const nlohmann::json &j) {
			_merkeProof = Utils::decodeHex(j["MerkleProof"].get<std::string>());
			_mainChainTransaction = Utils::decodeHex(j["MainChainTransaction"].get<std::string>());
		}
	}
}