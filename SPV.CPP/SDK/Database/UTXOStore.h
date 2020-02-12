// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UTXOSTORE_H__
#define __ELASTOS_SDK_UTXOSTORE_H__

#include "Sqlite.h"
#include "TableBase.h"

namespace Elastos {
	namespace ElaWallet {

		class UTXOEntity;

		class UTXOStore : public TableBase {
		public:
			UTXOStore();

			UTXOStore(Sqlite *sqlite);

			~UTXOStore();

			bool Puts(const std::vector<UTXOEntity> &entities);

			std::vector<UTXOEntity> Gets() const;

			bool DeleteAll();

			bool Delete(const std::vector<UTXOEntity> &entities);

			bool TableExist() const;

		private:
			bool PutInternal(const UTXOEntity &entity);

			bool DeleteInternal(const UTXOEntity &entity);

			bool TableExistInternal() const;

		private:
			bool _tableExist;
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
