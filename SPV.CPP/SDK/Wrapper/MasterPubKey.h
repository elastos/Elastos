// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERPUBKEY_H__
#define __ELASTOS_SDK_MASTERPUBKEY_H__

#include <string>
#include <boost/shared_ptr.hpp>

#include "BRBIP32Sequence.h"

#include "Wrapper.h"
#include "ByteData.h"

namespace Elastos {
    namespace SDK {

        class MasterPubKey :
            public Wrapper<BRMasterPubKey *> {
        public:

            MasterPubKey();

            MasterPubKey(const std::string& phrase);

            virtual std::string toString() const;

            virtual BRMasterPubKey* getRaw();

            ByteData serialize() const;

            void deserialize(const ByteData& data);

            ByteData getPubKey() const;

            boost::shared_ptr<BRKey> getPubKeyAsKey() const;

            static ByteData bip32BitIDKey(ByteData seed, int index, std::string uri);

            static bool validateRecoveryPhrase(std::vector<std::string> words, std::string phrase);

            static ByteData generatePaperKey (ByteData seed, std::vector<std::string> words);

        private:
            boost::shared_ptr<BRMasterPubKey> _masterPubKey;
        };

        typedef boost::shared_ptr<MasterPubKey> MasterPubKeyPtr;

    }
}

#endif //__ELASTOS_SDK_MASTERPUBKEY_H__
