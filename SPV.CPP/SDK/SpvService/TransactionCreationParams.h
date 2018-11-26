// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONCREATIONPARAM_H__
#define __ELASTOS_SDK_TRANSACTIONCREATIONPARAM_H__

#include <SDK/Common/CMemBlock.h>
#include <SDK/Implement/SubWalletType.h>
#include <SDK/ELACoreExt/Payload/PayloadRegisterProducer.h>
#include <SDK/ELACoreExt/Payload/PayloadCancelProducer.h>
#include <SDK/ELACoreExt/Payload/PayloadVoteProducer.h>

#include <Core/BRInt.h>

#include <string>
#include <map>

namespace Elastos {
	namespace ElaWallet {

		class TxParam {
		public:
			TxParam();

			virtual ~TxParam();

			const std::string &getFromAddress() const;

			void setFromAddress(const std::string &address);

			const std::string &getToAddress() const;

			void setToAddress(const std::string &address);

			uint64_t getAmount() const;

			void setAmount(uint64_t amount);

			const UInt256 &getAssetId() const;

			void setAssetId(const UInt256 &id);

			uint64_t getFee() const;

			void setFee(uint64_t fee);

			virtual SubWalletType getType() const { return Normal; }

			const std::string &getRemark() const;

			void setRemark(const std::string &remark);

			const std::string &getMemo() const;

			void setMemo(const std::string &memo);

		private:
			std::string _fromAddress;
			std::string _toAddress;
			uint64_t _amount;
			uint64_t _fee;
			UInt256 _assetId;
			std::string _remark;
			std::string _memo;
		};

		class DepositTxParam : public TxParam {
		public:
			std::string getSidechainAddress() const;

			void setSidechainAddress(const std::string &address);

			const std::vector<std::string> &getCrossChainAddress() const;

			const std::vector<uint64_t> &getCrossChainOutputIndexs() const;

			const std::vector<uint64_t> &getCrosschainAmouts() const;

			void setSidechainDatas(const std::vector<std::string> &crossChainAddress,
								   const std::vector<uint64_t> &outputIndex,
								   const std::vector<uint64_t> &crossChainAmount);

			virtual SubWalletType getType() const { return Mainchain; }

		private:
			std::string _sidechainAddress;
			std::vector<std::string> _crossChainAddress;
			std::vector<uint64_t> _outputIndex;
			std::vector<uint64_t> _crossChainAmount;
		};

		class WithdrawTxParam : public TxParam {
		public:
			std::string getMainchainAddress() const;

			void setMainchainAddress(const std::string &address);

			const std::vector<std::string> &getCrossChainAddress() const;

			const std::vector<uint64_t> &getCrossChainOutputIndexs() const;

			const std::vector<uint64_t> &getCrosschainAmouts() const;

			void setMainchainDatas(const std::vector<std::string> &crossChainAddress,
								   const std::vector<uint64_t> &outputIndex,
								   const std::vector<uint64_t> &crossChainAmount);

			virtual SubWalletType getType() const { return Sidechain; }

		private:
			std::string _mainchainAddress;
			std::vector<std::string> _crossChainAddress;
			std::vector<uint64_t> _outputIndex;
			std::vector<uint64_t> _crossChainAmount;
		};

		class IdTxParam : public TxParam {
		public:
			std::string getId() const;

			void setId(const std::string &id);

			const CMBlock &getData() const;

			void setData(const CMBlock &data);

			virtual SubWalletType getType() const { return Idchain; }

		private:
			std::string _id;
			CMBlock _data;
		};

		class RegisterProducerTxParam : public TxParam {
		public:
			const PayloadRegisterProducer &GetPayload() const;

			void SetPayload(const PayloadRegisterProducer &payload);

		private:
			PayloadRegisterProducer _payload;
		};

		class CancelProducerTxParam : public TxParam {
		public:
			const PayloadCancelProducer &GetPayload() const;

			void SetPayload(const PayloadCancelProducer &payload);

		private:
			PayloadCancelProducer _payload;
		};

		class VoteProducerTxParam : public TxParam {
		public:
			const PayloadVoteProducer &GetPayload() const;

			void SetPayload(const PayloadVoteProducer &payload);

		private:
			PayloadVoteProducer _payload;
		};

		class TxParamFactory {
		public:
			static TxParam *
			createTxParam(SubWalletType type, const std::string &fromAddress, const std::string &toAddress,
						  uint64_t amount, uint64_t fee, const std::string &memo, const std::string &remark,
						  const UInt256 &assetID);
		};
	}
}

#endif //__ELASTOS_SDK_TRANSACTIONCREATIONPARAM_H__
