// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADREGISTERASSET_H__
#define __ELASTOS_SDK_PAYLOADREGISTERASSET_H__

#include "IPayload.h"
#include <SDK/Plugin/Transaction/Asset.h>
#include <Core/BRInt.h>

namespace Elastos {
	namespace ElaWallet {

		class PayloadRegisterAsset :
				public IPayload {
		public:
			PayloadRegisterAsset();

			PayloadRegisterAsset(const PayloadRegisterAsset &payload);

			~PayloadRegisterAsset();

			void setAsset(const Asset &asset);

			Asset getAsset() const;

			void setAmount(uint64_t amount);

			uint64_t getAmount() const;

			void setController(const UInt168 &controller);

			const UInt168 &getController() const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(ByteStream &istream, uint8_t version);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &jsonData);

			virtual bool isValid() const;

			virtual IPayload &operator=(const IPayload &payload);

			PayloadRegisterAsset &operator=(const PayloadRegisterAsset &payload);

		private:
			Asset _asset;
			uint64_t _amount;
			UInt168 _controller;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADREGISTERASSET_H__
