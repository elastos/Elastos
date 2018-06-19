// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "InternalAddresses.h"

namespace Elastos {
	namespace SDK {

		InternalAddresses::InternalAddresses(Sqlite *sqlite) :
			TableBase(sqlite) {
			initializeTable(MB_DATABASE_CREATE);
		}

		InternalAddresses::InternalAddresses(SqliteTransactionType type, Sqlite *sqlite) :
			TableBase(type, sqlite) {
			initializeTable(MB_DATABASE_CREATE);
		}

		InternalAddresses::~InternalAddresses() {

		}

		bool InternalAddresses::putAddress(uint32_t startIndex, const std::string &address) {
			return doTransaction([startIndex, &address, this](){
				//todo complete me
			});
		}

		bool InternalAddresses::putAddresses(uint32_t startIndex, const std::vector<std::string> &addresses) {
			return doTransaction([startIndex, &addresses, this](){
				//todo complete me
			});
		}

		bool InternalAddresses::clearAddresses() {
			return doTransaction([this](){
				//todo complete me
			});
		}

		std::vector<std::string> InternalAddresses::getAddresses(uint32_t startIndex, uint32_t count) {
			std::vector<std::string> results;
			doTransaction([startIndex, count, &results, this]() {
				//todo complete me
			});
			return results;
		}

		uint32_t InternalAddresses::getAvailableAddresses(uint32_t startIndex) {
			uint32_t results;
			doTransaction([startIndex, &results, this]() {
				//todo complete me
			});
			return results;
		}
	}
}