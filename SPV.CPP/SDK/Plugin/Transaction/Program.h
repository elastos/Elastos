// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PROGRAM_H__
#define __ELASTOS_SDK_PROGRAM_H__

#include <SDK/Common/CMemBlock.h>
#include <SDK/Plugin/Interface/ELAMessageSerializable.h>

#include <Core/BRInt.h>
#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Transaction;

		class Program :
				public ELAMessageSerializable {
		public:
			Program();

			Program(const Program &program);

			Program(const CMBlock &code, const CMBlock &parameter);

			~Program();

			std::vector<CMBlock> DecodePublicKey() const;

			bool VerifySignature(const UInt256 &md) const;

			nlohmann::json GetSignedInfo(const UInt256 &md) const;

			const CMBlock &GetCode() const;

			const CMBlock &GetParameter() const;

			void SetCode(const CMBlock &code);

			void SetParameter(const CMBlock &parameter);

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &jsonData);

		private:
			CMBlock _code;
			CMBlock _parameter;
		};

		typedef boost::shared_ptr<Program> ProgramPtr;

	}
}

#endif //__ELASTOS_SDK_PROGRAM_H__
