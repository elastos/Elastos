// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERPUBKEY_H__
#define __ELASTOS_SDK_MASTERPUBKEY_H__

#include <SDK/Common/ByteStream.h>
#include <Core/BRInt.h>

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class MasterPubKey {
		public:

			MasterPubKey();

			MasterPubKey(const MasterPubKey &masterPubKey);

			MasterPubKey(const CMBlock &pubKey, const UInt256 &chainCode);

			~MasterPubKey();

			MasterPubKey &operator=(const MasterPubKey &masterPubKey);

			void Serialize(ByteStream &stream) const;

			bool Deserialize(ByteStream &stream);

			void SetFingerPrint(uint32_t fingerPrint);

			uint32_t GetFingerPrint() const;

			void SetPubKey(const CMBlock &pubKey);

			CMBlock GetPubKey() const;

			void SetChainCode(const UInt256 &chainCode);

			const UInt256 &GetChainCode() const;

			void Clean();

			bool Empty() const;

		private:
			uint32_t _fingerPrint;
			UInt256 _chainCode;
			uint8_t _pubKey[33];
		};

		typedef boost::shared_ptr<MasterPubKey> MasterPubKeyPtr;

	}
}

#endif //__ELASTOS_SDK_MASTERPUBKEY_H__
