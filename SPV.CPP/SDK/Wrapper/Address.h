// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ADDRESS_H__
#define __ELASTOS_SDK_ADDRESS_H__

#include <string>
#include <boost/shared_ptr.hpp>

#include "BRInt.h"
#include "Wrapper.h"
#include "CMemBlock.h"
#include "Transaction/Transaction.h"

namespace Elastos {
	namespace ElaWallet {

		class Address {
		public:
			Address();

			Address(const std::string &address);

			Address &operator= (const std::string &address);

			bool operator== (const std::string &address) const;

			bool isValid();

			CMBlock getPubKeyScript();

			int getSignType() const;

			std::string stringify() const;

			const char *GetChar() const;

			bool operator< (const Address &address) const;

			bool IsEqual(const Address &address) const;

		public:
			static boost::shared_ptr<Address> createAddress(const std::string &address);

			static boost::shared_ptr<Address> fromScriptPubKey(CMBlock script, int signType);

			static boost::shared_ptr<Address> fromScriptSignature(CMBlock script);

			static bool isValidAddress(const std::string &address);

			static bool UInt168IsValid(const UInt168 &u168);

			static bool isValidIdAddress(const std::string &address);

			static bool isValidProgramHash(const UInt168 &u168, const Transaction::Type &type);

			static Address None;

		private:
			char _s[75];
		};

		typedef boost::shared_ptr<Address> AddressPtr;

	}
}

#endif //__ELASTOS_SDK_ADDRESS_H__
