// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONOUTPUT_H__
#define __ELASTOS_SDK_TRANSACTIONOUTPUT_H__

#include <Plugin/Interface/ELAMessageSerializable.h>
#include <Plugin/Transaction/Payload/OutputPayload/IOutputPayload.h>
#include <Plugin/Transaction/Asset.h>
#include <WalletCore/Address.h>
#include <Common/BigInt.h>

#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class TransactionOutput : public JsonSerializer {

		public:
			enum Type {
				Default    = 0x00,
				VoteOutput = 0x01,
			};

		public:
			TransactionOutput();

			TransactionOutput(const TransactionOutput &output);

			TransactionOutput &operator=(const TransactionOutput &tx);

			TransactionOutput(const BigInt &amount, const Address &toAddress, const uint256 &assetID = Asset::GetELAAssetID(),
							  Type type = Default, const OutputPayloadPtr &payload = nullptr);

			~TransactionOutput();

			size_t EstimateSize() const;

			void Serialize(ByteStream &ostream, uint8_t txVersion, bool extend = false) const;

			bool Deserialize(const ByteStream &istream, uint8_t txVersion, bool extend = false);

			bool IsValid() const;

			const AddressPtr &Addr() const;

			const BigInt &Amount() const;

			void SetAmount(const BigInt &amount);

			const uint256 &AssetID() const;

			void SetAssetID(const uint256 &assetId);

			uint32_t OutputLock() const;

			void SetOutputLock(uint32_t outputLock);

			const Type &GetType() const;

			void SetType(const Type &type);

			const OutputPayloadPtr &GetPayload() const;

			OutputPayloadPtr &GetPayload();

			void SetPayload(const OutputPayloadPtr &payload);

			OutputPayloadPtr GeneratePayload(const Type &type);

			nlohmann::json ToJson() const;

			void FromJson(const nlohmann::json &j);

			size_t GetSize() const;

			uint16_t FixedIndex() const;

			void SetFixedIndex(uint16_t index);

		private:
			uint16_t _fixedIndex;

			BigInt _amount; // to support token chain
			uint256 _assetID;
			uint32_t _outputLock;
			AddressPtr _addr;

			Type _outputType;

			OutputPayloadPtr _payload;
		};

		typedef boost::shared_ptr<TransactionOutput> OutputPtr;
		typedef std::vector<OutputPtr> OutputArray;

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONOUTPUT_H__
