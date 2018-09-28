// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONINPUT_H__
#define __ELASTOS_SDK_TRANSACTIONINPUT_H__

#include <string>
#include <nlohmann/json.hpp>

#include "BRInt.h"
#include "CMemBlock.h"
#include "ByteStream.h"

namespace Elastos {
	namespace ElaWallet {

		class TransactionInput {
		public:
			TransactionInput();

			TransactionInput(const UInt256 &txHash, uint32_t index);

			~TransactionInput();

			const UInt256 &getTransctionHash() const;

			void setTransactionHash(const UInt256 &hash);

			uint32_t getIndex() const;

			void setIndex(uint32_t index);

			uint32_t getSequence() const;

			void setSequence(uint32_t sequence);

			void Serialize(ByteStream &ostream) const;

			bool Deserialize(ByteStream &istream);

			nlohmann::json toJson() const;

			void fromJson(const nlohmann::json &jsonData);

			size_t getSize() const;

		private:
			UInt256 _txHash;
			uint32_t _index;
			uint32_t _sequence;
		};

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONINPUT_H__
