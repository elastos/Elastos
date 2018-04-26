// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADCOINBASE_H
#define __ELASTOS_SDK_PAYLOADCOINBASE_H

#include "IPayload.h"
#include "ByteData.h"

namespace Elastos {
	namespace SDK {
		class PayloadCoinBase :
				public IPayload {
		public:
			PayloadCoinBase();

			PayloadCoinBase(ByteData &coinBaseData);

			~PayloadCoinBase();

			virtual ByteData getData() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual void Deserialize(ByteStream &istream);

		private:
			ByteData _coinBaseData;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADCOINBASE_H
