// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONOUTPUT_H__
#define __ELASTOS_SDK_TRANSACTIONOUTPUT_H__

#include <SDK/Plugin/Interface/ELAMessageSerializable.h>
#include <SDK/Plugin/Transaction/Payload/IPayload.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <Core/BRInt.h>

#include <boost/shared_ptr.hpp>

#define TX_RECHARGE_OUTPUT_SIZE 65

namespace Elastos {
	namespace ElaWallet {

		class TransactionOutput :
				public ELAMessageSerializable {

		public:
			enum Type {
				Default    = 0x00,
				VoteOutput = 0x01,
			};

		public:
			TransactionOutput();

			TransactionOutput(const TransactionOutput &output);

			TransactionOutput(uint64_t amount, const std::string &toAddress, const UInt256 &assetID = Asset::GetELAAssetID(),
							  Type type = Default, const PayloadPtr &payload = nullptr);

			TransactionOutput(uint64_t amount, const UInt168 &programHash, const UInt256 &assetID = Asset::GetELAAssetID(),
							  Type type = Default, const PayloadPtr &payload = nullptr);

			~TransactionOutput();

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			void Serialize(ByteStream &ostream, uint8_t txVersion) const;

			bool Deserialize(ByteStream &istream, uint8_t txVersion);

			std::string getAddress() const;

			void SetAddress(const std::string &address);

			uint64_t getAmount() const;

			void setAmount(uint64_t amount);

			const UInt256 &getAssetId() const;

			void setAssetId(const UInt256 &assetId);

			uint32_t getOutputLock() const;

			void setOutputLock(uint32_t outputLock);

			const UInt168 &getProgramHash() const;

			void setProgramHash(const UInt168 &hash);

			const Type &GetType() const;

			void SetType(const Type &type);

			const uint8_t &GetPayloadVersion() const;

			void SetPayloadVersion(const uint8_t &payloadVersion);

			const PayloadPtr &GetPayload() const;

			PayloadPtr &GetPayload();

			void SetPayload(const PayloadPtr &payload);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &j);

			nlohmann::json toJson(uint8_t txVersion) const;

			void fromJson(const nlohmann::json &j, uint8_t txVersion);

			size_t GetSize() const;

		private:
			uint64_t _amount;
			UInt256 _assetId;
			uint32_t _outputLock;
			UInt168 _programHash;

			Type _outputType;

			uint8_t _payloadVersion;
			PayloadPtr _payload;
		};

		typedef boost::shared_ptr<TransactionOutput> TransactionOutputPtr;

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONOUTPUT_H__
