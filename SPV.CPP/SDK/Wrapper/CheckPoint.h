// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CHECKPOINT_H__
#define __ELASTOS_SDK_CHECKPOINT_H__

#include <SDK/Wrapper/Wrapper.h>
#include <SDK/Common/Mstream.h>

#include <Core/BRChainParams.h>

namespace Elastos {
	namespace ElaWallet {

		class CheckPoint : public Wrapper<const BRCheckPoint> {
		public:
			CheckPoint();

			explicit CheckPoint(const BRCheckPoint &checkPoint);

			virtual std::string toString() const;

			virtual const BRCheckPoint *getRaw() const;

		private:
			JSON_SM_LS(CheckPoint);
			JSON_SM_RS(CheckPoint);
			TO_JSON(CheckPoint);
			FROM_JSON(CheckPoint);

		private:
			BRCheckPoint _checkPoint;
		};

	}
}

#endif //__ELASTOS_SDK_CHECKPOINT_H__
