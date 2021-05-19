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

		typedef struct HttpRequestBody {
			size_t used;
			size_t sz;
			char *data;
		} HttpRequestBody;

		static size_t HttpRequestBodyReadCallback(void *dest, size_t size,
												  size_t nmemb, void *userdata)
		{
			HttpRequestBody *request = (HttpRequestBody *)userdata;
			size_t length = size * nmemb;
			size_t bytes_copy = request->sz - request->used;

			if (bytes_copy) {
				if(bytes_copy > length)
					bytes_copy = length;

				memcpy(dest, request->data + request->used, bytes_copy);

				request->used += bytes_copy;
				return bytes_copy;
			}

			return 0;
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

		nlohmann::json RPCHelper::Post(const std::string &url, const std::string &rawData) const {
			nlohmann::json j;
			if (_curl == nullptr) {
				return j;
			}
			HttpRequestBody request;
			request.used = 0;
			request.sz = rawData.size();
			request.data = (char *)rawData.c_str();
			curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(_curl, CURLOPT_POST, 1L);
			curl_easy_setopt(_curl, CURLOPT_READFUNCTION, HttpRequestBodyReadCallback);
			curl_easy_setopt(_curl, CURLOPT_READDATA, &request);
			curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, (long)request.sz);
			HttpResponseBody response;
			curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, HttpResponseBodyWriteCallback);
			curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &response);

			struct curl_slist *headers = NULL;
			headers = curl_slist_append(headers, "Content-Type: application/json");
//			headers = curl_slist_append(headers, "Accept: application/json");
			curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, headers);

			memset(&response, 0, sizeof(response));
			CURLcode rc = curl_easy_perform(_curl);
			curl_slist_free_all(headers);
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