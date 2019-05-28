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

		static bytes_t getRandBytes(size_t size) {
			bytes_t block(size);

			for (size_t i = 0; i < size; ++i) {
				block[i] = (uint8_t) rand();
			}

			return block;
		}

		static uint256 getRanduint256(void) {
			return uint256(getRandBytes(32));
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

		static uint128 getRandUInt128(void) {
			return uint128(getRandBytes(16));
		}

		static uint168 getRandUInt168(void) {
			return uint168(getRandBytes(21));
		}

		static void initTransaction(Transaction &tx, const Transaction::TxVersion &version) {
			tx.SetVersion(version);
			tx.SetLockTime(getRandUInt32());
			tx.SetBlockHeight(getRandUInt32());
			tx.SetTimestamp(getRandUInt32());
			tx.SetTransactionType(Transaction::TransferAsset);
			tx.SetPayloadVersion(getRandUInt8());
			tx.SetFee(getRandUInt64());

			for (size_t i = 0; i < 20; ++i) {
				TransactionInput input;
				input.SetTransactionHash(getRanduint256());
				input.SetIndex(getRandUInt16());
				input.SetSequence(getRandUInt32());
				tx.AddInput(input);
			}

			for (size_t i = 0; i < 20; ++i) {
				TransactionOutput output;
				output.SetAmount(getRandUInt64());
				output.SetAssetID(getRanduint256());
				output.SetOutputLock(getRandUInt32());
				output.SetProgramHash(getRandUInt168());
				if (version >= Transaction::TxVersion::V09) {
					output.SetType(TransactionOutput::Type(i % 2));
					if (output.GetType() == TransactionOutput::VoteOutput) {
						std::vector<bytes_t> candidates;
						for (size_t i = 0; i < 50; ++i) {
							candidates.push_back(getRandBytes(33));
						}
						PayloadVote::VoteContent vc(PayloadVote::Delegate, candidates);
						output.SetPayload(OutputPayloadPtr(new PayloadVote({vc})));
					} else {
						output.SetPayload(OutputPayloadPtr(new PayloadDefault()));
					}
				}
				tx.AddOutput(output);
			}

			for (size_t i = 0; i < 20; ++i) {
				bytes_t data = getRandBytes(25);
				tx.AddAttribute(Attribute(Attribute::Script, data));
			}

			for (size_t i = 0; i < 20; ++i) {
				bytes_t code = getRandBytes(25);
				bytes_t parameter = getRandBytes(25);
				tx.AddProgram(Program(code, parameter));
			}

			tx.GetHash();
		}

		static void verifyTransaction(const Transaction &tx1, const Transaction &tx2, bool checkAll = true) {
			REQUIRE(tx1.GetLockTime() == tx2.GetLockTime());
			REQUIRE(tx1.GetTransactionType() == tx2.GetTransactionType());
			REQUIRE(tx1.GetPayloadVersion() == tx2.GetPayloadVersion());
			REQUIRE(tx1.GetVersion() == tx2.GetVersion());
			if (checkAll) {
				REQUIRE(tx1.GetBlockHeight() == tx2.GetBlockHeight());
				REQUIRE(tx1.GetTimestamp() == tx2.GetTimestamp());
				REQUIRE(tx1.GetFee() == tx2.GetFee());
			}

			REQUIRE(tx1.GetOutputs().size() == tx2.GetOutputs().size());
			REQUIRE(tx1.GetHash() == tx2.GetHash());
			REQUIRE(tx1.GetInputs().size() == tx2.GetInputs().size());
			for (size_t i = 0; i < tx1.GetInputs().size(); ++i) {
				TransactionInput in1, in2;
				in1 = tx1.GetInputs()[i];
				in2 = tx2.GetInputs()[i];
				REQUIRE(in1.GetTransctionHash() == in2.GetTransctionHash());
				REQUIRE(in1.GetIndex() == in2.GetIndex());
				REQUIRE(in1.GetSequence() == in2.GetSequence());
			}

			REQUIRE(tx1.GetOutputs().size() == tx2.GetOutputs().size());
			for (size_t i = 0; i < tx2.GetOutputs().size(); ++i) {
				TransactionOutput o1, o2;
				o1 = tx1.GetOutputs()[i];
				o2 = tx2.GetOutputs()[i];
				REQUIRE(o2.GetAssetID() == o1.GetAssetID());
				REQUIRE(o2.GetProgramHash() == o1.GetProgramHash());
				REQUIRE(o2.GetOutputLock() == o1.GetOutputLock());
				REQUIRE(o2.GetAmount() == o1.GetAmount());

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
						const std::vector<bytes_t> &cand1 = vc1[j].candidates;
						const std::vector<bytes_t> &cand2 = vc2[j].candidates;

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

			REQUIRE(tx1.GetAttributes().size() == tx2.GetAttributes().size());
			for (size_t i = 0; i < tx1.GetAttributes().size(); ++i) {
				Attribute attr1, attr2;
				attr1 = tx1.GetAttributes()[i];
				attr2 = tx2.GetAttributes()[i];
				REQUIRE(attr1.GetUsage() == attr2.GetUsage());
				REQUIRE((attr1.GetData() == attr2.GetData()));
			}

			REQUIRE(tx1.GetPrograms().size() == tx2.GetPrograms().size());
			for (size_t i = 0; i < tx2.GetPrograms().size(); ++i) {
				Program p1, p2;
				p1 = tx1.GetPrograms()[i];
				p2 = tx2.GetPrograms()[i];
				REQUIRE((p1.GetCode() == p2.GetCode()));
				REQUIRE((p1.GetParameter() == p2.GetParameter()));
			}
		}


		static AuxPow createDummyAuxPow() {
			AuxPow auxPow;

			std::vector<uint256> hashes(3);
			for (size_t i = 0; i < hashes.size(); ++i) {
				hashes[i] = getRanduint256();
			}
			auxPow.SetAuxMerkleBranch(hashes);

			for (size_t i = 0; i < hashes.size(); ++i) {
				hashes[i] = getRanduint256();
			}
			auxPow.SetCoinBaseMerkle(hashes);

			auxPow.SetAuxMerkleIndex(123);
			auxPow.SetParMerkleIndex(456);

			auxPow.SetParentHash(getRanduint256());

			// init transaction
			BRTransaction *tx = BRTransactionNew();
			tx->txHash = getRandUInt256();
			tx->version = rand();
			for (size_t i = 0; i < 3; ++i) {
				bytes_t script = getRandBytes(25);
				bytes_t signature = getRandBytes(28);
				BRTransactionAddInput(tx, getRandUInt256(), i, rand(),
									  &script[0], script.size(), &signature[0], signature.size(), rand());
			}
			for (size_t i = 0; i < 4; ++i) {
				bytes_t script = getRandBytes(25);
				BRTransactionAddOutput(tx, rand(), &script[0], script.size());
			}
			tx->lockTime = rand();
			tx->blockHeight = rand();
			tx->timestamp = rand();
			auxPow.SetBTCTransaction(tx);

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
			bytes_t flags = getRandBytes(5);
			block->flagsLen = flags.size();
			BRMerkleBlockSetTxHashes(block, MBHashes, ARRAY_SIZE(MBHashes), &flags[0], flags.size());
			block->height = rand();
			auxPow.SetParBlockHeader(block);

			return auxPow;
		}

		static void verrifyAuxPowEqual(const AuxPow &auxPow, const AuxPow &auxPowVerify, bool checkAll = true) {
			const std::vector<uint256> &origMerkleBranch = auxPow.GetAuxMerkleBranch();
			const std::vector<uint256> &merkleBranch = auxPowVerify.GetAuxMerkleBranch();
			REQUIRE(merkleBranch.size() == origMerkleBranch.size());
			for (size_t i = 0; i < merkleBranch.size(); ++i) {
				REQUIRE(merkleBranch[i] == origMerkleBranch[i]);
			}

			const std::vector<uint256> &origCoinBaseMerkle = auxPow.GetParCoinBaseMerkle();
			const std::vector<uint256> &coinBaseMerkle = auxPowVerify.GetParCoinBaseMerkle();
			REQUIRE(origCoinBaseMerkle.size() == coinBaseMerkle.size());
			for (size_t i = 0; i < coinBaseMerkle.size(); ++i) {
				REQUIRE(coinBaseMerkle[i] == origCoinBaseMerkle[i]);
			}

			REQUIRE(auxPow.GetAuxMerkleIndex() == auxPowVerify.GetAuxMerkleIndex());
			REQUIRE(auxPow.GetParMerkleIndex() == auxPowVerify.GetParMerkleIndex());

			const uint256 &parentHash = auxPow.GetParentHash();
			const uint256 &parentHashVerify = auxPowVerify.GetParentHash();
			REQUIRE(parentHash == parentHashVerify);

			// verify transaction
			BRTransaction *origTxn = auxPow.GetBTCTransaction();
			BRTransaction *txn = auxPowVerify.GetBTCTransaction();
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
			BRMerkleBlock *origBlock = auxPow.GetParBlockHeader();
			BRMerkleBlock *blockVerify = auxPowVerify.GetParBlockHeader();
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
			block->SetHeight((uint32_t) rand());
			block->SetTimestamp((uint32_t) rand());
			block->SetVersion((uint32_t) rand());

			std::vector<uint8_t> flags;
			for (size_t i = 0; i < 10; ++i) {
				flags.push_back((uint8_t) rand());
			}

			std::vector<uint256> hashes;
			for (size_t i = 0; i < 10; ++i) {
				hashes.push_back(getRanduint256());
			}

			block->SetRootBlockHash(getRanduint256());
			block->SetNonce((uint32_t) rand());
			block->SetPrevBlockHash(getRanduint256());
			block->SetTarget((uint32_t) rand());
			block->SetTransactionCount((uint32_t) rand());

			AuxPow auxPow;
			hashes.clear();
			for (size_t i = 0; i < 10; ++i) {
				hashes.push_back(getRanduint256());
			}
			auxPow.SetAuxMerkleBranch(hashes);

			hashes.clear();
			for (size_t i = 0; i < 10; ++i) {
				hashes.push_back(getRanduint256());
			}
			auxPow.SetCoinBaseMerkle(hashes);
			auxPow.SetAuxMerkleIndex(rand());

			BRTransaction *tx = BRTransactionNew();
			tx->txHash = getRandUInt256();
			tx->version = (uint32_t) rand();
			for (size_t i = 0; i < 10; ++i) {
				bytes_t script = getRandBytes(25);
				bytes_t signature = getRandBytes(35);
				BRTransactionAddInput(tx, getRandUInt256(), (uint32_t) rand(), (uint64_t) rand(), &script[0],
									  script.size(), &signature[0], signature.size(), (uint32_t) rand());
			}
			for (size_t i = 0; i < 10; ++i) {
				bytes_t script = getRandBytes(25);
				BRTransactionAddOutput(tx, rand(), &script[0], script.size());
			}
			tx->lockTime = rand();
			tx->blockHeight = rand();
			tx->timestamp = rand();
			auxPow.SetBTCTransaction(tx);
			block->SetAuxPow(auxPow);
		}

		static void verifyELAMerkleBlock(const MerkleBlock &newBlock, const MerkleBlock &block) {

			REQUIRE(newBlock.GetHash() == block.GetHash());
			REQUIRE(newBlock.GetHeight() == block.GetHeight());
			REQUIRE(newBlock.GetTimestamp() == block.GetTimestamp());
			REQUIRE(newBlock.GetVersion() == block.GetVersion());
			REQUIRE(newBlock.GetFlags().size() == block.GetFlags().size());
			for (size_t i = 0; i < block.GetFlags().size(); ++i) {
				REQUIRE(newBlock.GetFlags()[i] == block.GetFlags()[i]);
			}
			REQUIRE(newBlock.GetHashes().size() == block.GetHashes().size());
			for (size_t i = 0; i < block.GetHashes().size(); ++i) {
				REQUIRE(newBlock.GetHashes()[i] == block.GetHashes()[i]);
			}
			REQUIRE(newBlock.GetRootBlockHash() == block.GetRootBlockHash());
			REQUIRE(newBlock.GetNonce() == block.GetNonce());
			REQUIRE(newBlock.GetPrevBlockHash() == block.GetPrevBlockHash());
			REQUIRE(newBlock.GetTarget() == block.GetTarget());
			REQUIRE(newBlock.GetTransactionCount() == block.GetTransactionCount());

			verrifyAuxPowEqual(newBlock.GetAuxPow(), block.GetAuxPow(), false);
		}

	}
}

#endif //__ELASTOS_SDK_TESTHELPER_H__
