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

			PayloadRecord(const std::string &recordType, const CMBlock &recordData);

			PayloadRecord(const PayloadRecord &payload);

			~PayloadRecord();

			void setRecordType(const std::string &recordType);

			void setRecordData(const CMBlock &recordData);

			std::string getRecordType() const;
			CMBlock getRecordData() const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(ByteStream &istream, uint8_t version);

			virtual nlohmann::json toJson(uint8_t version) const;

			virtual void fromJson(const nlohmann::json &jsonData, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadRecord &operator=(const PayloadRecord &payload);

		private:
			std::string _recordType;
			CMBlock _recordData;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADRECORD_H
