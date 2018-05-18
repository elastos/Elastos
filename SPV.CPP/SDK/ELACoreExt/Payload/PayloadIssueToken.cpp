// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>

#include "PayloadIssueToken.h"

namespace Elastos {
	namespace SDK {

		PayloadIssueToken::PayloadIssueToken() {

		}

		PayloadIssueToken::PayloadIssueToken(const CMBlock &merkeProff) {
			_merkeProof = merkeProff;
		}

		PayloadIssueToken::~PayloadIssueToken() {
		}

		CMBlock PayloadIssueToken::getData() const {
			ByteStream byteStream;
			Serialize(byteStream);
			CMBlock ret(byteStream.length());
			uint8_t *tmp = byteStream.getBuf();
			memcpy(ret, tmp, byteStream.length());
			delete []tmp;
			return ret;
		}

		void PayloadIssueToken::Serialize(ByteStream &ostream) const {
			ostream.putVarUint(_merkeProof.GetSize());
			if (_merkeProof.GetSize() > 0) {
				ostream.putBytes(_merkeProof, _merkeProof.GetSize());
			}
		}

		void PayloadIssueToken::Deserialize(ByteStream &istream) {
			uint64_t len = istream.getVarUint();
			if (0 < len) {
				uint8_t *buff = new uint8_t[len];
				if (buff) {
					istream.getBytes(buff, len);
					_merkeProof.Resize(len);
					memcpy(_merkeProof, buff, len);
				}
			}
		}
	}
}