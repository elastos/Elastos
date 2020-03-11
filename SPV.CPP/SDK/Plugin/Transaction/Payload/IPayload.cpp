// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		IPayload::~IPayload() {

		}

		bytes_t IPayload::GetData(uint8_t version) const {
			ByteStream stream;
			Serialize(stream, version);

			return stream.GetBytes();
		}

		bool IPayload::IsValid(uint8_t version) const {
			return true;
		}

	}
}