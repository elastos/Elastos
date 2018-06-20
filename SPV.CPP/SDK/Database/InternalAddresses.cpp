// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "InternalAddresses.h"

namespace Elastos {
	namespace ElaWallet {

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
				this->putAddressInternal(startIndex, address);
			});
		}

		bool InternalAddresses::putAddresses(uint32_t startIndex, const std::vector<std::string> &addresses) {
			return doTransaction([startIndex, &addresses, this](){
				for (size_t i = 0; i < addresses.size(); ++i) {
					this->putAddressInternal(startIndex + i, addresses[i]);
				}
			});
		}

		bool InternalAddresses::putAddressInternal(uint32_t startIndex, const std::string &address) {
			std::stringstream ss;

			ss << "INSERT INTO " << IA_TABLE_NAME << " (" <<
				IA_COLUMN_ID     << "," <<
				IA_ADDRESS       <<
			   ") VALUES (?, ?);";

			sqlite3_stmt *stmt;
			if (!_sqlite->prepare(ss.str(), &stmt, nullptr)) {
				std::stringstream ess;
				ess << "prepare sql " << ss.str() << " fail";
				throw std::logic_error(ess.str());
			}

			_sqlite->bindInt(stmt, 1, startIndex);
			_sqlite->bindText(stmt, 2, address, nullptr);

			_sqlite->step(stmt);
			_sqlite->finalize(stmt);

			return true;
		}

		bool InternalAddresses::clearAddresses() {
			return doTransaction([this](){
				std::stringstream ss;

				ss << "DELETE FROM " << IA_TABLE_NAME << ";";

				if (!_sqlite->exec(ss.str(), nullptr, nullptr)) {
					std::stringstream ess;
					ess << "exec sql " << ss.str() << " fail";
					throw std::logic_error(ess.str());
				}
			});
		}

		std::vector<std::string> InternalAddresses::getAddresses(uint32_t startIndex, uint32_t count) const {
			std::vector<std::string> results;

			doTransaction([startIndex, count, &results, this]() {
				std::string addr;
				std::stringstream ss;
				ss << "SELECT " <<
					IA_ADDRESS  <<
				   " FROM "     << IA_TABLE_NAME <<
				   " WHERE "    << IA_COLUMN_ID  << " >= " << startIndex <<
				   " AND "      << IA_COLUMN_ID  << " < "  << startIndex + count << ";";

				sqlite3_stmt *stmt;
				if (!_sqlite->prepare(ss.str(), &stmt, nullptr)) {
					std::stringstream ess;
					ess << "prepare sql " << ss.str() << " fail";
					throw std::logic_error(ess.str());
				}

				while (SQLITE_ROW == _sqlite->step(stmt)) {
					addr = _sqlite->columnText(stmt, 0);
					results.push_back(addr);
				}

				_sqlite->finalize(stmt);
			});

			return results;
		}

		uint32_t InternalAddresses::getAvailableAddresses(uint32_t startIndex) const {
			uint32_t results = 0;

			doTransaction([startIndex, &results, this]() {
				std::stringstream ss;
				ss << "SELECT " <<
					" COUNT("   << IA_ADDRESS    << ") AS nums " <<
					" FROM "    << IA_TABLE_NAME <<
					" WHERE "   << IA_COLUMN_ID  << " >= " << startIndex << ";";

				sqlite3_stmt *stmt;
				if (!_sqlite->prepare(ss.str(), &stmt, nullptr)) {
					std::stringstream ess;
					ess << "prepare sql " << ss.str() << " fail";
					throw std::logic_error(ess.str());
				}

				while (SQLITE_ROW == _sqlite->step(stmt)) {
					results = (uint32_t)_sqlite->columnInt(stmt, 0);
				}

				_sqlite->finalize(stmt);
			});

			return results;
		}
	}
}