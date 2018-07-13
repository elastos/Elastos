// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SIDECHAINMERKLEBLOCK_H__
#define __ELASTOS_SDK_SIDECHAINMERKLEBLOCK_H__

#include "IdMerkleBlock.h"
#include "Plugin/Interface/IMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class SidechainMerkleBlock :
				public Wrapper<BRMerkleBlock>,
				public IMerkleBlock {
		public:
			SidechainMerkleBlock();

			SidechainMerkleBlock(IdMerkleBlock *merkleBlock, bool manageRaw);

			virtual ~SidechainMerkleBlock();

			virtual std::string toString() const;

			virtual BRMerkleBlock *getRaw() const;

			virtual IMerkleBlock *CreateMerkleBlock(bool manageRaw);

			virtual IMerkleBlock *CreateFromRaw(BRMerkleBlock *block, bool manageRaw);

			virtual IMerkleBlock *Clone(bool manageRaw) const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &);

			virtual BRMerkleBlock *getRawBlock() const;

			virtual void deleteRawBlock();

			virtual void initFromRaw(BRMerkleBlock *block, bool manageRaw);

			virtual UInt256 getBlockHash() const;

			virtual uint32_t getHeight() const;

			virtual void setHeight(uint32_t height);

			virtual bool isValid(uint32_t currentTime) const;

			virtual std::string getBlockType() const;

		private:
			IdMerkleBlock *_merkleBlock;
			bool _manageRaw;
		};

	}
}

#endif //__ELASTOS_SDK_SIDECHAINMERKLEBLOCK_H__
