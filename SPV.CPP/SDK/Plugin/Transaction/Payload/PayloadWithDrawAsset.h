// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADWITHDRAWASSET_H
#define __ELASTOS_SDK_PAYLOADWITHDRAWASSET_H

#include "IPayload.h"
#include <Core/BRInt.h>

namespace Elastos {
	namespace ElaWallet {

		class PayloadWithDrawAsset :
				public IPayload {
		public:
			PayloadWithDrawAsset();

			PayloadWithDrawAsset(const PayloadWithDrawAsset &payload);

			PayloadWithDrawAsset(uint32_t blockHeight, const std::string &genesisBlockAddress,
								 const std::vector<UInt256> &sideChainTransactionHash);

			~PayloadWithDrawAsset();

			void setBlockHeight(uint32_t blockHeight);
			uint32_t getBlockHeight() const;
			void setGenesisBlockAddress(const std::string &genesisBlockAddress);
			const std::string &getGenesisBlockAddress() const;
			void setSideChainTransacitonHash(const std::vector<UInt256> &sideChainTransactionHash);
			const std::vector<UInt256> &getSideChainTransacitonHash() const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(ByteStream &istream, uint8_t version);

			virtual nlohmann::json toJson(uint8_t version) const;

			virtual void fromJson(const nlohmann::json &jsonData, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadWithDrawAsset &operator=(const PayloadWithDrawAsset &payload);

		private:
			uint32_t _blockHeight;
			std::string _genesisBlockAddress;
			std::vector<UInt256> _sideChainTransactionHash;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADWITHDRAWASSET_H
