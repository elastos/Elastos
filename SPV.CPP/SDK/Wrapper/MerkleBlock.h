// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MERKLEBLOCK_H__
#define __ELASTOS_SDK_MERKLEBLOCK_H__

#include <boost/shared_ptr.hpp>

#include "BRMerkleBlock.h"

#include "Wrapper.h"

namespace Elastos {
    namespace SDK {

        class MerkleBlock :
            public Wrapper<BRMerkleBlock *> {

        public:
            MerkleBlock(BRMerkleBlock *merkleBlock);

            ~MerkleBlock();

            virtual std::string toString() const;

            virtual BRMerkleBlock *getRaw() const;

            //todo complete me

        private:

            BRMerkleBlock *_merkleBlock;
        };

        typedef boost::shared_ptr<MerkleBlock> MerkleBlockPtr;

    }
}

#endif //__ELASTOS_SDK_MERKLEBLOCK_H__
