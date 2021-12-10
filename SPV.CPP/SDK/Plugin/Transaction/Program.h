// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PROGRAM_H__
#define __ELASTOS_SDK_PROGRAM_H__

#include <Plugin/Interface/ELAMessageSerializable.h>
#include <Common/JsonSerializer.h>
#include <WalletCore/Address.h>

#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Transaction;
		class Address;

		class Program : public ELAMessageSerializable, public JsonSerializer {
		public:
			Program();

			Program(const Program &program);

			Program(const bytes_t &code, const bytes_t &parameter);

			~Program();

			Program &operator=(const Program &tx);

			SignType DecodePublicKey(std::vector<bytes_t> &pubkeys) const;

			bool VerifySignature(const uint256 &md) const;

			nlohmann::json GetSignedInfo(const uint256 &md) const;

			const bytes_t &GetCode() const;

			const bytes_t &GetParameter() const;

			void SetCode(const bytes_t &code);

			void SetParameter(const bytes_t &parameter);

			size_t EstimateSize() const;

			void Serialize(ByteStream &stream) const;

			bool Deserialize(const ByteStream &stream);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

			bool operator==(const Program &p) const;

			bool operator!=(const Program &p) const;

		private:
			bytes_t _code;
			bytes_t _parameter;
		};

		typedef boost::shared_ptr<Program> ProgramPtr;
		typedef std::vector<ProgramPtr> ProgramArray;

	}
}

#endif //__ELASTOS_SDK_PROGRAM_H__
