// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TESTHELPER_H__
#define __ELASTOS_SDK_TESTHELPER_H__

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define TEST_ASCII_BEGIN 48

#include "AuxPow.h"
#include "ELAMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		static UInt256 getRandUInt256(void) {
			UInt256 u;
			for (size_t i = 0; i < ARRAY_SIZE(u.u32); ++i) {
				u.u32[i] = rand();
			}
			return u;
		}

		static CMBlock getRandCMBlock(size_t size) {
			CMBlock block(size);

			for (size_t i = 0; i < size; ++i) {
				block[i] = (uint8_t) rand();
			}

			return block;
		}

		static std::string getRandString(size_t length) {
			char buf[length];
			for (size_t i = 0; i < length; ++i) {
				buf[i] = static_cast<uint8_t>(TEST_ASCII_BEGIN + rand() % 75);
			}

			return std::string(buf);
		}

		static UInt168 getRandUInt168(void) {
			UInt168 u;
			for (size_t i = 0; i < ARRAY_SIZE(u.u8); ++i) {
				u.u8[i] = rand();
			}
			return u;
		}

		static AuxPow createDummyAuxPow() {
			AuxPow auxPow;

			std::vector<UInt256> hashes(3);
			for (size_t i = 0; i < hashes.size(); ++i) {
				hashes[i] = getRandUInt256();
			}
			auxPow.setAuxMerkleBranch(hashes);

			for (size_t i = 0; i < hashes.size(); ++i) {
				hashes[i] = getRandUInt256();
			}
			auxPow.setCoinBaseMerkle(hashes);

			auxPow.setAuxMerkleIndex(123);
			auxPow.setParMerkleIndex(456);

			auxPow.setParentHash(getRandUInt256());

			// init transaction
			BRTransaction *tx = BRTransactionNew();
			tx->txHash = getRandUInt256();
			tx->version = rand();
			for (size_t i = 0; i < 3; ++i) {
				CMBlock script = getRandCMBlock(25);
				CMBlock signature = getRandCMBlock(28);
				BRTransactionAddInput(tx, getRandUInt256(), i, rand(),
									  script, script.GetSize(), signature, signature.GetSize(), rand());
			}
			for (size_t i = 0; i < 4; ++i) {
				CMBlock script = getRandCMBlock(25);
				BRTransactionAddOutput(tx, rand(), script, script.GetSize());
			}
			tx->lockTime = rand();
			tx->blockHeight = rand();
			tx->timestamp = rand();
			auxPow.setBTCTransaction(tx);

			// init merkle block
			BRMerkleBlock *block = BRMerkleBlockNew(nullptr);
			block->blockHash = getRandUInt256();
			block->version = rand();
			block->prevBlock = getRandUInt256();
			block->merkleRoot = getRandUInt256();
			block->timestamp = rand();
			block->target = rand();
			block->nonce = rand();
			block->totalTx = rand();
			UInt256 MBHashes[3];
			for (size_t i = 0; i < ARRAY_SIZE(MBHashes); ++i) {
				MBHashes[i] = getRandUInt256();
			}
			block->hashesCount = ARRAY_SIZE(MBHashes);
			CMBlock flags = getRandCMBlock(5);
			block->flagsLen = flags.GetSize();
			BRMerkleBlockSetTxHashes(block, MBHashes, ARRAY_SIZE(MBHashes), flags, flags.GetSize());
			block->height = rand();
			auxPow.setParBlockHeader(block);

			return auxPow;
		}

		static void verrifyAuxPowEqual(const AuxPow &auxPow, const AuxPow &auxPowVerify, bool checkAll = true) {
			const std::vector<UInt256> &origMerkleBranch = auxPow.getAuxMerkleBranch();
			const std::vector<UInt256> &merkleBranch = auxPowVerify.getAuxMerkleBranch();
			REQUIRE(merkleBranch.size() == origMerkleBranch.size());
			for (size_t i = 0; i < merkleBranch.size(); ++i) {
				REQUIRE(UInt256Eq(&merkleBranch[i], &origMerkleBranch[i]));
			}

			const std::vector<UInt256> &origCoinBaseMerkle = auxPow.getParCoinBaseMerkle();
			const std::vector<UInt256> &coinBaseMerkle = auxPowVerify.getParCoinBaseMerkle();
			REQUIRE(origCoinBaseMerkle.size() == coinBaseMerkle.size());
			for (size_t i = 0; i < coinBaseMerkle.size(); ++i) {
				REQUIRE(UInt256Eq(&coinBaseMerkle[i], &origCoinBaseMerkle[i]));
			}

			REQUIRE(auxPow.getAuxMerkleIndex() == auxPowVerify.getAuxMerkleIndex());
			REQUIRE(auxPow.getParMerkleIndex() == auxPowVerify.getParMerkleIndex());

			const UInt256 &parentHash = auxPow.getParentHash();
			const UInt256 &parentHashVerify = auxPowVerify.getParentHash();
			REQUIRE(UInt256Eq(&parentHash, &parentHashVerify));

			// verify transaction
			BRTransaction *origTxn = auxPow.getBTCTransaction();
			BRTransaction *txn = auxPowVerify.getBTCTransaction();
			if (checkAll) {
				REQUIRE(origTxn->blockHeight == txn->blockHeight);
				REQUIRE(UInt256Eq(&origTxn->txHash, &txn->txHash));
			}
			REQUIRE(origTxn->version == txn->version);
			REQUIRE(origTxn->inCount == txn->inCount);
			for (size_t i = 0; i < txn->inCount; ++i) {
				REQUIRE(UInt256Eq(&origTxn->inputs[i].txHash, &txn->inputs[i].txHash));
				REQUIRE(origTxn->inputs[i].index == txn->inputs[i].index);
				if (checkAll) {
					REQUIRE(0 == strcmp(origTxn->inputs[i].address, txn->inputs[i].address));
				}
				if (checkAll) {
					REQUIRE(origTxn->inputs[i].amount == txn->inputs[i].amount);
					REQUIRE(origTxn->inputs[i].scriptLen == txn->inputs[i].scriptLen);
					REQUIRE(0 == memcmp(origTxn->inputs[i].script, txn->inputs[i].script, txn->inputs[i].scriptLen));
				}
				REQUIRE(origTxn->inputs[i].sigLen == txn->inputs[i].sigLen);
				REQUIRE(0 == memcmp(origTxn->inputs[i].signature, txn->inputs[i].signature, txn->inputs[i].sigLen));
				REQUIRE(origTxn->inputs[i].sequence == txn->inputs[i].sequence);
			}
			REQUIRE(origTxn->outCount == txn->outCount);
			for (size_t i = 0; i < txn->outCount; ++i) {
				REQUIRE(0 == strcmp(origTxn->outputs[i].address, txn->outputs[i].address));
				REQUIRE(origTxn->outputs[i].amount == txn->outputs[i].amount);
				REQUIRE(origTxn->outputs[i].scriptLen == txn->outputs[i].scriptLen);
				REQUIRE(0 == memcmp(origTxn->outputs[i].script, txn->outputs[i].script, txn->outputs[i].scriptLen));
			}
			REQUIRE(origTxn->lockTime == txn->lockTime);
			if (checkAll) {
				REQUIRE(origTxn->blockHeight == txn->blockHeight);
				REQUIRE(origTxn->timestamp == txn->timestamp);
			}

			// verify merkle block
			BRMerkleBlock *origBlock = auxPow.getParBlockHeader();
			BRMerkleBlock *blockVerify = auxPowVerify.getParBlockHeader();
			if (checkAll) {
				REQUIRE(UInt256Eq(&origBlock->blockHash, &blockVerify->blockHash));
			}
			REQUIRE(origBlock->version == blockVerify->version);
			REQUIRE(UInt256Eq(&origBlock->prevBlock, &blockVerify->prevBlock));
			REQUIRE(UInt256Eq(&origBlock->merkleRoot, &blockVerify->merkleRoot));
			REQUIRE(origBlock->timestamp == blockVerify->timestamp);
			REQUIRE(origBlock->target == blockVerify->target);
			REQUIRE(origBlock->nonce == blockVerify->nonce);
			if (checkAll) {
				REQUIRE(origBlock->totalTx == blockVerify->totalTx);
				REQUIRE(origBlock->hashesCount == blockVerify->hashesCount);
				for (size_t i = 0; i < origBlock->hashesCount; ++i) {
					REQUIRE(UInt256Eq(origBlock->hashes + i, blockVerify->hashes + i));
				}
				REQUIRE(origBlock->flagsLen == blockVerify->flagsLen);
				for (size_t i = 0; i < origBlock->flagsLen; ++i) {
					REQUIRE(origBlock->flags[i] == blockVerify->flags[i]);
				}
				REQUIRE(origBlock->height == blockVerify->height);
			}
		}

		static ELAMerkleBlock *createELAMerkleBlock() {
			ELAMerkleBlock *block = ELAMerkleBlockNew();

			block->raw.height = (uint32_t)rand();
			block->raw.timestamp = (uint32_t)rand();
			block->raw.version = (uint32_t)rand();

			block->raw.flagsLen = 10;
			block->raw.flags = (uint8_t *)malloc(block->raw.flagsLen);
			for (size_t i = 0; i < block->raw.flagsLen; ++i) {
				block->raw.flags[i] = (uint8_t)rand();
			}

			block->raw.hashesCount = 10;
			block->raw.hashes = (UInt256 *) malloc(sizeof(UInt256) * block->raw.hashesCount);
			for (size_t i = 0; i < block->raw.hashesCount; ++i) {
				block->raw.hashes[i] = getRandUInt256();
			}
			block->raw.merkleRoot = getRandUInt256();
			block->raw.nonce = (uint32_t)rand();
			block->raw.prevBlock = getRandUInt256();
			block->raw.target = (uint32_t)rand();
			block->raw.totalTx = (uint32_t)rand();

			std::vector<UInt256> hashes;
			for (size_t i = 0; i < 10; ++i) {
				hashes.push_back(getRandUInt256());
			}
			block->auxPow.setAuxMerkleBranch(hashes);

			hashes.clear();
			for (size_t i = 0; i < 10; ++i) {
				hashes.push_back(getRandUInt256());
			}
			block->auxPow.setCoinBaseMerkle(hashes);
			block->auxPow.setAuxMerkleIndex(rand());

			BRTransaction *tx = BRTransactionNew();
			tx->txHash = getRandUInt256();
			tx->version = (uint32_t)rand();
			for (size_t i = 0; i < 10; ++i) {
				CMBlock script = getRandCMBlock(25);
				CMBlock signature = getRandCMBlock(35);
				BRTransactionAddInput(tx, getRandUInt256(), (uint32_t)rand(), (uint64_t)rand(), script, script.GetSize(), signature, signature.GetSize(), (uint32_t)rand());
			}
			for (size_t i = 0; i < 10; ++i) {
				CMBlock script = getRandCMBlock(25);
				BRTransactionAddOutput(tx, rand(), script, script.GetSize());
			}
			tx->lockTime = rand();
			tx->blockHeight = rand();
			tx->timestamp = rand();
			block->auxPow.setBTCTransaction(tx);

			return block;
		}

		static void verifyELAMerkleBlock(ELAMerkleBlock *newBlock, ELAMerkleBlock *block) {

			REQUIRE(0 == memcmp(&newBlock->raw.blockHash, &block->raw.blockHash, sizeof(UInt256)));
			REQUIRE(newBlock->raw.height == block->raw.height);
			REQUIRE(newBlock->raw.timestamp == block->raw.timestamp);
			REQUIRE(newBlock->raw.version == block->raw.version);
			REQUIRE(newBlock->raw.flagsLen == block->raw.flagsLen);
			for (size_t i = 0; i < block->raw.flagsLen; ++i) {
				REQUIRE(newBlock->raw.flags[i] == block->raw.flags[i]);
			}
			REQUIRE(newBlock->raw.hashesCount == block->raw.hashesCount);
			for (size_t i = 0; i < block->raw.hashesCount; ++i) {
				REQUIRE(0 == memcmp(&newBlock->raw.hashes[i], &block->raw.hashes[i], sizeof(UInt256)));
			}
			REQUIRE(0 == memcmp(&newBlock->raw.hashes[1], &block->raw.hashes[1], sizeof(UInt256)));
			REQUIRE(0 == memcmp(&newBlock->raw.hashes[2], &block->raw.hashes[2], sizeof(UInt256)));
			REQUIRE(0 == memcmp(&newBlock->raw.merkleRoot, &block->raw.merkleRoot, sizeof(UInt256)));
			REQUIRE(newBlock->raw.nonce == block->raw.nonce);
			REQUIRE(0 == memcmp(&newBlock->raw.prevBlock, &block->raw.prevBlock, sizeof(UInt256)));
			REQUIRE(newBlock->raw.target == block->raw.target);
			REQUIRE(newBlock->raw.totalTx == block->raw.totalTx);

			verrifyAuxPowEqual(newBlock->auxPow, block->auxPow, false);
		}

	}
}

#endif //__ELASTOS_SDK_TESTHELPER_H__
