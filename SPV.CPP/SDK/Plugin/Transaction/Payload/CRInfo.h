// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CRINFO_H__
#define __ELASTOS_SDK_CRINFO_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

#define CRInfoVersion 0x00
#define CRInfoDIDVersion 0x01

		class CRInfo : public IPayload {
		public:
			CRInfo();

			~CRInfo();

			const bytes_t &GetCode() const;

			void SetCode(const bytes_t &code);

			const uint168 &GetCID() const;

			void SetCID(const uint168 &cid);

			const uint168 &GetDID() const;

			void SetDID(const uint168 &did);

			const std::string &GetNickName() const;

			void SetNickName(const std::string &nickName);

			const std::string &GetUrl() const;

			void SetUrl(const std::string &url);

			uint64_t GetLocation() const;

			void SetLocation(uint64_t location);

			const bytes_t &GetSignature() const;

			void SetSignature(const bytes_t &signature);

			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			void SerializeUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeUnsigned(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual bool IsValid(uint8_t version) const;

			virtual IPayload &operator=(const IPayload &payload);

			CRInfo &operator=(const CRInfo &payload);

		private:
			bytes_t _code;
			uint168 _cid;
			uint168 _did;
			std::string _nickName;
			std::string _url;
			uint64_t _location;
			bytes_t _signature;
		};
	}
}

#endif //__ELASTOS_SDK_CRINFO_H__
