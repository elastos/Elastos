// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MULTISIGNACCOUNT_H__
#define __ELASTOS_SDK_MULTISIGNACCOUNT_H__

#include <vector>

#include "IAccount.h"
#include "StandardAccount.h"

namespace Elastos {
	namespace ElaWallet {

		class MultiSignAccount : public IAccount {
		public:
			MultiSignAccount(StandardAccount *standard, const std::vector<std::string> &coSigners);

			std::string GetAddress();

			void AppendSign();

		private:
			void SortSigners();

		private:
			StandardAccount *_standard;
			std::vector<std::string> _coSigners;
		};

	}
}

#endif //__ELASTOS_SDK_MULTISIGNACCOUNT_H__
