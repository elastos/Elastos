// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONINPUT_H__
#define __ELASTOS_SDK_TRANSACTIONINPUT_H__

#include <Common/ByteStream.h>
#include <Common/JsonSerializer.h>
#include <Plugin/Interface/ELAMessageSerializable.h>

#include <nlohmann/json.hpp>

#include <string>

namespace Elastos {
	namespace ElaWallet {

		class TransactionInput : public ELAMessageSerializable, public JsonSerializer {
		public:
			TransactionInput();

			TransactionInput(const TransactionInput &input);

			TransactionInput &operator=(const TransactionInput &tx);

			TransactionInput(const uint256 &txHash, uint16_t index);

			~TransactionInput();

			const uint256 &TxHash() const;

			void SetTxHash(const uint256 &hash);

			uint16_t Index() const;

			void SetIndex(uint16_t index);

			uint32_t Sequence() const;

			void SetSequence(uint32_t sequence);

			size_t EstimateSize() const;

			void Serialize(ByteStream &ostream) const;

			bool Deserialize(const ByteStream &istream);

			nlohmann::json ToJson() const;

			void FromJson(const nlohmann::json &j);

			size_t GetSize() const;

		private:
			uint256 _txHash;
			uint16_t _index;
			uint32_t _sequence;
		};

		typedef boost::shared_ptr<TransactionInput> InputPtr;
		typedef std::vector<InputPtr> InputArray;

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONINPUT_H__
