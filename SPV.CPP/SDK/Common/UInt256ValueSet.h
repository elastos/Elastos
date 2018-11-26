// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UINT256SET_H__
#define __ELASTOS_SDK_UINT256SET_H__

#include <Core/BRInt.h>

#include <set>
#include <map>
#include <boost/function.hpp>

namespace Elastos {
	namespace ElaWallet {

		struct UInt256Compare {
			bool operator()(const UInt256 &first, const UInt256 &second) const {
				return UInt256LessThan(&first, &second) == 1;
			}
		};

		class UInt256ValueSet {
		public:
			void Insert(const UInt256 &value);

			bool Contains(const UInt256 &value) const;

			void Remove(const UInt256 &value);

		private:
			std::set<UInt256, UInt256Compare> _set;
		};

		template<class T>
		class UInt256ValueMap {
		public:
			typedef std::map<UInt256, T, UInt256Compare> MapType;

			bool Contains(const UInt256 &key) const {
				return _map.find(key) != _map.end();
			}

			void Insert(const UInt256 &key, const T &value) {
				_map[key] = value;
			}

			typename MapType::iterator Begin() {
				return _map.begin();
			}

			typename MapType::iterator End() {
				return _map.end();
			}

			typename MapType::const_iterator CBegin() const {
				return _map.cbegin();
			}

			typename MapType::const_iterator CEnd() const {
				return _map.cend();
			}

			const T &Get(const UInt256 &key) const {
				assert(Contains(key));
				return _map.find(key)->second;
			}

			T &operator[](const UInt256 &key) {
				return _map[key];
			}

			void ForEach(const boost::function<void(const UInt256 &key, const T &value)> &fun) {
				for (typename MapType::iterator it = _map.begin(); it != _map.end(); ++it) {
					fun(it->first, it->second);
				}
			}

			void ForEach(const boost::function<void(const UInt256 &key, const T &value) const> &fun) const {
				for (typename MapType::iterator it = _map.begin(); it != _map.end(); ++it) {
					fun(it->first, it->second);
				}
			}

			bool Empty() const {
				return _map.empty();
			}

		private:
			MapType _map;
		};

	}
}


#endif //__ELASTOS_SDK_UINT256SET_H__
