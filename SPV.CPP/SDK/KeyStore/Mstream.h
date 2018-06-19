// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef __ELASTOS_SDK__MSTREAM_H__
#define __ELASTOS_SDK__MSTREAM_H__

namespace Elastos {
	namespace ElaWallet {

#define JSON_SM_LS(T) \
        friend nlohmann::json &operator<<(nlohmann::json&, const T&);
#define JSON_SM_RS(T) \
        friend const nlohmann::json &operator>>(const nlohmann::json&, T&);
#define TO_JSON(T) \
		friend void to_json(nlohmann::json&, const T&);
#define FROM_JSON(T) \
		friend void from_json(const nlohmann::json&, T&);

	}
}

#endif //__ELASTOS_SDK__MSTREAM_H__
