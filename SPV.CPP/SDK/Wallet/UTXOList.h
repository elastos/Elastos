// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UTXOSET_H__
#define __ELASTOS_SDK_UTXOSET_H__

#include <SDK/Common/uint256.h>
#include <SDK/Common/BigInt.h>

#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class TransactionInput;

		struct UTXO {
			UTXO() : n(0) {}

			UTXO(const uint256 &h, uint32_t i, const BigInt &a, uint32_t c) :
					hash(h),
					n(i) {
				amount = a;
			}

			bool operator<(const UTXO &otherUtxo) {
				return hash < otherUtxo.hash && n < otherUtxo.n;
			}

			uint256 hash;
			uint32_t n;
			BigInt amount;
		};

		class UTXOList {
		public:
			bool Contains(const UTXO &o) const;

			bool Contains(const uint256 &hash, uint32_t n) const;

			bool Contains(const TransactionInput &input) const;

			UTXO &operator[](size_t i);

			size_t size() const;

			void Clear();

			const std::vector<UTXO> &GetUTXOs() const;

			void AddByTxInput(const TransactionInput &input, const BigInt &amount, uint32_t confirms);

			void AddUTXO(const UTXO &o);

			void RemoveAt(size_t index);

			bool Compare(const UTXO &o1, const UTXO &o2) const;

			void SortBaseOnOutputAmount(const BigInt &totalOutputAmount, uint64_t feePerKB);

		private:
			std::vector<UTXO> _utxos;
		};

	}
}

#endif //__ELASTOS_SDK_UTXOSET_H__
