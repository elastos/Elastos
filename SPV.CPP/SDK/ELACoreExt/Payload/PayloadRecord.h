// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADRECORD_H
#define __ELASTOS_SDK_PAYLOADRECORD_H

#include "IPayload.h"

namespace Elastos {
	namespace SDK {
		class PayloadRecord :
				public IPayload {
		public:
			PayloadRecord();

			PayloadRecord(const std::string &recordType, const ByteData recordData);

			~PayloadRecord();

			std::string getRecordType() const;

			ByteData getRecordData() const;

			virtual ByteData getData() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual void Deserialize(ByteStream &istream);

		private:
			std::string _recordType;
			ByteData _recordData;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADRECORD_H
