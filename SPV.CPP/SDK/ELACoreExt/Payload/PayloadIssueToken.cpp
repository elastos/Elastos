// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>

#include "PayloadIssueToken.h"

namespace Elastos {
	namespace SDK {

		PayloadIssueToken::PayloadIssueToken() {

		}

		PayloadIssueToken::PayloadIssueToken(const ByteData &merkeProff) {
			uint8_t *buff = new uint8_t[merkeProff.length];
			memcpy(buff, merkeProff.data, merkeProff.length);
			_merkeProof = ByteData(buff, merkeProff.length);
		}

		PayloadIssueToken::~PayloadIssueToken() {
		}

		ByteData PayloadIssueToken::getData() const {
			ByteStream byteStream;
			Serialize(byteStream);
			return ByteData(byteStream.getBuf(), byteStream.length());
		}

		void PayloadIssueToken::Serialize(ByteStream &ostream) const {
			ostream.putVarUint(_merkeProof.length);
			if (_merkeProof.length > 0) {
				ostream.putBytes(_merkeProof.data, _merkeProof.length);
			}
		}

		void PayloadIssueToken::Deserialize(ByteStream &istream) {
			uint64_t len = istream.getVarUint();
			uint8_t *buff = new uint8_t[len];
			if (len > 0) {
				istream.getBytes(buff, len);
			}
			_merkeProof = ByteData(buff, len);
		}
	}
}