// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDTRANSACTION_H__
#define __ELASTOS_SDK_IDTRANSACTION_H__

#include "Transaction.h"

namespace Elastos {
	namespace ElaWallet {

		class IDTransaction : public Transaction {
		public:
			enum {
				registerIdentification = 0x09, // deprecated
				didTransaction         = 0x0A,
			};

		public:
			IDTransaction();

			IDTransaction(uint8_t type, const PayloadPtr &payload);

			IDTransaction(const IDTransaction &tx);

			IDTransaction &operator=(const IDTransaction &tx);

			virtual ~IDTransaction();

			virtual bool DeserializeType(const ByteStream &istream);

			virtual bool IsDPoSTransaction() const;

			virtual bool IsCRCTransaction() const;

			virtual bool IsProposalTransaction() const;

			virtual bool IsIDTransaction() const;
		public:
			virtual PayloadPtr InitPayload(uint8_t type);

		};

	}
}


#endif //__ELASTOS_SDK_IDTRANSACTION_H__
