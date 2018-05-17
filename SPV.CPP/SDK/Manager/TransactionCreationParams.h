// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONCREATIONPARAM_H__
#define __ELASTOS_SDK_TRANSACTIONCREATIONPARAM_H__

#include <string>

#include "ByteData.h"

namespace Elastos {
	namespace SDK {

		class TxParam {
		public:
			TxParam();

			virtual ~TxParam();

			std::string getToAddress() const;

			void setToAddress(const std::string &address);

			uint64_t getAmount() const;

			void setAmount(uint64_t amount);

		private:
			std::string _toAddress;
			uint64_t _amount;
		};

		class DepositTxParam : public TxParam {
		public:
			std::string getSidechainAddress() const;

			void setSidechainAddress(const std::string &address);

		private:
			std::string _sidechainAddress;
		};

		class WithdrawTxParam : public TxParam {
		public:
			std::string getMainchainAddress() const;

			void setMainchainAddress(const std::string &address);

		private:
			std::string _mainchainAddress;
		};

		class IdTxParam : public TxParam {
		public:
			std::string getId() const;

			void setId(const std::string &id);

			const ByteData &getData() const;

			void setData(const ByteData &data);

		private:
			std::string _id;
			ByteData _data;
		};
	}
}

#endif //__ELASTOS_SDK_TRANSACTIONCREATIONPARAM_H__
