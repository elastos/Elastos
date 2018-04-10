// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MerkleBlock.h"

namespace Elastos {
    namespace SDK {

        MerkleBlock::MerkleBlock(BRMerkleBlock *merkleBlock) :
                _merkleBlock(merkleBlock) {
        }

        MerkleBlock::~MerkleBlock() {
            if (_merkleBlock != nullptr)
                BRMerkleBlockFree(_merkleBlock);
        }

        std::string MerkleBlock::toString() const {
            //todo complete me
            return "";
        }

        BRMerkleBlock *MerkleBlock::getRaw() const {
            return _merkleBlock;
        }

    }
}