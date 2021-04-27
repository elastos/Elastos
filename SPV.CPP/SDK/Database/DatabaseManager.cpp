/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "DatabaseManager.h"
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		DatabaseManager::DatabaseManager(const boost::filesystem::path &path) :
			_path(path),
			_sqlite(path),
			_addressUsed(&_sqlite),
			_settings(&_sqlite)
		{
		}

		DatabaseManager::DatabaseManager() : DatabaseManager("spv_wallet.db") {}

		DatabaseManager::~DatabaseManager() {}

		void DatabaseManager::ClearData() {
			_addressUsed.DeleteAll();
		}

		int DatabaseManager::GetSyncMode() const {
			return _settings.GetSetting("syncMode");
		}

		bool DatabaseManager::SetSyncMode(int mode) {
			return _settings.PutSetting("syncMode", mode);
		}

		const boost::filesystem::path &DatabaseManager::GetPath() const {
			return _path;
		}

		bool DatabaseManager::PutUsedAddresses(const std::vector<std::string> &addresses, bool replace) {
			return _addressUsed.Puts(addresses, replace);
		}

		std::vector<std::string> DatabaseManager::GetUsedAddresses() const {
			return _addressUsed.Gets();
		}

		bool DatabaseManager::DeleteAllUsedAddresses() {
			return _addressUsed.DeleteAll();
		}

		void DatabaseManager::flush() {
			_addressUsed.flush();
			_settings.flush();
		}

	} // namespace ElaWallet
} // namespace Elastos
