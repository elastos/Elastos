// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <fstream>
#include <catch.hpp>
#include <boost/filesystem.hpp>

#include "KeyStore/KeyStore.h"

using namespace Elastos::ElaWallet;

TEST_CASE("createAccountFromJson test", "[createAccountFromJson]") {

	SECTION("open") {

		std::string password = "s12345678";
		nlohmann::json j = "{\"iv\":\"neksof2hUCccAvdcMeYOqg==\",\"v\":1,\"iter\":10000,\"ks\":128,\"ts\":64,\"mode\":\"ccm\",\"adata\":\"\",\"cipher\":\"aes\",\"salt\":\"Sv8OvJi34Rs=\",\"ct\":\"qGWsLoHyWuZ/2ARas9G+qXfuIOj67oULVlIJOyK/n1HrhlVVUYukeh9Q3TWMdsKFFQdmLIrhd0dI9dic/VU/YulJrCQK7kvTeYTp9/Tp3INbU6XPygrt48addEYTQNmkcplHasuSGupYnXFGgfhda6F5ABRy41xHf4vbqDh9+7b+k/QyIyTqglVpIKJEiJ+J5/8lkVuIILm2OWNYdh/dMO+/s4gapLvGncBN8uZrpY2GlGDIVMhcH0B+hKsorDSSBssMn3E7JOXTlym2T+wC0Qy1FiUTmsCk7TQu4NpUa3OsPx1Lcp4Ow7AjkI7xmbR48mEBZbLD1GtkRwQiS5j4cp25iGHeeEM3jsdTnOSVa4sZpsnzq2jxEgHVeLmPwZdWwWfKrQo47gH2c95F1hPI7J4eGGLeN9l2YF0JBS9Zl6zQO/jge+PUZolibpKAU9OoGzZF+nCjoLZ9IJJrPIXxVycDquHd07WSGUoQgK+e+BybH5zlbIb3dtMjJFFXI6oXj1+G0ZgN5MKXjnyhcMHf1ojefK3A4BejoQ3dPav34PVJyWPUW3XdZLtfb/7KUk2jgx06kZgcIMFG2QIvK6uexH3KvIcPteVSSDVQ4oSj+HAKDX5qvfku7Yqb7FE04NjB4bRJIhvTngXbW6kuz2iowkc+V1Q2ssfV/js7/IIde1CGfy/fk0qbUNt5QYDgtmX4YmphcCdby67Uaba+YklqinHOJWfS+/pAJ2+FJ/WSS6bUvZljEATsbZdY25xexx1PZ0h5mhIC8tZHOQntcU37AH+DFj/fePqm+SNnXdiKC4ZX2Al8l6hU2yOYCswtWMd1uF2ZVZbZ1xoRkx9A/NOyQt3+/i/qrsa2or4hrg3Dob/sef8zPNlUoMBgvgQ/W0xczugmI0E+Ojh9k4ibzTQMzwicvgYYD2oLnFAlYIUUcRpgE1MQ5ETxeRJsg5tyEz7tEenaOBVbM/xkUDvcbzwKpa+7irBF+fKtyHZseJzTJxfNAfKdyy8smuFqwILrEBWJ4Ci0FYKG2CjlHPqsNK5U0v2YnyYs1nmuxnRhl7v/5QJcgtFZIDbGDuzILCde5NwAv2rsyb7FxFW/1Mic+YbZg3lMmSpa6yOQ89/5v+K3XyFzoad8LFxrjzxsChj1EFci6wc8MNoUlTbMYeO+jhbIRqYje6U4kUKMuglq1CAZEb0KQAK+FHF37Ap8Gd3RR2VE5bQ8K6G4+Tx71c+RLZYqPkDn8STCiGcrXLkrNZfSo1iLYqA0vIfpyZHmkTAjxdWMgBBBAKu503qwSamZYwN23SsAvg/qxj++NMsHKRek98j7jwrZWmNZDfe6W+RAEcrG2LWKcHVTyKWLKm5rE+bi1cKB5W9ZTPCLGkN/jw8tDa25BYd2rMA5mqeGShBqSbG/acw05rQFAgVKvfCZRyMjieGMoZ9Pk68xxABn5ZNFjaogFOylt3VOtL/Dz92wmin+6Q/1oRX8qCkavdVmJu230L2Z7JpVAeoRc2usm+bGRdbvdVC9j3NFW6Mu8+8AcN5ZPGm8yzG7yP2MaGPSGO/PiXc/CUtZNUGQW27BDaClj4EtKIRqp76lDam4e88pLgp67O5naUdcdB4ISmlkleDH4OMdtqSoSj/YDHZvg7rta9SSEDloNB0Y7X+JCaI7czLPZp4CfKHjzfmqflrxZ7YYZLhgkKHdU8xHhS4xg5punUGQHxYlS2xGAAdBSBJ8WaeTtdNXVXYa2b02zJNMSRaWyvnHh5IZezg2P5f4g8MTvInbDmO66vMOe12A/hpf/Zyolw==\"}"_json;
		KeyStore ks("Data");
		ks.open(j, password);

		IAccount *account = ks.createAccountFromJson("", password);
		REQUIRE(account->GetType() == "Standard");
	}
}