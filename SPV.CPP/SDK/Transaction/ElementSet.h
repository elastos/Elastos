// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONSET_H__
#define __ELASTOS_SDK_TRANSACTIONSET_H__

#include <set>

#include "Transaction.h"
#include "Plugin/Interface/IMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		template<class T>
		class ElementSet {
		public:

			const T &Get(const UInt256 &hash) const {
				if (!Contains(hash)) return nullptr;

				typename std::set<T>::const_iterator it;
				it = std::find_if(_elements.begin(), _elements.end(), [&hash](const T &e) {
					return UInt256Eq(&hash, &e->getHash()) == 1;
				});
				return *it;
			}

			bool Contains(const T &tx) const {
				return _elements.find(tx) != _elements.end();
			}

			bool Contains(const UInt256 &hash) const {
				typename std::set<T>::const_iterator it;
				it = std::find_if(_elements.begin(), _elements.end(), [&hash](const T &e) {
					return UInt256Eq(&hash, &e->getHash()) == 1;
				});
				return it != _elements.end();
			}

			void Insert(const T &tx) {
				_elements.insert(tx);
			}

			void Remove(const T &tx) {
				typename std::set<T>::const_iterator it;
				for(it = _elements.cbegin(); it != _elements.cend();) {
					if (UInt256Eq(&tx->getHash(), &(*it)->getHash())) {
						it = _elements.erase(it);
					} else {
						++it;
					}
				}
			}

			void Clear() {
				_elements.clear();
			}

		private:
			std::set<T> _elements;
		};

		typedef ElementSet<TransactionPtr> TransactionSet;
		typedef ElementSet<MerkleBlockPtr> BlockSet;
	}
}

#endif //__ELASTOS_SDK_TRANSACTIONSET_H__
