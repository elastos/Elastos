// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ADDRESS_H__
#define __ELASTOS_SDK_ADDRESS_H__

#include <SDK/Crypto/Key.h>

namespace Elastos {
	namespace ElaWallet {

#define ELA_SIDECHAIN_DESTROY_ADDR "1111111111111111111114oLvT2"

		class Address {
		public:
			Address();

			Address(const std::string &address);

			Address(const UInt168 &programHash);

			Address(const CMBlock &pubKey, Prefix prefix);

			Address(const Address &address);

			~Address();

			bool Valid() const;

			bool IsIDAddress() const;

			std::string String() const;

			const UInt168 &ProgramHash() const;

			bool operator<(const Address &address) const;

			bool operator==(const Address &address) const;

			bool operator==(const std::string &address) const;

			bool operator!=(const Address &address) const;

			bool operator!=(const std::string &address) const;

			Address &operator=(const Address &address);

		private:
			void CheckValid();

		private:
			UInt168 _programHash;
			bool _isValid;
		};

	}
}


#endif //__ELASTOS_SDK_ADDRESS_H__
