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

#ifndef __ELASTOS_SDK_UTXOSTORE_H__
#define __ELASTOS_SDK_UTXOSTORE_H__

#include "Sqlite.h"
#include "TableBase.h"

namespace Elastos {
	namespace ElaWallet {

		class UTXOEntity;

		class UTXOStore : public TableBase {
		public:
			UTXOStore(Sqlite *sqlite, SqliteTransactionType type = IMMEDIATE);

			~UTXOStore();

			virtual void InitializeTable();

			bool Puts(const std::vector<UTXOEntity> &entities);

			std::vector<UTXOEntity> Gets() const;

			bool Update(const std::vector<UTXOEntity> &added, const std::vector<UTXOEntity> &deleted, bool replace);

			bool DeleteAll();

			bool Delete(const std::vector<UTXOEntity> &entities);

			const std::string &GetTableName() const;

			const std::string &GetTxHashColumnName() const;

			const std::string &GetIndexColumnName() const;
		private:
			bool PutInternal(const UTXOEntity &entity);

			bool DeleteInternal(const UTXOEntity &entity);

		private:
			std::string _tableName;
			std::string _txHash;
			std::string _index;
			std::string _tableCreation;
		};

		class UTXOEntity {
		public:
			UTXOEntity() : _n(0) {
			}

			UTXOEntity(const std::string &hash, uint16_t n) :
				_hash(hash), _n(n) {
			}

			~UTXOEntity() {
			}

			bool operator==(const UTXOEntity &e) const {
				return this->_hash == e._hash && this->_n == e._n;
			}

			const std::string &Hash() const {
				return _hash;
			}

			uint16_t Index() const {
				return _n;
			}

		private:
			friend class UTXOStore;
			std::string _hash;
			uint16_t _n;
		};

	}
}

#endif //__ELASTOS_SDK_UTXOSTORE_H__
