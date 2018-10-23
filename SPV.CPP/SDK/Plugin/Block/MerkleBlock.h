// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MERKLEBLOCK_H__
#define __ELASTOS_SDK_MERKLEBLOCK_H__

#include <fruit/fruit.h>
#include <boost/shared_ptr.hpp>
#include <nlohmann/json.hpp>

#include "Wrapper.h"
#include "CMemBlock.h"
#include "Plugin/Block/MerkleBlockBase.h"
#include "AuxPow.h"
#include "ELAMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class MerkleBlock :
				public MerkleBlockBase {

		public:
			INJECT(MerkleBlock());

			MerkleBlock(const MerkleBlock &merkleBlock);

			~MerkleBlock();

			MerkleBlock &operator=(const MerkleBlock &other);

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &);

			virtual const UInt256 &getHash() const;

			virtual bool isValid(uint32_t currentTime) const;

			virtual std::string getBlockType() const { return "ELA"; }

			const AuxPow &getAuxPow() const;

			void setAuxPow(const AuxPow &pow);

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
