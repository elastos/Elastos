// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UTXO_H__
#define __ELASTOS_SDK_UTXO_H__

#include <SDK/Common/uint256.h>

#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class TransactionInput;
		class TransactionOutput;
		class BigInt;
		typedef boost::shared_ptr<TransactionOutput> OutputPtr;
		typedef boost::shared_ptr<TransactionInput> InputPtr;

		class UTXO {
		public:
			UTXO();

			UTXO(const UTXO &u);

			UTXO &operator=(const UTXO &u);

			UTXO(const uint256 &hash, uint16_t i, time_t t, uint32_t h, const OutputPtr &o);

			UTXO(const TransactionInput &input);

			virtual ~UTXO();

			bool operator==(const UTXO &u) const;

			const uint256 &Hash() const;

			void SetHash(const uint256 &hash);

			const uint16_t &Index() const;

			void SetIndex(uint16_t index);

			bool Spent() const;

			void SetSpent(bool status);

			time_t Timestamp() const;

			void SetTimestamp(time_t t);

			uint32_t BlockHeight() const;

			void SetBlockHeight(uint32_t h);

			const OutputPtr &Output() const;

			uint32_t GetConfirms(uint32_t lastBlockHeight) const;

			bool Equal(const InputPtr &input) const;

			bool Equal(const uint256 &hash, uint16_t index) const;

		protected:
			time_t _timestamp;
			uint32_t _blockHeight;
			bool _spent;
			OutputPtr _output;

			uint256 _hash;
			uint16_t _n;
		};
		typedef boost::shared_ptr<UTXO> UTXOPtr;
		typedef std::vector<UTXOPtr> UTXOArray;

	}
}

#endif //__ELASTOS_SDK_UTXO_H__
