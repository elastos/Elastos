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
#ifndef __ELASTOS_SDK__ADLSERIALIZER_H__
#define __ELASTOS_SDK__ADLSERIALIZER_H__

#include <nlohmann/json.hpp>
#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class JsonSerializer {
		public:
			virtual nlohmann::json ToJson() const = 0;

			virtual void FromJson(const nlohmann::json &j) = 0;
		};

	}
}

namespace nlohmann {
	template<typename T>
	struct adl_serializer<boost::shared_ptr<T>> {
		static void to_json(json &j, const boost::shared_ptr<T> &p) {
			j = p->ToJson();
		}

		static void from_json(const json &j, boost::shared_ptr<T> &p) {
			p = boost::shared_ptr<T>(new T());
			p->FromJson(j);
		}
	};

}
#endif //__ELASTOS_SDK__ADLSERIALIZER_H__
