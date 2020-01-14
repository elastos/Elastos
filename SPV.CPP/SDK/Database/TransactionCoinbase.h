// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_COINBASEUTXODATASTORE_H__
#define __ELASTOS_SDK_COINBASEUTXODATASTORE_H__

#include "TransactionDataStore.h"

namespace Elastos {
	namespace ElaWallet {

		class UTXO;

		typedef boost::shared_ptr<UTXO> UTXOPtr;
		typedef std::vector<UTXOPtr> UTXOArray;

		class TransactionCoinbase : public TransactionDataStore {
		public:
			explicit TransactionCoinbase(Sqlite *sqlite);

			~TransactionCoinbase();

		private:
			bool ContainOldData() const;

			void ConvertFromOldData();

		private:
			std::string _tableNameOld;
			std::string _txHashOld;
			std::string _blockHeightOld;
			std::string _timestampOld;
			std::string _indexOld;
			std::string _programHashOld;
			std::string _assetIDOld;
			std::string _outputLockOld;
			std::string _amountOld;
			std::string _payloadOld;
			std::string _spentOld;
		};

	}
}


#endif //__ELASTOS_SDK_COINBASEUTXODATASTORE_H__
