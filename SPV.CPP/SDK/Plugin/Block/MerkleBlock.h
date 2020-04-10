// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MERKLEBLOCK_H__
#define __ELASTOS_SDK_MERKLEBLOCK_H__

#include "MerkleBlockBase.h"
#include "AuxPow.h"
#include "ELAMerkleBlock.h"

#include <fruit/fruit.h>
#include <nlohmann/json.hpp>
#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class MerkleBlock :
				public MerkleBlockBase {

		public:
			INJECT(MerkleBlock());

			MerkleBlock(const MerkleBlock &merkleBlock);

			~MerkleBlock();

			MerkleBlock &operator=(const MerkleBlock &other);

			virtual void Serialize(ByteStream &ostream, int version) const;

			virtual bool Deserialize(const ByteStream &istream, int version);

			virtual const uint256 &GetHash() const;

			virtual bool IsValid(uint32_t currentTime) const;

			virtual std::string GetBlockType() const { return "ELA"; }

			const AuxPow &GetAuxPow() const;

			void SetAuxPow(const AuxPow &pow);

		private:
			AuxPow _auxPow;
		};

		class IMerkleBlockFactory {
		public:
			virtual MerkleBlockPtr createBlock() = 0;
		};

		class MerkleBlockFactory : public IMerkleBlockFactory {
		public:
			INJECT(MerkleBlockFactory()) = default;

			virtual MerkleBlockPtr createBlock();
		};

		fruit::Component<IMerkleBlockFactory> getMerkleBlockFactoryComponent();

	}
}

#endif //__ELASTOS_SDK_MERKLEBLOCK_H__
