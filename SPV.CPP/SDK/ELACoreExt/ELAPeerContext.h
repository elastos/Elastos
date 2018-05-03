// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAPEERCONTEXT_H__
#define __ELASTOS_SDK_ELAPEERCONTEXT_H__

#include <map>
#include <vector>

#include <BRMerkleBlock.h>
#include <BRPeerMessages.h>

namespace Elastos {
	namespace SDK {

		struct UInt256Compare {
			bool operator()(const UInt256& x, const UInt256& y) const;
		};

		struct ELAPeerContext {
			BRPeerContext innerContex;

			std::map<BRMerkleBlock *, std::vector<UInt256> > blockTxListMap;
			std::map<UInt256, BRMerkleBlock *, UInt256Compare> txBlockMap;
		};

		BRPeer *ELAPeerNew(uint32_t magicNumber);

		BRPeer *ELAPeerCopy(const BRPeer *peer);

		void ELAPeerFree(BRPeer *peer);

	}
}

#endif //__ELASTOS_SDK_ELAPEERCONTEXT_H__
