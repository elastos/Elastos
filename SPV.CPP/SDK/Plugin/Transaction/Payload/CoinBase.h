// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_COINBASE_H
#define __ELASTOS_SDK_COINBASE_H

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {
		class CoinBase :
				public IPayload {
		public:
			CoinBase();

			CoinBase(const bytes_t &coinBaseData);

			CoinBase(const CoinBase &payload);

			~CoinBase();

			void SetCoinBaseData(const bytes_t &coinBaseData);

			const bytes_t &GetCoinBaseData() const;

			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			CoinBase &operator=(const CoinBase &payload);

		private:
			bytes_t _coinBaseData;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADCOINBASE_H
