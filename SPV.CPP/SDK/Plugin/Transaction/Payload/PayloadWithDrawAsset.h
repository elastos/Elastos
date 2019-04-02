// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADWITHDRAWASSET_H
#define __ELASTOS_SDK_PAYLOADWITHDRAWASSET_H

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		class PayloadWithDrawAsset :
				public IPayload {
		public:
			PayloadWithDrawAsset();

			PayloadWithDrawAsset(const PayloadWithDrawAsset &payload);

			PayloadWithDrawAsset(uint32_t blockHeight, const std::string &genesisBlockAddress,
								 const std::vector<uint256> &sideChainTransactionHash);

			~PayloadWithDrawAsset();

			void SetBlockHeight(uint32_t blockHeight);

			uint32_t GetBlockHeight() const;

			void SetGenesisBlockAddress(const std::string &genesisBlockAddress);

			const std::string &GetGenesisBlockAddress() const;

			void SetSideChainTransacitonHash(const std::vector<uint256> &sideChainTransactionHash);

			const std::vector<uint256> &GetSideChainTransacitonHash() const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadWithDrawAsset &operator=(const PayloadWithDrawAsset &payload);

		private:
			uint32_t _blockHeight;
			std::string _genesisBlockAddress;
			std::vector<uint256> _sideChainTransactionHash;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADWITHDRAWASSET_H
