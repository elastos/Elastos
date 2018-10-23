// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADIDCHAIN_H__
#define __ELASTOS_SDK_PAYLOADIDCHAIN_H__

#include "BRInt.h"

#include "CMemBlock.h"
#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		class PayloadRegisterIdentification : public IPayload {
		public:
			struct ValueItem {
				UInt256 DataHash;
				std::string Proof;
			};

			struct SignContent {
				std::string Path;
				std::vector<ValueItem> Values;
			};

		public:
			PayloadRegisterIdentification();

			~PayloadRegisterIdentification();

			const std::string &getId() const;

			void setId(const std::string &id);

			const std::string &getPath(size_t index) const;

			void setPath(const std::string &path, size_t index);

			const UInt256 &getDataHash(size_t index, size_t valueIndex) const;

			void setDataHash(const UInt256 &dataHash, size_t index, size_t valueIndex);

			const std::string &getProof(size_t index, size_t valueIndex) const;

			void setProof(const std::string &proof, size_t index, size_t valueIndex);

			size_t getContentCount() const;

			void addContent(const SignContent &content);

			void removeContent(size_t index);

			const CMBlock &getSign() const;

			void setSign(const CMBlock &sign);

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &);

			virtual bool isValid() const;

		private:
			std::string _id;
			CMBlock _sign;
			std::vector<SignContent> _contents;
		};

	}
}

#endif //__ELASTOS_SDK_PAYLOADIDCHAIN_H__
