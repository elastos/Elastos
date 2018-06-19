// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		IPayload::~IPayload() {

		}

		CMBlock IPayload::getData() const {
			ByteStream stream;
			Serialize(stream);
			uint8_t* buf = stream.getBuf();
			uint64_t len = stream.length();
			CMBlock db(len);
			memcpy(db, buf, len);

			return db;
		}

		bool IPayload::isValid() const {
			return true;
		}
	}
}