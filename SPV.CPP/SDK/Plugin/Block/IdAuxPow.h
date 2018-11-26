// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDAUXPOW_H__
#define __ELASTOS_SDK_IDAUXPOW_H__

#include "ELAMerkleBlock.h"

#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Interface/ELAMessageSerializable.h>

#include <Core/BRInt.h>
#include <Core/BRTransaction.h>
#include <Core/BRMerkleBlock.h>

#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class IdAuxPow :
			public ELAMessageSerializable {
		public:
			IdAuxPow();

			IdAuxPow(const IdAuxPow &auxPow);

			~IdAuxPow();

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &jsonData);

			IdAuxPow &operator=(const IdAuxPow &idAuxPow);

			void setIdAuxMerkleBranch(const std::vector<UInt256> &idAuxMerkleBranch);
			void setIdAuxMerkleIndex(uint32_t index);
			void setIdAuxBlockTx(const Transaction &tx);
			void setMainBlockHeader(ELAMerkleBlock *blockHeader);

			const std::vector<UInt256> &getIdAuxMerkleBranch() const;
			uint32_t getIdAuxMerkleIndex() const;
			const Transaction &getIdAuxBlockTx() const;
			ELAMerkleBlock *getMainBlockHeader() const;

		private:
			nlohmann::json mainBlockHeaderToJson() const;
			void mainBlockHeaderFromJson(const nlohmann::json &j);

		private:
			std::vector<UInt256> _idAuxMerkleBranch;
			uint32_t _idAuxMerkleIndex;
			Transaction _idAuxBlockTx;
			ELAMerkleBlock *_mainBlockHeader;
		};

	}
}

#endif //__ELASTOS_SDK_IDAUXPOW_H__
