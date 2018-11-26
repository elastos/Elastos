// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONOUTPUT_H__
#define __ELASTOS_SDK_TRANSACTIONOUTPUT_H__

#include <SDK/Plugin/Interface/ELAMessageSerializable.h>
#include <Core/BRInt.h>

#include <boost/shared_ptr.hpp>

#define TX_RECHARGE_OUTPUT_SIZE 65

namespace Elastos {
	namespace ElaWallet {

		class TransactionOutput :
				public ELAMessageSerializable {

		public:
			TransactionOutput();

			TransactionOutput(const TransactionOutput &output);

			TransactionOutput(uint64_t amount, const std::string &toAddress);

			TransactionOutput(uint64_t amount, const UInt168 &programHash);

			~TransactionOutput();

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			size_t getSize() const;

			std::string getAddress() const;

			uint64_t getAmount() const;

			void setAmount(uint64_t amount);

			const UInt256 &getAssetId() const;

			void setAssetId(const UInt256 &assetId);

			uint32_t getOutputLock() const;

			void setOutputLock(uint32_t outputLock);

			const UInt168 &getProgramHash() const;

			void setProgramHash(const UInt168 &hash);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &jsonData);

			size_t GetSize() const;

		private:
			uint64_t _amount;
			UInt256 _assetId;
			uint32_t _outputLock;
			UInt168 _programHash;
		};

		typedef boost::shared_ptr<TransactionOutput> TransactionOutputPtr;

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONOUTPUT_H__
