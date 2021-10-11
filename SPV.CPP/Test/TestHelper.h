// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TESTHELPER_H__
#define __ELASTOS_SDK_TESTHELPER_H__

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/TransactionInput.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Transaction/Attribute.h>
#include <Plugin/Transaction/Program.h>
#include <Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>
#include <Plugin/Transaction/Payload/OutputPayload/PayloadDefault.h>
#include <Common/BigInt.h>
#include <support/BRInt.h>

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

		static bytes_ptr getRandBytesPtr(size_t size) {
			bytes_ptr data(new bytes_t(size));

			for (size_t i = 0; i < size; ++i)
				(*data)[i] = (uint8_t) rand();

			return data;
		}

		static uint256 getRanduint256(void) {
			return uint256(getRandBytes(32));
		}

		static BigInt getRandBigInt() {
			BigInt bg;
			bg.setUint64(rand());
			return bg;
		}

		static std::string getRandHexString(size_t length) {
			char buf[length];
			for (size_t i = 0; i < length; ) {
				char ch = rand();
				if (isxdigit(ch)) {
					buf[i++] = tolower(ch);
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
			std::vector<uint8_t> prefixs = {PrefixStandard, PrefixMultiSign, PrefixCrossChain, PrefixCRExpenses, PrefixDeposit, PrefixIDChain, PrefixDestroy};
			uint168 programHash(getRandBytes(21));
			uint8_t index = getRandUInt8() % prefixs.size();
			*programHash.begin() = prefixs[index];
			return programHash;
		}

		static void initTransaction(Transaction &tx, const Transaction::TxVersion &version) {
			tx.SetVersion(version);
			tx.SetLockTime(getRandUInt32());
			tx.SetBlockHeight(getRandUInt32());
			tx.SetTimestamp(getRandUInt32());
			tx.SetPayloadVersion(getRandUInt8());
			tx.SetFee(getRandUInt64());

			for (size_t i = 0; i < 20; ++i) {
				uint168 programHash = getRandUInt168();
				*programHash.begin() = Prefix::PrefixStandard;
				InputPtr input(new TransactionInput(getRanduint256(), getRandUInt16()));
				tx.AddInput(input);
			}

			for (size_t i = 0; i < 20; ++i) {
				Address addr(getRandUInt168());
				OutputPtr output(new TransactionOutput(getRandBigInt(), addr, getRanduint256()));
				output->SetOutputLock(getRandUInt32());
				if (version >= Transaction::TxVersion::V09) {
					output->SetType(TransactionOutput::Type(i % 2));
					if (output->GetType() == TransactionOutput::VoteOutput) {
						std::vector<CandidateVotes> candidates;
						uint8_t v = rand() % 2;
						for (size_t i = 0; i < 50; ++i) {
							candidates.push_back(CandidateVotes(getRandBytes(33), v == VOTE_PRODUCER_CR_VERSION ? getRandUInt64() : 0));
						}

						VoteContent vc(VoteContent::Delegate, candidates);
						output->SetPayload(OutputPayloadPtr(new PayloadVote({vc}, v)));
					} else {
						output->SetPayload(OutputPayloadPtr(new PayloadDefault()));
					}
				}
				tx.AddOutput(output);
			}

			for (size_t i = 0; i < 20; ++i) {
				bytes_t data = getRandBytes(25);
				tx.AddAttribute(AttributePtr(new Attribute(Attribute::Script, data)));
			}

			for (size_t i = 0; i < 20; ++i) {
				bytes_t code = getRandBytes(25);
				bytes_t parameter = getRandBytes(25);
				tx.AddProgram(ProgramPtr(new Program(code, parameter)));
			}
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
				InputPtr in1, in2;
				in1 = tx1.GetInputs()[i];
				in2 = tx2.GetInputs()[i];
				REQUIRE(in1->TxHash() == in2->TxHash());
				REQUIRE(in1->Index() == in2->Index());
				REQUIRE(in1->Sequence() == in2->Sequence());
			}

			REQUIRE(tx1.GetOutputs().size() == tx2.GetOutputs().size());
			for (size_t i = 0; i < tx2.GetOutputs().size(); ++i) {
				OutputPtr o1, o2;
				o1 = tx1.GetOutputs()[i];
				o2 = tx2.GetOutputs()[i];
				REQUIRE(o2->AssetID() == o1->AssetID());
				REQUIRE(o2->GetAddress() == o1->GetAddress());
				REQUIRE(o2->OutputLock() == o1->OutputLock());
				REQUIRE(o2->Amount() == o1->Amount());

				REQUIRE(o1->GetType() == o2->GetType());
				OutputPayloadPtr p1 = o1->GetPayload();
				OutputPayloadPtr p2 = o2->GetPayload();
				if (o1->GetType() == TransactionOutput::VoteOutput) {
					const PayloadVote *pv1 = dynamic_cast<const PayloadVote *>(p1.get());
					const PayloadVote *pv2 = dynamic_cast<const PayloadVote *>(p2.get());
					REQUIRE(pv1 != nullptr);
					REQUIRE(pv2 != nullptr);
					const std::vector<VoteContent> &vc1 = pv1->GetVoteContent();
					const std::vector<VoteContent> &vc2 = pv2->GetVoteContent();
					REQUIRE(vc1.size() == vc2.size());

					for (size_t j = 0; j < vc1.size(); ++j) {
						REQUIRE(vc1[j].GetType() == vc2[j].GetType());
						const std::vector<CandidateVotes> &cand1 = vc1[j].GetCandidateVotes();
						const std::vector<CandidateVotes> &cand2 = vc2[j].GetCandidateVotes();

						REQUIRE(cand1.size() == cand2.size());
						for (size_t k = 0; k < cand1.size(); ++k) {
							REQUIRE(cand1[k].GetCandidate() == cand2[k].GetCandidate());
							REQUIRE(cand1[k].GetVotes() == cand2[k].GetVotes());
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
				AttributePtr attr1, attr2;
				attr1 = tx1.GetAttributes()[i];
				attr2 = tx2.GetAttributes()[i];
				REQUIRE(attr1->GetUsage() == attr2->GetUsage());
				REQUIRE((attr1->GetData() == attr2->GetData()));
			}

			REQUIRE(tx1.GetPrograms().size() == tx2.GetPrograms().size());
			for (size_t i = 0; i < tx2.GetPrograms().size(); ++i) {
				ProgramPtr p1, p2;
				p1 = tx1.GetPrograms()[i];
				p2 = tx2.GetPrograms()[i];
				REQUIRE((p1->GetCode() == p2->GetCode()));
				REQUIRE((p1->GetParameter() == p2->GetParameter()));
			}
		}

	}
}

#endif //__ELASTOS_SDK_TESTHELPER_H__
