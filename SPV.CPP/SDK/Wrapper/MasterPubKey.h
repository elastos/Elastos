// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERPUBKEY_H__
#define __ELASTOS_SDK_MASTERPUBKEY_H__

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "BRBIP32Sequence.h"

#include "Wrapper.h"
#include "Key.h"
#include "CMemBlock.h"

namespace Elastos {
	namespace SDK {

		class MasterPubKey :
			public Wrapper<BRMasterPubKey> {
		public:

			MasterPubKey();

			MasterPubKey(const std::string &phrase);

			virtual std::string toString() const;

			virtual BRMasterPubKey *getRaw() const;

			CMBlock serialize() const;

			void deserialize(const CMBlock &data);

			CMBlock getPubKey() const;

			boost::shared_ptr<Key> getPubKeyAsKey() const;

			static CMBlock bip32BitIDKey(const CMBlock &seed, int index, const std::string &uri);

			static bool validateRecoveryPhrase(const std::vector<std::string> &words, const std::string &phrase);

			static CMBlock generatePaperKey(const CMBlock &seed, const std::vector<std::string> &words);

		private:
			boost::shared_ptr<BRMasterPubKey> _masterPubKey;
		};

		typedef boost::shared_ptr<MasterPubKey> MasterPubKeyPtr;

	}
}

#endif //__ELASTOS_SDK_MASTERPUBKEY_H__
