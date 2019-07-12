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

		class UTXO {
		public:
			UTXO();

			UTXO(const UTXO &u);

			UTXO(const uint256 &h, uint16_t i, const BigInt &a);

			virtual ~UTXO();

			bool operator<(const UTXO &otherUtxo);

			const uint256 &Hash() const;

			void SetHash(const uint256 &hash);

			const uint16_t &Index() const;

			void SetIndex(uint16_t index);

			const BigInt &Amount() const;

			void SetAmount(const BigInt &amount);

			UTXO &operator=(const UTXO &u);

		protected:
			friend class UTXOList;
			friend class GroupedAsset;

			uint256 _hash;
			uint16_t _n;
			BigInt _amount;
		};

		class CoinBaseUTXO : public UTXO {
		public:
			CoinBaseUTXO();

			~CoinBaseUTXO();

			const uint256 &AssetID() const;

			void SetAssetID(const uint256 &assetID);

			const uint168 &ProgramHash() const;

			void SetProgramHash(const uint168 &programHash);

			const uint32_t &OutputLock() const;

			void SetOutputLock(uint32_t outputLock);

			const uint32_t &BlockHeight() const;

			void SetBlockHeight(uint32_t height);

			const time_t &Timestamp() const;

			void SetTimestamp(time_t t);

			bool Spent() const;

			void SetSpent(bool status);

			uint32_t GetConfirms(uint32_t lastBlockHeight) const;



		private:
			uint256 _assetID;
			uint168 _programHash;
			uint32_t _outputLock;
			uint32_t _blockHeight;
			time_t _timestamp;
			bool _spent;
		};

		typedef boost::shared_ptr<CoinBaseUTXO> CoinBaseUTXOPtr;

		class UTXOList {
		public:
			bool Contains(const UTXO &o) const;

			bool Contains(const uint256 &hash, uint32_t n) const;

			bool Contains(const TransactionInput &input) const;

			UTXO &operator[](size_t i);

			size_t size() const;

			void Clear();

			const std::vector<UTXO> &GetUTXOs() const;

			void AddByTxInput(const TransactionInput &input, const BigInt &amount);

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
