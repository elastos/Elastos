// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include <Core/BRMerkleBlock.h>
#include <SDK/Common/Log.h>

#include "Utils.h"
#include "MerkleBlock.h"
#include "SidechainMerkleBlock.h"
#include "Plugin/Registry.h"

#define MAX_PROOF_OF_WORK 0xff7fffff    // highest value for difficulty target

namespace Elastos {
	namespace ElaWallet {

		SidechainMerkleBlock::SidechainMerkleBlock() {
		}

		SidechainMerkleBlock::~SidechainMerkleBlock() {
		}

		void SidechainMerkleBlock::Serialize(ByteStream &ostream) const {
			MerkleBlockBase::serializeNoAux(ostream);
			idAuxPow.Serialize(ostream);
			ostream.writeUint8(1);
			MerkleBlockBase::serializeAfterAux(ostream);
		}

		bool SidechainMerkleBlock::Deserialize(ByteStream &istream) {
			if (!MerkleBlockBase::deserializeNoAux(istream) || !idAuxPow.Deserialize(istream))
				return false;

			istream.drop(1);
			if (!MerkleBlockBase::deserializeAfterAux(istream))
				return false;

			getHash();

			return true;
		}

		nlohmann::json SidechainMerkleBlock::toJson() const {
			nlohmann::json j = MerkleBlockBase::toJson();
			j["IdAuxPow"] = idAuxPow.toJson();

			return j;
		}

		void SidechainMerkleBlock::fromJson(const nlohmann::json &j) {
			MerkleBlockBase::fromJson(j);
			nlohmann::json auxPowJson = j["IdAuxPow"];
			idAuxPow.fromJson(auxPowJson);
		}

		const UInt256 &SidechainMerkleBlock::getHash() const {
			UInt256 zero = UINT256_ZERO;
			if (UInt256Eq(&_blockHash, &zero)) {
				ByteStream ostream;
				MerkleBlockBase::serializeNoAux(ostream);
				UInt256 hash = UINT256_ZERO;
				CMBlock buf = ostream.getBuffer();
				BRSHA256_2(&hash, buf, buf.GetSize());
				UInt256Set((void *) &_blockHash, hash);
			}
			return _blockHash;
		}

		bool SidechainMerkleBlock::isValid(uint32_t currentTime) const {
			// target is in "compact" format, where the most significant byte is the size of resulting value in bytes, the next
			// bit is the sign, and the remaining 23bits is the value after having been right shifted by (size - 3)*8 bits
			static const uint32_t maxsize = MAX_PROOF_OF_WORK >> 24, maxtarget = MAX_PROOF_OF_WORK & 0x00ffffff;
			const uint32_t size = _target >> 24, target = target & 0x00ffffff;
			size_t hashIdx = 0, flagIdx = 0;
			UInt256 merkleRoot = MerkleBlockBase::MerkleBlockRootR(&hashIdx, &flagIdx, 0), t = UINT256_ZERO;
			int r = 1;

			// check if merkle root is correct
			if (_totalTx > 0 && !UInt256Eq(&(merkleRoot), &(merkleRoot))) r = 0;

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

		std::string SidechainMerkleBlock::getBlockType() const {
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