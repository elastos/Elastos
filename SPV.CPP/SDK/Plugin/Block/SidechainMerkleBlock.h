// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SIDECHAINMERKLEBLOCK_H__
#define __ELASTOS_SDK_SIDECHAINMERKLEBLOCK_H__

#include <fruit/fruit.h>

#include "MerkleBlockBase.h"
#include "IdAuxPow.h"

namespace Elastos {
	namespace ElaWallet {

		class SidechainMerkleBlock :
				public MerkleBlockBase {
		public:
			SidechainMerkleBlock();

			virtual ~SidechainMerkleBlock();

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &);

			virtual const UInt256 &getHash() const;

			virtual bool isValid(uint32_t currentTime) const;

			virtual std::string getBlockType() const;

		private:
			IdAuxPow idAuxPow;
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
