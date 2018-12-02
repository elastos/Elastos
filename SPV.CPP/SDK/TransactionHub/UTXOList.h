// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UTXOSET_H__
#define __ELASTOS_SDK_UTXOSET_H__

#include <SDK/Plugin/Transaction/TransactionInput.h>
#include <Core/BRInt.h>

#include <vector>

namespace Elastos {
	namespace ElaWallet {

		struct UTXO {
			UTXO() : hash(UINT256_ZERO), n(0), amount(0), confirms(0) {}

			UTXO(const UInt256 &h, uint32_t i, uint64_t a, uint32_t c) :
					n(i),
					amount(a),
					confirms(c) {
				memcpy(hash.u8, h.u8, sizeof(h));
			}

			bool operator<(const UTXO &otherUtxo) {
				if (UInt256Eq(&hash, &otherUtxo.hash))
					return UInt256LessThan(&hash, &otherUtxo.hash) == 1;
				else
					return n < otherUtxo.n;
			}

			UInt256 hash;
			uint32_t n;
			uint64_t amount;
			uint32_t confirms;
		};

		class UTXOList {
		public:
			bool Constains(const UInt256 &hash) const;

			UTXO &operator[](size_t i);

			size_t size() const;

			void Clear();

			const std::vector<UTXO> &GetUTXOs() const;

			void AddByTxInput(const TransactionInput &input, uint64_t amount, uint32_t confirms);

			void AddUTXO(const UInt256 &hash, uint32_t index, uint64_t amount, uint32_t confirms);

			void RemoveAt(size_t index);

			bool Compare(const UTXO &o1, const UTXO &o2) const;

			void SortBaseOnOutputAmount(uint64_t totalOutputAmount, uint64_t feePerKB);

		private:
			std::vector<UTXO> _utxos;
		};

	}
}

#endif //__ELASTOS_SDK_UTXOSET_H__
