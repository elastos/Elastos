// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONOUTPUT_H__
#define __ELASTOS_SDK_TRANSACTIONOUTPUT_H__

#include <SDK/Plugin/Interface/ELAMessageSerializable.h>
#include <SDK/Plugin/Transaction/Payload/OutputPayload/IOutputPayload.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/WalletCore/BIPs/Address.h>

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

			TransactionOutput(uint64_t amount, const Address &toAddress, const uint256 &assetID = Asset::GetELAAssetID(),
							  Type type = Default, const OutputPayloadPtr &payload = nullptr);

			TransactionOutput(uint64_t amount, const uint168 &programHash, const uint256 &assetID = Asset::GetELAAssetID(),
							  Type type = Default, const OutputPayloadPtr &payload = nullptr);

			~TransactionOutput();

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(const ByteStream &istream);

			void Serialize(ByteStream &ostream, uint8_t txVersion) const;

			bool Deserialize(const ByteStream &istream, uint8_t txVersion);

			bool IsValid() const;

			Address GetAddress() const;

			uint64_t GetAmount() const;

			void SetAmount(uint64_t amount);

			const uint256 &GetAssetId() const;

			void SetAssetId(const uint256 &assetId);

			uint32_t GetOutputLock() const;

			void SetOutputLock(uint32_t outputLock);

			const uint168 &GetProgramHash() const;

			void SetProgramHash(const uint168 &hash);

			const Type &GetType() const;

			void SetType(const Type &type);

			const OutputPayloadPtr &GetPayload() const;

			OutputPayloadPtr &GetPayload();

			void SetPayload(const OutputPayloadPtr &payload);

			OutputPayloadPtr GeneratePayload(const Type &type);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

			nlohmann::json ToJson(uint8_t txVersion) const;

			void FromJson(const nlohmann::json &j, uint8_t txVersion);

			size_t GetSize() const;

		private:
			uint64_t _amount;
			uint256 _assetId;
			uint32_t _outputLock;
			uint168 _programHash;

			Type _outputType;

			OutputPayloadPtr _payload;
		};

		typedef boost::shared_ptr<TransactionOutput> TransactionOutputPtr;

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONOUTPUT_H__
