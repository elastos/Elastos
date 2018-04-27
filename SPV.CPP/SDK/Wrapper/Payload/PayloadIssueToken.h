// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADISSUETOKEN_H
#define __ELASTOS_SDK_PAYLOADISSUETOKEN_H

#include "IPayload.h"

namespace Elastos {
	namespace SDK {

		class PayloadIssueToken :
				public IPayload {
		public:
			PayloadIssueToken();

			PayloadIssueToken(const ByteData &merkeProff);

			~PayloadIssueToken();

			virtual ByteData getData() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual void Deserialize(ByteStream &istream);

		private:
			ByteData _merkeProof;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADISSUETOKEN_H
