// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADCOINBASE_H
#define __ELASTOS_SDK_PAYLOADCOINBASE_H

#include "IPayload.h"

namespace Elastos {
	namespace SDK {
		class PayloadCoinBase :
				public IPayload {
		public:
			PayloadCoinBase();

			PayloadCoinBase(CMBlock &coinBaseData);

			~PayloadCoinBase();

			void setCoinBaseData(const CMBlock &coinBaseData);

			virtual CMBlock getData() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual void Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson();

			virtual void fromJson(nlohmann::json jsonData);

		private:
			CMBlock _coinBaseData;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADCOINBASE_H
