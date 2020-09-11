/*
 * Copyright (c) 2020 Elastos Foundation
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

#include "RPCHelper.h"

namespace Elastos {
	namespace ElaWallet {

		typedef struct HttpResponseBody {
			size_t used;
			size_t sz;
			void *data;
		} HttpResponseBody;

		static size_t HttpResponseBodyWriteCallback(char *ptr,
													size_t size, size_t nmemb, void *userdata) {
			HttpResponseBody *response = (HttpResponseBody *) userdata;
			size_t length = size * nmemb;

			if (response->sz - response->used < length) {
				size_t new_sz;
				size_t last_try;
				void *new_data;

				if (response->sz + length < response->sz) {
					response->used = 0;
					return 0;
				}

				for (new_sz = response->sz ? response->sz << 1 : 512, last_try = response->sz;
					 new_sz > last_try && new_sz <= response->sz + length;
					 last_try = new_sz, new_sz <<= 1);

				if (new_sz <= last_try)
					new_sz = response->sz + length;

				new_sz += 16;

				new_data = realloc(response->data, new_sz);
				if (!new_data) {
					response->used = 0;
					return 0;
				}

				response->data = new_data;
				response->sz = new_sz;
			}

			memcpy((char *) response->data + response->used, ptr, length);
			response->used += length;

			return length;
		}

		RPCHelper::RPCHelper() {
			/* In windows, this will init the winsock stuff */
			curl_global_init(CURL_GLOBAL_ALL);

			/* get a curl handle */
			_curl = curl_easy_init();
		}

		RPCHelper::~RPCHelper() {
			if (_curl != nullptr) {
				/* always cleanup */
				curl_easy_cleanup(_curl);
			}

			curl_global_cleanup();
		}

		nlohmann::json RPCHelper::Get(const std::string &url) const {
			nlohmann::json j;
			if (_curl == nullptr) {
				return j;
			}
			HttpResponseBody response;

			curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1L);
			/* use a GET to fetch this */
			curl_easy_setopt(_curl, CURLOPT_HTTPGET, 1L);

			curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, HttpResponseBodyWriteCallback);
			curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &response);

			memset(&response, 0, sizeof(response));
			/* Perform the request */
			CURLcode rc = curl_easy_perform(_curl);
			if (rc != CURLE_OK) {
				fprintf(stderr, "RPC call error, status: %d, message: %s", rc, curl_easy_strerror(rc));
				if (response.data)
					free(response.data);

				return j;
			}

			((char *)response.data)[response.used] = 0;
			std::string jstring = std::string((char *)response.data);
			j = nlohmann::json::parse(jstring);
			free(response.data);

			return j;
		}

	}
}