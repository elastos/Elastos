// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDPATH_H__
#define __ELASTOS_SDK_IDPATH_H__

namespace Elastos {
	namespace SDK {

		struct IdPath {
			IdPath() :
					Purpose(0),
					Index(0) {

			}

			IdPath(uint32_t purpose, uint32_t index) :
					Purpose(purpose),
					Index(index) {
			}

			inline bool operator<(const IdPath &b) {
				if (Purpose != b.Purpose)
					return Purpose < b.Purpose;
				return Index < b.Index;
			}

			inline bool operator==(const IdPath &b) {
				return Purpose == b.Purpose && Index == b.Index;
			}

			uint32_t Purpose;
			uint32_t Index;
		};

	}
}

#endif //__ELASTOS_SDK_IDPATH_H__
