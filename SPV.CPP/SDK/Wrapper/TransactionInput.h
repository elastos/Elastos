// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONINPUT_H__
#define __ELASTOS_SDK_TRANSACTIONINPUT_H__

#include <boost/shared_ptr.hpp>

#include "BRTransaction.h"

#include "Wrapper.h"
#include "ByteData.h"
#include "ELAMessageSerializable.h"

namespace Elastos {
	namespace SDK {

		class TransactionInput :
			public Wrapper<BRTxInput>,
			public ELAMessageSerializable {

		public:
			TransactionInput();

			TransactionInput(BRTxInput *input);

			TransactionInput(UInt256 hash, uint32_t index, uint64_t amount,
							 ByteData script, ByteData signature, uint32_t sequence);

			virtual std::string toString() const;

			virtual BRTxInput *getRaw() const;

			virtual void Serialize(std::istream &istream) const;

			virtual void Deserialize(std::ostream &ostream);

			std::string getAddress() const;

			void setAddress(std::string address);

			UInt256 getHash() const;

			uint32_t getIndex() const;

			uint64_t getAmount() const;

			ByteData getScript() const;

			ByteData getSignature() const;

			uint32_t getSequence() const;

		private:
			boost::shared_ptr<BRTxInput> _input;
		};

		typedef boost::shared_ptr<TransactionInput> TransactionInputPtr;

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONINPUT_H__
