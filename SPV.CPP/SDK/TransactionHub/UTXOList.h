// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UTXOSET_H__
#define __ELASTOS_SDK_UTXOSET_H__

#include <SDK/Plugin/Transaction/TransactionInput.h>

#include <vector>

namespace Elastos {
	namespace ElaWallet {

		struct UTXO {
			UTXO() : n(0), amount(0) {}

			UTXO(const uint256 &h, uint16_t i, uint64_t a) :
					hash(h),
					n(i),
					amount(a) {
			}

			bool operator<(const UTXO &otherUtxo) {
				return hash < otherUtxo.hash && n < otherUtxo.n;
			}

			uint256 hash;
			uint16_t n;
			uint64_t amount;
		};

		class UTXOList {
		public:
			bool Contains(const UTXO &o) const;

			bool Contains(const uint256 &hash, uint32_t n) const;

			UTXO &operator[](size_t i);

			size_t size() const;

			void Clear();

			const std::vector<UTXO> &GetUTXOs() const;

			void AddByTxInput(const TransactionInput &input, uint64_t amount);

			void AddUTXO(const uint256 &hash, uint16_t index, uint64_t amount);

			void RemoveAt(size_t index);

			bool Compare(const UTXO &o1, const UTXO &o2) const;

			void SortBaseOnOutputAmount(uint64_t totalOutputAmount, uint64_t feePerKB);

		private:
			std::vector<UTXO> _utxos;
		};

	}
}

#endif //__ELASTOS_SDK_UTXOSET_H__
