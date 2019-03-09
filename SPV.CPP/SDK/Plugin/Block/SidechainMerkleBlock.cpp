// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MerkleBlock.h"
#include "SidechainMerkleBlock.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Plugin/Registry.h>

#include <Core/BRMerkleBlock.h>
#include <Core/BRCrypto.h>

#include <sstream>

#define MAX_PROOF_OF_WORK 0xff7fffff    // highest value for difficulty target

namespace Elastos {
	namespace ElaWallet {

		SidechainMerkleBlock::SidechainMerkleBlock() {
		}

		SidechainMerkleBlock::~SidechainMerkleBlock() {
		}

		void SidechainMerkleBlock::Serialize(ByteStream &ostream) const {
			MerkleBlockBase::SerializeNoAux(ostream);
			idAuxPow.Serialize(ostream);
			ostream.WriteUint8(1);
			MerkleBlockBase::SerializeAfterAux(ostream);
		}

		bool SidechainMerkleBlock::Deserialize(ByteStream &istream) {
			if (!MerkleBlockBase::DeserializeNoAux(istream) || !idAuxPow.Deserialize(istream))
				return false;

			istream.Drop(1);
			if (!MerkleBlockBase::DeserializeAfterAux(istream))
				return false;

			GetHash();

			return true;
		}

		nlohmann::json SidechainMerkleBlock::ToJson() const {
			nlohmann::json j = MerkleBlockBase::ToJson();
			j["IdAuxPow"] = idAuxPow.ToJson();

			return j;
		}

		void SidechainMerkleBlock::FromJson(const nlohmann::json &j) {
			MerkleBlockBase::FromJson(j);
			nlohmann::json auxPowJson = j["IdAuxPow"];
			idAuxPow.FromJson(auxPowJson);
		}

		const UInt256 &SidechainMerkleBlock::GetHash() const {
			UInt256 zero = UINT256_ZERO;
			if (UInt256Eq(&_blockHash, &zero)) {
				ByteStream ostream;
				MerkleBlockBase::SerializeNoAux(ostream);
				UInt256 hash = UINT256_ZERO;
				CMBlock buf = ostream.GetBuffer();
				BRSHA256_2(&hash, buf, buf.GetSize());
				UInt256Set((void *) &_blockHash, hash);
			}
			return _blockHash;
		}

		bool SidechainMerkleBlock::IsValid(uint32_t currentTime) const {
			// target is in "compact" format, where the most significant byte is the size of resulting value in bytes, the next
			// bit is the sign, and the remaining 23bits is the value after having been right shifted by (size - 3)*8 bits
			static const uint32_t maxsize = MAX_PROOF_OF_WORK >> 24, maxtarget = MAX_PROOF_OF_WORK & 0x00ffffff;
			const uint32_t size = _target >> 24, target = _target & 0x00ffffff;
			size_t hashIdx = 0, flagIdx = 0;
			UInt256 merkleRoot = MerkleBlockBase::MerkleBlockRootR(&hashIdx, &flagIdx, 0), t = UINT256_ZERO;
			int r = 1;

			// check if merkle root is correct
			if (_totalTx > 0 && !UInt256Eq(&(merkleRoot), &(_merkleRoot))) r = 0;

			// check if timestamp is too far in future
			if (_timestamp > currentTime + BLOCK_MAX_TIME_DRIFT) r = 0;

			// check if proof-of-work target is out of range
			if (target == 0 || target & 0x00800000 || size > maxsize || (size == maxsize && target > maxtarget)) r = 0;

			if (size > 3) UInt32SetLE(&t.u8[size - 3], target);
			else UInt32SetLE(t.u8, target >> (3 - size) * 8);

			//todo verify block hash
//			UInt256 auxBlockHash = _merkleBlock->auxPow.getParBlockHeaderHash();
//			for (int i = sizeof(t) - 1; r && i >= 0; i--) { // check proof-of-work
//				if (auxBlockHash.u8[i] < t.u8[i]) break;
//				if (auxBlockHash.u8[i] > t.u8[i]) r = 0;
//			}

			return r;
		}

		std::string SidechainMerkleBlock::GetBlockType() const {
			return "SideStandard";
		}

		MerkleBlockPtr SidechainMerkleBlockFactory::createBlock() {
			return MerkleBlockPtr(new SidechainMerkleBlock);
		}

		fruit::Component<ISidechainMerkleBlockFactory> getSidechainMerkleBlockFactoryComponent() {
			return fruit::createComponent()
					.bind<ISidechainMerkleBlockFactory, SidechainMerkleBlockFactory>();
		}
	}
}