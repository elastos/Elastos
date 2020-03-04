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

#ifndef __ELASTOS_SDK_COINBASEUTXODATASTORE_H__
#define __ELASTOS_SDK_COINBASEUTXODATASTORE_H__

#include "TransactionNormal.h"

namespace Elastos {
	namespace ElaWallet {

		class UTXO;

		typedef boost::shared_ptr<UTXO> UTXOPtr;
		typedef std::vector<UTXOPtr> UTXOArray;

		class TransactionCoinbase : public TransactionNormal {
		public:
			explicit TransactionCoinbase(Sqlite *sqlite, SqliteTransactionType type = IMMEDIATE);

			~TransactionCoinbase();

			virtual void InitializeTable();

			virtual std::vector<TransactionPtr> GetAll(const std::string &chainID) const;

			void RemoveOld();
		private:
			std::vector<TransactionPtr> GetAllOld(const std::string &chainID) const;

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
