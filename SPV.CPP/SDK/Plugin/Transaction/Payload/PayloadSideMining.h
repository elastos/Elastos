// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADSIDEMINING_H
#define __ELASTOS_SDK_PAYLOADSIDEMINING_H

#include "IPayload.h"
#include <Core/BRInt.h>

namespace Elastos {
	namespace ElaWallet {

		class PayloadSideMining :
				public IPayload {
		public:
			PayloadSideMining();

			PayloadSideMining(const PayloadSideMining &payload);

			PayloadSideMining(const UInt256 &sideBlockHash, const UInt256 &sideGensisHash, uint32_t height, const CMBlock &signedData);

			~PayloadSideMining();

            void SetSideBlockHash(const UInt256 &sideBlockHash);

            void SetSideGenesisHash(const UInt256 &sideGensisHash);

            void SetBlockHeight(uint32_t height);

            void SetSignedData(const CMBlock &signedData);

            const UInt256 &GetSideBlockHash() const;

            const UInt256 &GetSideGenesisHash() const;

            const uint32_t &GetBlockHeight() const;

            const CMBlock &GetSignedData() const;


			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(ByteStream &istream, uint8_t version);

			virtual nlohmann::json toJson(uint8_t version) const;

			virtual void fromJson(const nlohmann::json &jsonData, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadSideMining &operator=(const PayloadSideMining &payload);

		private:
			UInt256 _sideBlockHash;
			UInt256 _sideGenesisHash;
			uint32_t _blockHeight;
			CMBlock _signedData;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADSIDEMINING_H
