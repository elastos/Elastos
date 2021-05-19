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
#ifndef __ELASTOS_SDK_TRANSACTIONSET_H__
#define __ELASTOS_SDK_TRANSACTIONSET_H__

#include <set>
#include <Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {

		template<class T>
		class ElementSet {
		public:
			typedef struct {
				bool operator() (const T &x, const T &y) const {
					return x->GetHash() < y->GetHash();
				}
			} TCompare;

			T Get(const uint256 &hash) const {
				typename std::set<T, TCompare>::const_iterator it;
				it = std::find_if(_elements.begin(), _elements.end(), [&hash](const T &e) {
					return hash == e->GetHash();
				});

				if (it == _elements.end())
					return nullptr;

				return *it;
			}

			std::set<T, TCompare> &Raw() {
				return _elements;
			}

			bool Contains(const T &e) const {
				return _elements.find(e) != _elements.end();
			}

			bool Contains(const uint256 &hash) const {
				return Get(hash) != nullptr;
			}

			bool Insert(const T &e) {
				return _elements.insert(e).second;
			}

			size_t Size() {
				return _elements.size();
			}

			bool Remove(const T &e) {
				return _elements.erase(e) > 0;
			}

			bool RemoveMatchPrevHash(const uint256 &hash) {
				typename std::set<T, TCompare>::const_iterator it;
				it = std::find_if(_elements.cbegin(), _elements.cend(), [&hash](const T &e) {
					return hash == e->GetPrevBlockHash();
				});

				if (it != _elements.end()) {
					_elements.erase(it);
					return true;
				}

				return false;
			}

			T GetMatchPrevHash(const uint256 &hash) const {
				typename std::set<T, TCompare>::const_iterator it;
				it = std::find_if(_elements.cbegin(), _elements.cend(), [&hash](const T &e) {
					return hash == e->GetPrevBlockHash();
				});

				if (it != _elements.end())
					return *it;

				return nullptr;
			}

			void Clear() {
				_elements.clear();
			}

		private:
			std::set<T, TCompare> _elements;
		};

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONSET_H__
