// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADSIDEMINING_H
#define __ELASTOS_SDK_PAYLOADSIDEMINING_H

#include "IPayload.h"
#include <SDK/Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {

		class PayloadSideMining :
				public IPayload {
		public:
			PayloadSideMining();

			PayloadSideMining(const PayloadSideMining &payload);

			PayloadSideMining(const uint256 &sideBlockHash, const uint256 &sideGensisHash, uint32_t height, const bytes_t &signedData);

			~PayloadSideMining();

            void SetSideBlockHash(const uint256 &sideBlockHash);

            void SetSideGenesisHash(const uint256 &sideGensisHash);

            void SetBlockHeight(uint32_t height);

            void SetSignedData(const bytes_t &signedData);

            const uint256 &GetSideBlockHash() const;

            const uint256 &GetSideGenesisHash() const;

            const uint32_t &GetBlockHeight() const;

            const bytes_t &GetSignedData() const;


			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadSideMining &operator=(const PayloadSideMining &payload);

		private:
			uint256 _sideBlockHash;
			uint256 _sideGenesisHash;
			uint32_t _blockHeight;
			bytes_t _signedData;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADSIDEMINING_H
