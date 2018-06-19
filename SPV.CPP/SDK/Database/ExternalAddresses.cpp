// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ExternalAddresses.h"

namespace Elastos {
	namespace ElaWallet {

		ExternalAddresses::ExternalAddresses(Sqlite *sqlite) :
			TableBase(sqlite) {
			initializeTable(MB_DATABASE_CREATE);
		}

		ExternalAddresses::ExternalAddresses(SqliteTransactionType type, Sqlite *sqlite) :
			TableBase(type, sqlite) {
			initializeTable(MB_DATABASE_CREATE);
		}

		ExternalAddresses::~ExternalAddresses() {

		}

		bool ExternalAddresses::putAddress(uint32_t startIndex, const std::string &address) {
			return doTransaction([startIndex, &address, this](){
				//todo complete me
			});
		}

		bool ExternalAddresses::putAddresses(uint32_t startIndex, const std::vector<std::string> &addresses) {
			return doTransaction([startIndex, &addresses, this](){
				//todo complete me
			});
		}

		bool ExternalAddresses::clearAddresses() {
			return doTransaction([this](){
				//todo complete me
			});
		}

		std::vector<std::string> ExternalAddresses::getAddresses(uint32_t startIndex, uint32_t count) {
			std::vector<std::string> results;
			doTransaction([startIndex, count, &results, this]() {
				//todo complete me
			});
			return results;
		}

		uint32_t ExternalAddresses::getAvailableAddresses(uint32_t startIndex) {
			uint32_t results;
			doTransaction([startIndex, &results, this]() {
				//todo complete me
			});
			return results;
		}
	}
}