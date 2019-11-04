// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PRODUCERINFO_H__
#define __ELASTOS_SDK_PRODUCERINFO_H__

#include <Plugin/Transaction/Payload/IPayload.h>

namespace Elastos {
	namespace ElaWallet {

		class ProducerInfo : public IPayload {
		public:
			ProducerInfo();

			ProducerInfo(const ProducerInfo &payload);

			~ProducerInfo();

			const bytes_t &GetPublicKey() const;

			void SetPublicKey(const bytes_t &key);

			const bytes_t &GetNodePublicKey() const;

			void SetNodePublicKey(const bytes_t &key);

			const std::string &GetNickName() const;

			void SetNickName(const std::string &name);

			const std::string &GetUrl() const;

			void SetUrl(const std::string &url);

			uint64_t GetLocation() const;

			void SetLocation(uint64_t location);

			const std::string &GetAddress() const;

			void SetAddress(const std::string &address);

			const bytes_t &GetSignature() const;

			void SetSignature(const bytes_t &signature);

			void SerializeUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeUnsigned(const ByteStream &istream, uint8_t version);

			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			ProducerInfo &operator=(const ProducerInfo &payload);

		private:
			bytes_t _ownerPublicKey;
			bytes_t _nodePublicKey;
			std::string _nickName;
			std::string _url;
			uint64_t _location;
			std::string _address;
			bytes_t _signature;
		};

	}
}

#endif //__ELASTOS_SDK_PAYLOADREGISTERPRODUCER_H__
