// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONCREATIONPARAM_H__
#define __ELASTOS_SDK_TRANSACTIONCREATIONPARAM_H__

#include <string>

#include "CMemBlock.h"

namespace Elastos {
	namespace SDK {

		enum TxType {
			Normal = 0,
			Deposit,
			Withdraw,
			ID
		};

		class TxParam {
		public:
			TxParam();

			virtual ~TxParam();

			std::string getToAddress() const;

			void setToAddress(const std::string &address);

			uint64_t getAmount() const;

			void setAmount(uint64_t amount);

			const UInt256 &getAssetId() const;

			void setAssetId(const UInt256 &id);

			virtual TxType getType() const { return Normal;}

		private:
			std::string _toAddress;
			uint64_t _amount;
			UInt256 _assetId;
		};

		class DepositTxParam : public TxParam {
		public:
			std::string getSidechainAddress() const;

			void setSidechainAddress(const std::string &address);

			virtual TxType getType() const { return Deposit;}

		private:
			std::string _sidechainAddress;
		};

		class WithdrawTxParam : public TxParam {
		public:
			std::string getMainchainAddress() const;

			void setMainchainAddress(const std::string &address);

			virtual TxType getType() const { return Withdraw;}

		private:
			std::string _mainchainAddress;
		};

		class IdTxParam : public TxParam {
		public:
			std::string getId() const;

			void setId(const std::string &id);

			const CMBlock &getData() const;

			void setData(const CMBlock &data);

			virtual TxType getType() const { return ID;}

		private:
			std::string _id;
			CMBlock _data;
		};
	}
}

#endif //__ELASTOS_SDK_TRANSACTIONCREATIONPARAM_H__
