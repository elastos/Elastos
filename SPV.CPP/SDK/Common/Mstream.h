/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ELASTOS_SDK__MSTREAM_H__
#define __ELASTOS_SDK__MSTREAM_H__

#include <nlohmann/json.hpp>

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
