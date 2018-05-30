// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADIDCHAIN_H__
#define __ELASTOS_SDK_PAYLOADIDCHAIN_H__

#include "BRInt.h"

#include "CMemBlock.h"
#include "IPayload.h"

namespace Elastos {
	namespace SDK {

		class PayloadRegisterIdentification : public IPayload {
		public:
			PayloadRegisterIdentification();

			~PayloadRegisterIdentification();

			const std::string &getId() const;

			void setId(const std::string &id);

			const std::string &getPath() const;

			void setPath(const std::string &path);

			const UInt256 &getDataHash() const;

			void setDataHash(const UInt256 &dataHash);

			const CMBlock &getProof() const;

			void setProof(const CMBlock &proof);

			const CMBlock &getSign() const;

			void setSign(const CMBlock &sign);

			virtual void Serialize(ByteStream &ostream) const;

			virtual void Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson();

			virtual void fromJson(const nlohmann::json &);

			virtual bool isValid() const;

		private:
			std::string _id;
			std::string _path;
			UInt256 _dataHash;
			CMBlock _proof;
			CMBlock _sign;
		};

	}
}

#endif //__ELASTOS_SDK_PAYLOADIDCHAIN_H__
