// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADRECORD_H
#define __ELASTOS_SDK_PAYLOADRECORD_H

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {
		class PayloadRecord :
				public IPayload {
		public:
			PayloadRecord();

			PayloadRecord(const std::string &recordType, const bytes_t &recordData);

			PayloadRecord(const PayloadRecord &payload);

			~PayloadRecord();

			void SetRecordType(const std::string &recordType);

			void SetRecordData(const bytes_t &recordData);

			std::string GetRecordType() const;

			bytes_t GetRecordData() const;

			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadRecord &operator=(const PayloadRecord &payload);

		private:
			std::string _recordType;
			bytes_t _recordData;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADRECORD_H
