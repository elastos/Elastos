// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ADDRESSCACHE_H__
#define __ELASTOS_SDK_ADDRESSCACHE_H__

#include <string>

#include "MasterPrivKey.h"
#include "DatabaseManager.h"

#define INTERNAL_ADDRESS_CACHE_SIZE 1000
#define EXTERNAL_ADDRESS_CACHE_SIZE 1000

namespace Elastos {
	namespace SDK {

		class AddressCache {
		public:
			AddressCache(const MasterPrivKey &masterPrivKey, DatabaseManager *databaseManager,
				uint32_t internalCacheSize = INTERNAL_ADDRESS_CACHE_SIZE,
				uint32_t externalCacheSize = EXTERNAL_ADDRESS_CACHE_SIZE);

			~AddressCache();

			std::vector<std::string> FetchAddresses(size_t size, bool external);

			void Reset(const std::string &payPassword, uint32_t startIndex, bool external);

		private:
			DatabaseManager *_databaseManager;
			MasterPrivKey _masterPrivKey;
			uint32_t _internalStartIndex;
			uint32_t _externalStartIndex;

			uint32_t _internalCacheSize;
			uint32_t _externalCacheSize;
		};

	}
}

#endif //__ELASTOS_SDK_ADDRESSCACHE_H__
