// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IOutputPayload.h"

namespace Elastos {
	namespace ElaWallet {

		IOutputPayload::~IOutputPayload() {

		}

		bytes_t IOutputPayload::getData() const {
			ByteStream stream;
			Serialize(stream);

			return stream.GetBytes();
		}

	}
}