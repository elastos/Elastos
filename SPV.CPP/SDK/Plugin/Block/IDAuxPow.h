// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDAUXPOW_H__
#define __ELASTOS_SDK_IDAUXPOW_H__

#include "ELAMerkleBlock.h"

#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Interface/ELAMessageSerializable.h>

#include <bitcoin/BRTransaction.h>
#include <bitcoin/BRMerkleBlock.h>

#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class IDAuxPow {
		public:
			IDAuxPow();

			IDAuxPow(const IDAuxPow &auxPow);

			~IDAuxPow();

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(const ByteStream &istream);

			IDAuxPow &operator=(const IDAuxPow &idAuxPow);

			void SetIdAuxMerkleBranch(const std::vector<uint256> &idAuxMerkleBranch);
			void SetIdAuxMerkleIndex(uint32_t index);
			void SetIdAuxBlockTx(const Transaction &tx);
			void SetMainBlockHeader(ELAMerkleBlock *blockHeader);

			const std::vector<uint256> &GetIdAuxMerkleBranch() const;
			uint32_t GetIdAuxMerkleIndex() const;
			const Transaction &GetIdAuxBlockTx() const;
			ELAMerkleBlock *GetMainBlockHeader() const;

		private:
			std::vector<uint256> _idAuxMerkleBranch;
			uint32_t _idAuxMerkleIndex;
			Transaction _idAuxBlockTx;
			ELAMerkleBlock *_mainBlockHeader;
		};

	}
}

#endif //__ELASTOS_SDK_IDAUXPOW_H__
