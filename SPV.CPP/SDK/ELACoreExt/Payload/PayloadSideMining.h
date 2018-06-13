// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADSIDEMINING_H
#define __ELASTOS_SDK_PAYLOADSIDEMINING_H

#include "IPayload.h"
#include "BRInt.h"

namespace Elastos {
	namespace SDK {

		class PayloadSideMining :
				public IPayload {
		public:
			PayloadSideMining();

			PayloadSideMining(const UInt256 &sideBlockHash, const UInt256 &sideGensisHash);

			~PayloadSideMining();

            void setSideBlockHash(const UInt256 &sideBlockHash);
            void setSideGenesisHash(const UInt256 &sideGensisHash);

			virtual CMBlock getData() const;


			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson();

			virtual void fromJson(const nlohmann::json &jsonData);

		private:
			UInt256 _sideBlockHash;
			UInt256 _sideGenesisHash;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADSIDEMINING_H
