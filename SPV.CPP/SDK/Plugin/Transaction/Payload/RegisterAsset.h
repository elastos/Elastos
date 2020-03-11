// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_REGISTERASSET_H__
#define __ELASTOS_SDK_REGISTERASSET_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		class Asset;
		typedef boost::shared_ptr<Asset> AssetPtr;

		class RegisterAsset :
				public IPayload {
		public:
			RegisterAsset();

			RegisterAsset(const AssetPtr &asset, uint64_t amount, const uint168 &controller);

			RegisterAsset(const RegisterAsset &payload);

			~RegisterAsset();

			void SetAsset(const AssetPtr &asset) {
				_asset = asset;
			}

			const AssetPtr &GetAsset() const {
				return _asset;
			}

			void SetAmount(uint64_t amount) {
				_amount = amount;
			}

			uint64_t GetAmount() const {
				return _amount;
			}

			void SetController(const uint168 &controller) {
				_controller = controller;
			}

			const uint168 &GetController() const {
				return _controller;
			}

			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual bool IsValid(uint8_t version) const;

			virtual IPayload &operator=(const IPayload &payload);

			RegisterAsset &operator=(const RegisterAsset &payload);

		private:
			AssetPtr _asset;
			uint64_t _amount;
			uint168 _controller;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADREGISTERASSET_H__
