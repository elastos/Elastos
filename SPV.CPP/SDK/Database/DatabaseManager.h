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

#ifndef __ELASTOS_SDK_DATABASEMANAGER_H__
#define __ELASTOS_SDK_DATABASEMANAGER_H__

#include "AddressUsed.h"

namespace Elastos {
	namespace ElaWallet {

		class DatabaseManager {
		public:
			DatabaseManager(const boost::filesystem::path &path);

			DatabaseManager();

			~DatabaseManager();

			void ClearData();

			// Used Address
			bool PutUsedAddresses(const std::vector<std::string> &addresses, bool replace);

			std::vector<std::string> GetUsedAddresses() const;

			bool DeleteAllUsedAddresses();

			// common
			const boost::filesystem::path &GetPath() const;

			void flush();

		private:
			boost::filesystem::path _path;
			Sqlite _sqlite;
			AddressUsed _addressUsed;
		};

		typedef boost::shared_ptr<DatabaseManager> DatabaseManagerPtr;
		typedef boost::weak_ptr<DatabaseManager> DatabaseManagerWeakPtr;

	}
}

#endif //__ELASTOS_SDK_DATABASEMANAGER_H__
