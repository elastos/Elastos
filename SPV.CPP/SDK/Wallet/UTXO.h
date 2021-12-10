// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UTXO_H__
#define __ELASTOS_SDK_UTXO_H__

#include <Common/uint256.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class TransactionInput;
		class BigInt;
		typedef boost::shared_ptr<TransactionInput> InputPtr;

		class UTXO {
		public:
			UTXO();

			UTXO(const UTXO &u);

			UTXO &operator=(const UTXO &u);

			UTXO(const uint256 &hash, uint16_t n, const Address &address, const BigInt &amount);

			virtual ~UTXO();

			bool operator==(const UTXO &u) const;

			const uint256 &Hash() const;

			void SetHash(const uint256 &hash);

			const uint16_t &Index() const;

			void SetIndex(uint16_t index);

			const Address &GetAddress() const;

			void SetAddress(const Address &address);

			const BigInt &GetAmount() const;

			void SetAmount(const BigInt &amount);

			bool Equal(const InputPtr &input) const;

			bool Equal(const uint256 &hash, uint16_t index) const;

		protected:
			Address _address;
			BigInt _amount;

			uint256 _hash;
			uint16_t _n;
		};
		typedef boost::shared_ptr<UTXO> UTXOPtr;
		typedef std::vector<UTXOPtr> UTXOArray;

		typedef struct _UTXOCompare {
			bool operator() (const UTXOPtr &x, const UTXOPtr &y) const {
				if (x->Hash() == y->Hash()) {
					return x->Index() < y->Index();
				} else {
					return x->Hash() < y->Hash();
				}
			}
		} UTXOCompare;

		typedef std::set<UTXOPtr, UTXOCompare> UTXOSet;

	}
}

#endif //__ELASTOS_SDK_UTXO_H__
