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

#ifndef __ELASTOS_SDK_MERKLEBLOCKDATASOURCE_H__
#define __ELASTOS_SDK_MERKLEBLOCKDATASOURCE_H__

#include "Sqlite.h"
#include "TableBase.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace Elastos {
	namespace ElaWallet {

		class IMerkleBlock;
		typedef boost::shared_ptr<IMerkleBlock> MerkleBlockPtr;

		class MerkleBlockDataSource : public TableBase {

		public:
			MerkleBlockDataSource(Sqlite *sqlite, SqliteTransactionType type = IMMEDIATE);

			~MerkleBlockDataSource();

			virtual void InitializeTable();

			bool PutMerkleBlock(const MerkleBlockPtr &blockPtr);

			bool PutMerkleBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			bool DeleteMerkleBlock(long id);

			bool DeleteAllBlocks();

			std::vector<MerkleBlockPtr> GetAllMerkleBlocks(const std::string &chainID) const;

		private:
			bool PutMerkleBlockInternal(const MerkleBlockPtr &blockPtr);


		private:
			/*
			 * merkle block table
			 */
			const std::string MB_TABLE_NAME = "merkleBlockTable";
			const std::string MB_COLUMN_ID = "_id";
			const std::string MB_BUFF = "merkleBlockBuff";
			const std::string MB_HEIGHT = "merkleBlockHeight";
			const std::string MB_ISO = "merkleBlockISO";

			const std::string MB_DATABASE_CREATE = "create table if not exists " + MB_TABLE_NAME + " (" +
				MB_COLUMN_ID + " integer primary key autoincrement, " +
				MB_BUFF + " blob, " +
				MB_HEIGHT + " integer, " +
				MB_ISO + " text DEFAULT 'ELA');";
		};

	}
}


#endif //__ELASTOS_SDK_MERKLEBLOCKDATASOURCE_H__

