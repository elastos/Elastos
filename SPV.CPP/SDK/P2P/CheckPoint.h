// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CHECKPOINT_H__
#define __ELASTOS_SDK_CHECKPOINT_H__

#include <SDK/Common/Mstream.h>
#include <SDK/Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {

		class CheckPoint {
		public:
			CheckPoint(uint32_t height, const std::string &hash, time_t timestamp, uint32_t target);

			CheckPoint(uint32_t height, const uint256 &hash, time_t timestamp, uint32_t target);

			~CheckPoint();

			CheckPoint &operator=(const CheckPoint &checkpoint);

			const uint32_t &GetHeight() const;

			const uint256 &GetHash() const;

			const time_t &GetTimestamp() const;

			const uint32_t &GetTarget() const;

		private:
			uint32_t _height;
			uint256 _hash;
			time_t _timestamp;
			uint32_t _target;
		};

	}
}

#endif //__ELASTOS_SDK_CHECKPOINT_H__
