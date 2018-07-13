// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MERKLEBLOCK_H__
#define __ELASTOS_SDK_MERKLEBLOCK_H__

#include <boost/shared_ptr.hpp>
#include <nlohmann/json.hpp>

#include "Wrapper.h"
#include "CMemBlock.h"
#include "Plugin/Interface/IMerkleBlock.h"
#include "Plugin/Registry.h"
#include "ELACoreExt/AuxPow.h"
#include "ELACoreExt/ELAMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class MerkleBlock :
				public IMerkleBlock,
				public Wrapper<BRMerkleBlock> {

		public:
			MerkleBlock();

			MerkleBlock(ELAMerkleBlock *merkleBlock, bool manageRaw);

			MerkleBlock(const ELAMerkleBlock &merkleBlock);

			~MerkleBlock();

			virtual std::string toString() const;

			virtual IMerkleBlock *CreateMerkleBlock(bool manageRaw);

			virtual IMerkleBlock *CreateFromRaw(BRMerkleBlock *block, bool manageRaw);
			virtual IMerkleBlock *Clone(bool manageRaw) const;

			virtual BRMerkleBlock *getRaw() const;

			virtual void initFromRaw(BRMerkleBlock *block, bool manageRaw);

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &);

			virtual UInt256 getBlockHash() const;

			uint32_t getVersion() const;

			UInt256 getPrevBlockHash() const;

			UInt256 getRootBlockHash() const;

			uint32_t getTimestamp() const;

			uint32_t getTarget() const;

			uint32_t getNonce() const;

			uint32_t getTransactionCount() const;

			virtual uint32_t getHeight() const;

			virtual void setHeight(uint32_t height);

			virtual bool isValid(uint32_t currentTime) const;

			const AuxPow &getAuxPow() const;

			bool containsTransactionHash(UInt256 hash) const;

			virtual BRMerkleBlock *getRawBlock() const;

			virtual void deleteRawBlock();

			virtual std::string getBlockType() const { return "ELA"; }

			static void serializeNoAux(ByteStream &ostream, const BRMerkleBlock &raw);

			static UInt256 MerkleBlockRootR(size_t *hashIdx, size_t *flagIdx, int depth, const BRMerkleBlock &raw);

		private:
			ELAMerkleBlock *_merkleBlock;
			bool _manageRaw;
		};

	}
}

#endif //__ELASTOS_SDK_MERKLEBLOCK_H__
