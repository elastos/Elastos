// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SIDECHAINMERKLEBLOCK_H__
#define __ELASTOS_SDK_SIDECHAINMERKLEBLOCK_H__

#include "MerkleBlockBase.h"
#include <Plugin/Block/IDAuxPow.h>

#include <fruit/fruit.h>

namespace Elastos {
	namespace ElaWallet {

		class SidechainMerkleBlock :
				public MerkleBlockBase {
		public:
			SidechainMerkleBlock();

			virtual ~SidechainMerkleBlock();

			virtual void Serialize(ByteStream &ostream, int version) const;

			virtual bool Deserialize(const ByteStream &istream, int version);

			virtual const uint256 &GetHash() const;

			virtual bool IsValid(uint32_t currentTime) const;

			virtual std::string GetBlockType() const;

		private:
			IDAuxPow idAuxPow;
		};

		class ISidechainMerkleBlockFactory {
		public:
			virtual MerkleBlockPtr createBlock() = 0;
		};

		class SidechainMerkleBlockFactory : public ISidechainMerkleBlockFactory {
		public:
			INJECT(SidechainMerkleBlockFactory()) = default;

			virtual MerkleBlockPtr createBlock();
		};

		fruit::Component<ISidechainMerkleBlockFactory> getSidechainMerkleBlockFactoryComponent();

	}
}

#endif //__ELASTOS_SDK_SIDECHAINMERKLEBLOCK_H__
