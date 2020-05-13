/*
 * Copyright (c) 2020 Elastos Foundation
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

#ifndef __ELASTOS_SPVSDK_SIMPLETABLE_H__
#define __ELASTOS_SPVSDK_SIMPLETABLE_H__

#include "TableBase.h"

namespace Elastos {
	namespace ElaWallet {

		class SimpleTable : public TableBase {
		public:
			SimpleTable(Sqlite *sqlite, SqliteTransactionType type = IMMEDIATE);

			~SimpleTable();

			virtual void InitializeTable();

			bool Puts(const std::vector<std::string> &items, bool replace);

			std::vector<std::string> Gets() const;

			bool DeleteAll();

			bool ContainTable() const;

			const std::string &GetTableName() const;

			const std::string &GetTxHashColumnName() const;

		private:
			bool PutInternal(const std::string &item);

		protected:
			std::string _tableName;
			std::string _columnName;
			std::string _tableCreation;

			bool _existTable;
		};

	}
}

#endif
