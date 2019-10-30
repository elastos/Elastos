// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Utils.h"

#include <random>

#define DID_MAX_LEN      512

namespace Elastos {
	namespace ElaWallet {

		uint8_t Utils::getRandomByte() {
			std::random_device rd;
			std::mt19937_64 gen(rd());
			std::uniform_int_distribution<> dis(0, 255);
			auto dice = std::bind(dis, gen);
			return dice();
		}

		bytes_t Utils::GetRandom(size_t bytes) {
			bytes_t out(bytes);

			for (size_t i = 0; i < out.size(); i++) {
				out[i] = Utils::getRandomByte();
			}

			return out;
		}

		bool Utils::ParseInternetTime(const std::string &utcTime, time_t &timeStamp) {
			char *ptr;
			struct tm ptm;
			size_t len;
			char string_copy[DID_MAX_LEN];

			if (!timeStamp || utcTime.empty())
				return false;

			memset(&ptm, 0, sizeof(ptm));

			len = utcTime.size();
			if (utcTime[len - 1] != 'Z')
				return false;

			strcpy(string_copy, utcTime.c_str());
			string_copy[len - 1] = '\0';      //remove the last 'Z'

			ptr = strrchr(string_copy, ':');
			if (!ptr)
				return false;
			ptm.tm_sec = atoi(ptr + sizeof(char));
			*ptr = '\0';

			ptr = strrchr(string_copy, ':');
			if (!ptr)
				return false;
			ptm.tm_min = atoi(ptr + sizeof(char));
			*ptr = '\0';

			ptr = strrchr(string_copy, 'T');
			if (!ptr)
				return false;
			ptm.tm_hour = atoi(ptr + sizeof(char));
			*ptr = '\0';

			ptr = strrchr(string_copy, '-');
			if (!ptr)
				return false;
			ptm.tm_mday = atoi(ptr + sizeof(char));
			*ptr = '\0';

			ptr = strrchr(string_copy, '-');
			if (!ptr)
				return false;
			ptm.tm_mon = atoi(ptr + sizeof(char)) - 1;
			*ptr = '\0';

			ptm.tm_year = atoi(string_copy) - 1900;

			timeStamp = mktime(&ptm);
			return true;
		}
	}
}

