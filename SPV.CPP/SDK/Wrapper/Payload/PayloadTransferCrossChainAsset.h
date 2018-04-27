// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADTRANSFERCROSSCHAINASSET_H
#define __ELASTOS_SDK_PAYLOADTRANSFERCROSSCHAINASSET_H

#include <map>

#include "IPayload.h"

namespace Elastos {
	namespace SDK {

		class PayloadTransferCrossChainAsset :
				public IPayload {
		public:
			PayloadTransferCrossChainAsset();

			PayloadTransferCrossChainAsset(const std::map<std::string, uint64_t> addressMap);

			~PayloadTransferCrossChainAsset();

			virtual ByteData getData() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual void Deserialize(ByteStream &istream);

		private:

			std::map<std::string, uint64_t> _addressMap;

		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADTRANSFERCROSSCHAINASSET_H
