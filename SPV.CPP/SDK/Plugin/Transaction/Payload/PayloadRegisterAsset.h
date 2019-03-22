// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADREGISTERASSET_H__
#define __ELASTOS_SDK_PAYLOADREGISTERASSET_H__

#include "IPayload.h"
#include <SDK/Plugin/Transaction/Asset.h>

namespace Elastos {
	namespace ElaWallet {

		class PayloadRegisterAsset :
				public IPayload {
		public:
			PayloadRegisterAsset();

			PayloadRegisterAsset(const PayloadRegisterAsset &payload);

			~PayloadRegisterAsset();

			void SetAsset(const Asset &asset);

			Asset GetAsset() const;

			void SetAmount(uint64_t amount);

			uint64_t GetAmount() const;

			void SetController(const uint168 &controller);

			const uint168 &GetController() const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual bool IsValid() const;

			virtual IPayload &operator=(const IPayload &payload);

			PayloadRegisterAsset &operator=(const PayloadRegisterAsset &payload);

		private:
			Asset _asset;
			uint64_t _amount;
			uint168 _controller;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADREGISTERASSET_H__
