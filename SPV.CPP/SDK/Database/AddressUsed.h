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

#ifndef __ELASTOS_SDK_ADDRESSUSED_H__
#define __ELASTOS_SDK_ADDRESSUSED_H__

#include "TableBase.h"

namespace Elastos {
	namespace ElaWallet {

		class AddressUsed : public TableBase {
		public:
			AddressUsed(Sqlite *sqlite, SqliteTransactionType type = IMMEDIATE);

			~AddressUsed();

			virtual void InitializeTable();

			bool Puts(const std::vector<std::string> &items, bool replace);

			std::vector<std::string> Gets() const;

			bool DeleteAll();

		private:
			bool PutInternal(const std::string &item);

		private:
			const std::string _tableName = "UsedAddressTable";
			const std::string _columnName = "Address";
			const std::string _tableCreation = "CREATE TABLE IF NOT EXISTS " + _tableName + "(" + _columnName + " TEXT NOT NULL UNIQUE);";
		};

	}
}


#endif
