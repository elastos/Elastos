// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADWITHDRAWASSET_H
#define __ELASTOS_SDK_PAYLOADWITHDRAWASSET_H

#include "IPayload.h"

namespace Elastos {
	namespace SDK {

		class PayloadWithDrawAsset :
				public IPayload {
		public:
			PayloadWithDrawAsset();

			PayloadWithDrawAsset(const uint32_t blockHeight, const std::string genesisBlockAddress,
			                     const std::string sideChainTransactionHash);

			~PayloadWithDrawAsset();

			virtual ByteData getData() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual void Deserialize(ByteStream &istream);

		private:
			uint32_t _blockHeight;
			std::string _genesisBlockAddress;
			std::string _sideChainTransactionHash;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADWITHDRAWASSET_H
