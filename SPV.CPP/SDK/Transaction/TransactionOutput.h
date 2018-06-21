// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONOUTPUT_H__
#define __ELASTOS_SDK_TRANSACTIONOUTPUT_H__

#include <boost/shared_ptr.hpp>

#include "BRTransaction.h"
#include "SDK/ELACoreExt/ELATxOutput.h"
#include "Wrapper.h"
#include "CMemBlock.h"
#include "ELAMessageSerializable.h"

namespace Elastos {
	namespace ElaWallet {

		class TransactionOutput :
				public Wrapper<BRTxOutput>,
				public ELAMessageSerializable {

		public:

			TransactionOutput();

			explicit TransactionOutput(ELATxOutput *output);

			TransactionOutput(const TransactionOutput &output);

			TransactionOutput(uint64_t amount, const CMBlock &script);

			~TransactionOutput();

			virtual std::string toString() const;

			virtual BRTxOutput *getRaw() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			std::string getAddress() const;

			void setAddress(const std::string &address);

			uint64_t getAmount() const;

			void setAmount(uint64_t amount);

			CMBlock getScript() const;

			const UInt256 &getAssetId() const;

			void setAssetId(const UInt256 &assetId);

			uint32_t getOutputLock() const;

			void setOutputLock(uint32_t outputLock);

			const UInt168 &getProgramHash() const;

			void setProgramHash(const UInt168 &hash);

			virtual nlohmann::json toJson();

			virtual void fromJson(nlohmann::json jsonData);

		private:
			ELATxOutput *_output;
		};

		typedef boost::shared_ptr<TransactionOutput> TransactionOutputPtr;

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONOUTPUT_H__
