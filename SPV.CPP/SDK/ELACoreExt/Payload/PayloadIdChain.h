// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADIDCHAIN_H__
#define __ELASTOS_SDK_PAYLOADIDCHAIN_H__

#include "IPayload.h"

namespace Elastos {
	namespace SDK {

		class PayloadIdChain : public IPayload {
		public:
			PayloadIdChain();

			~PayloadIdChain();

			const std::string &getId() const;

			void setId(const std::string &id);

			const std::string &getPath() const;

			void setPath(const std::string &path);

			const std::string &getDataHash() const;

			void setDataHash(const std::string &dataHash);

			const std::string &getProof() const;

			void setProof(const std::string &proof);

			const std::string &getSign() const;

			void setSign(const std::string &sign);

			virtual void Serialize(ByteStream &ostream) const;

			virtual void Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson();

			virtual void fromJson(nlohmann::json);

		private:
			std::string _id;
			std::string _path;
			std::string _dataHash;
			std::string _proof;
			std::string _sign;
		};

	}
}

#endif //__ELASTOS_SDK_PAYLOADIDCHAIN_H__
