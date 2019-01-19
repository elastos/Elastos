// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TESTHELPER_H__
#define __ELASTOS_SDK_TESTHELPER_H__

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#include <SDK/Plugin/Block/AuxPow.h>
#include <SDK/Plugin/Block/MerkleBlock.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>
#include <SDK/Plugin/Transaction/Payload/OutputPayload/PayloadDefault.h>

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

		static std::string getRandHexString(size_t length) {
			char buf[length];
			for (size_t i = 0; i < length; ) {
				char ch = rand();
				if (isxdigit(ch)) {
					buf[i++] = ch;
				}
			}

			return std::string(buf, length);
		}

		static std::string getRandString(size_t length) {
			char buf[length];
			for (size_t i = 0; i < length; ) {
				char ch = rand();
				if (isalnum(ch)) {
					buf[i++] = ch;
				}
			}

			return std::string(buf, length);
		}

		static uint64_t getRandUInt64() {
			return uint64_t(rand());
		}

		static uint32_t getRandUInt32() {
			return uint32_t(rand());
		}

		static uint16_t getRandUInt16() {
			return uint16_t(rand());
		}

		static uint8_t getRandUInt8() {
			return uint8_t(rand());
		}

		static UInt168 getRandUInt168(void) {
			UInt168 u;
			for (size_t i = 0; i < ARRAY_SIZE(u.u8); ++i) {
				u.u8[i] = rand();
			}
			return u;
		}

		static void initTransaction(Transaction &tx, const Transaction::TxVersion &version) {
			tx.setVersion(version);
			tx.setLockTime(getRandUInt32());
			tx.setBlockHeight(getRandUInt32());
			tx.setTimestamp(getRandUInt32());
			tx.setTransactionType(Transaction::TransferAsset);
			tx.setPayloadVersion(getRandUInt8());
			tx.setFee(getRandUInt64());
			tx.setRemark(getRandString(40));

			for (size_t i = 0; i < 20; ++i) {
				TransactionInput input;
				input.setTransactionHash(getRandUInt256());
				input.setIndex(getRandUInt16());
				input.setSequence(getRandUInt32());
				tx.addInput(input);
			}

			for (size_t i = 0; i < 20; ++i) {
				TransactionOutput output;
				output.setAmount(getRandUInt64());
				output.setAssetId(getRandUInt256());
				output.setOutputLock(getRandUInt32());
				output.setProgramHash(getRandUInt168());
				if (version >= Transaction::TxVersion::V09) {
					output.SetType(TransactionOutput::Type(i % 2));
					if (output.GetType() == TransactionOutput::VoteOutput) {
						std::vector<CMBlock> candidates;
						for (size_t i = 0; i < 50; ++i) {
							candidates.push_back(getRandCMBlock(33));
						}
						PayloadVote::VoteContent vc(PayloadVote::Delegate, candidates);
						output.SetPayload(OutputPayloadPtr(new PayloadVote({vc})));
					} else {
						output.SetPayload(OutputPayloadPtr(new PayloadDefault()));
					}
				}
				tx.addOutput(output);
			}

			for (size_t i = 0; i < 20; ++i) {
				CMBlock data = getRandCMBlock(25);
				tx.addAttribute(Attribute(Attribute::Script, data));
			}

			for (size_t i = 0; i < 20; ++i) {
				CMBlock code = getRandCMBlock(25);
				CMBlock parameter = getRandCMBlock(25);
				tx.addProgram(Program(code, parameter));
			}

			tx.getHash();
		}

		static void verifyTransaction(const Transaction &tx1, const Transaction &tx2, bool checkAll = true) {
			REQUIRE(tx1.getLockTime() == tx2.getLockTime());
			REQUIRE(tx1.getTransactionType() == tx2.getTransactionType());
			REQUIRE(tx1.getPayloadVersion() == tx2.getPayloadVersion());
			REQUIRE(tx1.getVersion() == tx2.getVersion());
			if (checkAll) {
				REQUIRE(tx1.getBlockHeight() == tx2.getBlockHeight());
				REQUIRE(tx1.getTimestamp() == tx2.getTimestamp());
				REQUIRE(tx1.getFee() == tx2.getFee());
				REQUIRE(tx1.getRemark() == tx2.getRemark());
			}

			REQUIRE(tx1.getOutputs().size() == tx2.getOutputs().size());
			REQUIRE(UInt256Eq(&tx1.getHash(), &tx2.getHash()));
			REQUIRE(tx1.getInputs().size() == tx2.getInputs().size());
			for (size_t i = 0; i < tx1.getInputs().size(); ++i) {
				TransactionInput in1, in2;
				in1 = tx1.getInputs()[i];
				in2 = tx2.getInputs()[i];
				REQUIRE(UInt256Eq(&in1.getTransctionHash(), &in2.getTransctionHash()));
				REQUIRE(in1.getIndex() == in2.getIndex());
				REQUIRE(in1.getSequence() == in2.getSequence());
			}

			REQUIRE(tx1.getOutputs().size() == tx2.getOutputs().size());
			for (size_t i = 0; i < tx2.getOutputs().size(); ++i) {
				TransactionOutput o1, o2;
				o1 = tx1.getOutputs()[i];
				o2 = tx2.getOutputs()[i];
				REQUIRE(UInt256Eq(&o2.getAssetId(), &o1.getAssetId()));
				REQUIRE(UInt168Eq(&o2.getProgramHash(), &o1.getProgramHash()));
				REQUIRE(o2.getOutputLock() == o1.getOutputLock());
				REQUIRE(o2.getAmount() == o1.getAmount());

				REQUIRE(o1.GetType() == o2.GetType());
				OutputPayloadPtr p1 = o1.GetPayload();
				OutputPayloadPtr p2 = o2.GetPayload();
				if (o1.GetType() == TransactionOutput::VoteOutput) {
					const PayloadVote *pv1 = dynamic_cast<const PayloadVote *>(p1.get());
					const PayloadVote *pv2 = dynamic_cast<const PayloadVote *>(p2.get());
					REQUIRE(pv1 != nullptr);
					REQUIRE(pv2 != nullptr);
					const std::vector<PayloadVote::VoteContent> &vc1 = pv1->GetVoteContent();
					const std::vector<PayloadVote::VoteContent> &vc2 = pv2->GetVoteContent();
					REQUIRE(vc1.size() == vc2.size());

					for (size_t j = 0; j < vc1.size(); ++j) {
						REQUIRE(vc1[j].type == vc2[j].type);
						const std::vector<CMBlock> &cand1 = vc1[j].candidates;
						const std::vector<CMBlock> &cand2 = vc2[j].candidates;

						REQUIRE(cand1.size() == cand2.size());
						for (size_t k = 0; k < cand1.size(); ++k) {
							REQUIRE(cand1[k] == cand2[k]);
						}
					}
				} else {
					const PayloadDefault *pd1 = dynamic_cast<const PayloadDefault *>(p1.get());
					const PayloadDefault *pd2 = dynamic_cast<const PayloadDefault *>(p2.get());
					REQUIRE(pd1 != nullptr);
					REQUIRE(pd2 != nullptr);
				}
			}

			REQUIRE(tx1.getAttributes().size() == tx2.getAttributes().size());
			for (size_t i = 0; i < tx1.getAttributes().size(); ++i) {
				Attribute attr1, attr2;
				attr1 = tx1.getAttributes()[i];
				attr2 = tx2.getAttributes()[i];
				REQUIRE(attr1.GetUsage() == attr2.GetUsage());
				REQUIRE((attr1.GetData() == attr2.GetData()));
			}

			REQUIRE(tx1.getPrograms().size() == tx2.getPrograms().size());
			for (size_t i = 0; i < tx2.getPrograms().size(); ++i) {
				Program p1, p2;
				p1 = tx1.getPrograms()[i];
				p2 = tx2.getPrograms()[i];
				REQUIRE((p1.getCode() == p2.getCode()));
				REQUIRE((p1.getParameter() == p2.getParameter()));
			}
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

		static void setMerkleBlockValues(MerkleBlock *block) {
			block->setHeight((uint32_t) rand());
			block->setTimestamp((uint32_t) rand());
			block->setVersion((uint32_t) rand());

			std::vector<uint8_t> flags;
			for (size_t i = 0; i < 10; ++i) {
				flags.push_back((uint8_t) rand());
			}

			std::vector<UInt256> hashes;
			for (size_t i = 0; i < 10; ++i) {
				hashes.push_back(getRandUInt256());
			}

			block->setRootBlockHash(getRandUInt256());
			block->setNonce((uint32_t) rand());
			block->setPrevBlockHash(getRandUInt256());
			block->setTarget((uint32_t) rand());
			block->setTransactionCount((uint32_t) rand());

			AuxPow auxPow;
			hashes.clear();
			for (size_t i = 0; i < 10; ++i) {
				hashes.push_back(getRandUInt256());
			}
			auxPow.setAuxMerkleBranch(hashes);

			hashes.clear();
			for (size_t i = 0; i < 10; ++i) {
				hashes.push_back(getRandUInt256());
			}
			auxPow.setCoinBaseMerkle(hashes);
			auxPow.setAuxMerkleIndex(rand());

			BRTransaction *tx = BRTransactionNew();
			tx->txHash = getRandUInt256();
			tx->version = (uint32_t) rand();
			for (size_t i = 0; i < 10; ++i) {
				CMBlock script = getRandCMBlock(25);
				CMBlock signature = getRandCMBlock(35);
				BRTransactionAddInput(tx, getRandUInt256(), (uint32_t) rand(), (uint64_t) rand(), script,
									  script.GetSize(), signature, signature.GetSize(), (uint32_t) rand());
			}
			for (size_t i = 0; i < 10; ++i) {
				CMBlock script = getRandCMBlock(25);
				BRTransactionAddOutput(tx, rand(), script, script.GetSize());
			}
			tx->lockTime = rand();
			tx->blockHeight = rand();
			tx->timestamp = rand();
			auxPow.setBTCTransaction(tx);
			block->setAuxPow(auxPow);
		}

		static void verifyELAMerkleBlock(const MerkleBlock &newBlock, const MerkleBlock &block) {

			REQUIRE(UInt256Eq(&newBlock.getHash(), &block.getHash()));
			REQUIRE(newBlock.getHeight() == block.getHeight());
			REQUIRE(newBlock.getTimestamp() == block.getTimestamp());
			REQUIRE(newBlock.getVersion() == block.getVersion());
			REQUIRE(newBlock.getFlags().size() == block.getFlags().size());
			for (size_t i = 0; i < block.getFlags().size(); ++i) {
				REQUIRE(newBlock.getFlags()[i] == block.getFlags()[i]);
			}
			REQUIRE(newBlock.getHashes().size() == block.getHashes().size());
			for (size_t i = 0; i < block.getHashes().size(); ++i) {
				REQUIRE(UInt256Eq(&newBlock.getHashes()[i], &block.getHashes()[i]));
			}
			REQUIRE(UInt256Eq(&newBlock.getRootBlockHash(), &block.getRootBlockHash()));
			REQUIRE(newBlock.getNonce() == block.getNonce());
			REQUIRE(UInt256Eq(&newBlock.getPrevBlockHash(), &block.getPrevBlockHash()));
			REQUIRE(newBlock.getTarget() == block.getTarget());
			REQUIRE(newBlock.getTransactionCount() == block.getTransactionCount());

			verrifyAuxPowEqual(newBlock.getAuxPow(), block.getAuxPow(), false);
		}

	}
}

#endif //__ELASTOS_SDK_TESTHELPER_H__
