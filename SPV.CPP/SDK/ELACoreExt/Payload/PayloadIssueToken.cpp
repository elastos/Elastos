// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>

#include "PayloadIssueToken.h"

namespace Elastos {
	namespace ElaWallet {

		PayloadIssueToken::PayloadIssueToken() {

		}

		PayloadIssueToken::PayloadIssueToken(const CMBlock &merkeProff) {
			_merkeProof = merkeProff;
		}

		PayloadIssueToken::~PayloadIssueToken() {
		}

		CMBlock PayloadIssueToken::getData() const {
			ByteStream stream;
			Serialize(stream);
			return stream.getBuffer();
		}

		void PayloadIssueToken::Serialize(ByteStream &ostream) const {
			ostream.putVarUint(_merkeProof.GetSize());
			if (_merkeProof.GetSize() > 0) {
				ostream.putBytes(_merkeProof, _merkeProof.GetSize());
			}
		}

		bool PayloadIssueToken::Deserialize(ByteStream &istream) {
			uint64_t len = istream.getVarUint();
			if (0 < len) {
				uint8_t *buff = new uint8_t[len];
				if (buff) {
					istream.getBytes(buff, len);
					_merkeProof.Resize(size_t(len));
					memcpy(_merkeProof, buff, len);
				}
			}

			return true;
		}

		nlohmann::json PayloadIssueToken::toJson() {
			char *data = new char[_merkeProof.GetSize()];
			memcpy(data, _merkeProof, _merkeProof.GetSize());
			std::string content(data, _merkeProof.GetSize());

			nlohmann::json jsonData;
			jsonData["data"] = content;
			return jsonData;
		}

		void PayloadIssueToken::fromJson(const nlohmann::json &jsonData) {
			std::string content = jsonData["data"].get<std::string>();
			const char* data = content.c_str();
			_merkeProof.Resize(content.size());
			memcpy(_merkeProof, data, content.size());
		}
	}
}