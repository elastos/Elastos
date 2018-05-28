// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADREGISTERASSET_H
#define __ELASTOS_SDK_PAYLOADREGISTERASSET_H

#include "IPayload.h"
#include "Asset.h"
#include "BRInt.h"

namespace Elastos {
	namespace SDK {
		class PayloadRegisterAsset :
				public IPayload {
		public:
			PayloadRegisterAsset();

			~PayloadRegisterAsset();

			void setAsset(const Asset &asset);

			Asset getAsset() const;

			void setAmount(uint64_t amount);

			uint64_t getAmount() const;

			void setController(const UInt168 &controller);

			const UInt168 &getController() const;

			virtual CMBlock getData() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual void Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson();

			virtual void fromJson(const nlohmann::json &jsonData);

		private:
			Asset _asset;
			uint64_t _amount;
			UInt168 _controller;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADREGISTERASSET_H
