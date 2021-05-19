// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <nlohmann/json.hpp>
#include <WalletCore/BloomFilter.h>
#include <WalletCore/Address.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE( "BloomFilter test", "[BloomFilter]" ) {
	Log::registerMultiLogger();

	nlohmann::json addrJsonArray = R"([
		"ERLrzNjk8i3U2kocWSZbatqDABAoiAFpPt", "EHwuWDorzLcpJMAG9pqBBDG2cWKv9SPf6W",
		"EZW9b8XebHcjwstj2zKJP4qzF7jeAGuhfU", "EbbJyLiztacZr3LhpuYsZdwjaQU5mcKmPC",
		"EJFW2GHy3C9RFgUn6U2Xkj5YBvGPameSfL", "EN5yKRrJ9ke8774v4QqvZbcjKH2ejBjjrL",
		"ETBYJSFj89xgRRPhkSSYyaYnsn9ZX4h4Ju", "EPAkHbmxB8L2TCNADWH2LiywZh5W7abzXb",
		"EavXXBiytaCeZkgTPHJ2XQpYbNQGo2PMLR", "EfUMtQVFBYrGE63xqrRBQEgxgBN2CkQM9X",
		"ERMSjZS2gkfdo2jnwFaPXM3UxeFmBXS9DU", "EPBBs2ikPghVFuw684JXHT2ggf7yPkQRJo",
		"EM8RZYK3Ug8HQ5D1ENkhYocrKCqvfdTfkU", "EKXLgCfHDqb11jjahNqdC3kXVoHhnfHTCx",
		"EWvQjdTWzJQRVqV6ZwcmiB6uD8c3iKDdb8", "EVqFz6o9xufVEdKEMkRHriHsA1a9A2SuBZ",
		"EKKPCfnT6hm99qXAizR7evq1RMnRAGknR9", "ETpfRezHepDDWPm4ZoyBoLjpYaeJTYcQaP",
		"EThMuK1YaDQ7r27PzgncjaVAxwDwvsXhNx", "EKbi1JRHEHfXCpoDSga1bgwGrXdn6aeNtf",
		"EXpBXPuLboUx5whfeFHifZ2FK4qsjPWgPv", "Eer6hQzFWZQVM2o2UgSrLYMwRoZ5YoCZoE",
		"ET3icNtfu89UHDTPdm1H39R6Gn5CQBWKc3", "Eccs9p6oreiHWbVqwBRwSzuvZaBo75vzaZ",
		"EJgDxopdsptSc9GhrpsbCpBJwRYCFbarLz", "EVXoVGYwceq4ir2Z5TnKgE6h9UnRdb5vSi",
		"EauthD812iHgyLNdKNkV8sLdgMTro6CHJw", "EWPQ5gWiPxfqqbePwvocchowpBem5tByQu",
		"EeiB4FUDUMLMiLKovNrufQuHMHDu5sfTPF", "ETJrnWfZ6Xa85N3UvqAHDbdLkDC1ePFXSh",
		"EHpL8a3vBhbjYCMg6CvFErRsqgogzgBamV", "EHUMByUmSfGvXwgE2kvUfaaL561sQ5GCvQ",
		"EV3zYrJLAY4yCoG7SFEW9AC6gz7JNYPdoS", "Edxp6MEbNjhzNkVafJNu3FJ7JbMt3T3V4f",
		"EJDPVu2w8LSdTJSf8WVdmdMEbDzpkb39xn", "ESAsa9G6xEsq8ppu1K41RcPHibymGVT2SA",
		"EbhPr1QZ3iwsZFc4WyJFrvKMgXFnJFGQF6", "Eb9Fowru9cxEFdK79KEMBN1o1msbm615kM",
		"ERntVU1US6N7K2YSj7e7GJbaCkzcnZdSmA", "EU8838QCXLVL77i9kFngzH8wBbRtNah1hN",
		"EcG6PBgbqAt6DCDQVWTSqfgxQbCno1QFqE", "ENfSBSBuBtgUxSSijHQbFq2y3CwGyhfzN8",
		"EJwq5aQqJx1jBKSQNcwvhrSJJnBgqmkoia", "EWUvETtjLFY37vuGxT6m9wdujwrqs7tewA",
		"EWkAt1mtye3ahB3G8MaJneUf5iQxsKGfu6", "EUU3YLYn1GyQA1ncYJsC4fyjU5xm5XSdKj",
		"EfMdhX88ZcS1xevoJ9HCMe6jcAGjM1RzVW", "EQigbgwTZ8ZdRt5xtmEqNUv5ZoTmtLj2Fr",
		"EZwBjqLQmXVeaLTTqa7QaGuvmDGKFXnFSL", "ESFLKdVp1AEscSHvwBcnYy3WmCu3DhfqRd",
		"EKVWk8s3iJwV924VvUnp99kSx5arRmKAft", "EPDTnrMNQePfUv9XReVkn4Juk2r5ZCSteS",
		"ERqfveRQunArsoejQGZNPhAfFiZpoB2mRG", "Efh2cuyZ1Lgxik71zczdpCdeYLQYYNnDSn",
		"EbdDVfYAPUfRjQDYyruSTDWEtth1Aniqxh", "EVqd2gMzi9FHH9dyXeAMTNS9b4cGNz2ptw",
		"EWnwLcRTveQmMvywjugcMgxBwng8Sv9Nvk", "EbPeTqvrXbmsg46AYaxuEzxxSFGvvk3EUo",
		"EbdDK8G5hTm8gfXxDk7PcnFbB1GGQ8jfUZ", "ENN4n7ZKvoRS2vnebFcSHoAnWa5eNBRwJE",
		"ELwfeuG2KJzhY941X9hpKVvGVgFS8vgS8b", "EYC7FTUu8exkqrW4rH7pZVGEia8LVTEeEg",
		"Eff3cLLWF17hvfdvwJXPswh8W54B2ezfzt", "EcwoWxmyna5VMVwjvXb1U9yJwgWbpULLtM",
		"EdRHxVFhrkSmaAfRkAnwR27yavLKfQAvEP", "ETHPY6DU93a8JH91GxEArNtcnJYx1Zza7x",
		"EYG3tjsEnyS1APoB6MSyusAee2xEovtWkw", "EPmt9ZfWSRbtF6hCXvrSAN6ytP2ZiNab4Q",
		"EHYPdLGAeu7GbD36ZeEpjRYXqRJpQuyvTC", "EaDqF5xxhdEPQ3QWoxFmwEVFocpiLvmDwW",
		"EMWuAWXLMVWMAAgzpQqUi2AZc8xr2B3T7e", "ES6GfyZmjoZzgPc8y8bvVKeUvgNM1xhiZ3",
		"EeCBL94jTkAk4bLbG8PB8V9N1gAcPGsFYh", "EZdjpZ9ZMvEx8YFPKbbRkcKne4qcCUEvMe",
		"EPnnfzp8AxxyEZBpS6jBdytMWjLZbG8tNJ", "EVx727ArNh2Qkqhk5ufi91SXeUikum5VPW",
		"ES6jPb8pCH96ZFjPSCSsViPkGjQDbtGqwn", "EUtcXqX8zMQpCe9Y2E4KZbkibnQQnHjUXy",
		"EbdPADR9B3rEPcyg3vqCtoSMyNmHnZxGxU", "ETsBkyaWNLW4b9wC8JnKmqzzdWQUDK3f69",
		"EZfhh2ervfLU9YEm43pog3tW6Ds2tpeEht", "EeM6MP2rXUtMj3KifGTCfJKFQyMm2mSRnc",
		"EZxuAscPjZwr4TejHkJwtwtAL1QBdYMueD", "ER5FdGcYT5Jd8kqxEZK6Lz1BXXriJdbRgV",
		"EffuLf2LXaXoXCWh6g4owNpXvxexs54iqx", "EesDxWMeVGDuh2gozrQ88MFAFg7w9Jswdt",
		"EP8ZhBWTFE7h3GEc4a31uVRx3wHf4ghqBa", "EfyNx1v415wKyX2D6V1nDjvjZFf966p1WZ",
		"EJDpCnSHUboNyGdwsjdPK5grDGue8NnmbZ", "Ed5iyqL7fnyLncfPxDmyNWr6wVeu7xn4wc",
		"ESzcqYm29k4PSNZofa6HjfYsxuw4vT3YGS", "EgBLk9F7gdeQMGDQBZtkZ52DLXqTiGcaQA",
		"EXpnk834nMQnootnikeDZctvsmEqRQjX9t", "EeH62vYRVfJvEakJ4qgefhNx4Xc1nMgn7G",
		"EXXq8ymUbDQRFsMzMqL7dnuDMNBXCpqKui", "EMcY2BADb6cPKVmTi8ayzynKd5UN6spzJF",
		"ESUwVWug1Jpm2fZiFF7VkGnx7atxJeakpu", "EYj55oMvQugq1WQyPvBEWoxeMoYpo4KNTe",
		"EepJimgtyVNTBHgWTrXaZUsU3xYy7YkZRj", "EYhjHp5StC8T2Gfaky2BQzpyr7Ypy2VW2p",
		"Ebjfi7Bq46rLTJf83UbKDcTrvzEbMeL8tp", "EWeHttsms8oAQJCta7mSwkEzaqMKaHhwWa",
		"EKE7Qq981Jhbrap6dWncEpjNTTxhxaHjd3", "ENxSW4g3Z1vJajZEZKz9RRHPDCnjcWG9QH",
		"EekQ8bREBSdi5qFrfT5aJ3gLoSwiZbuTKJ", "EZf7cTiWrGsNoA3jYrvo8XGjmW5rj3frgj",
		"EHNyVMorACaza9SThNN27CcZND2Vfkuok2", "ESNdxtDaWG9VxJ1yTjz3FL1oKe4fXpwCfH",
		"ELwVuexAcN9CCBu2rWa5euK9Mjun6mKKHy", "ETqKjXVmrPN3VooJez4x2uzVMLbpTc3avu",
		"EPxvDgV6PyJC1pbs7r9Bxwo8PUvTKZDCoh", "EJzmiUp9r4CxRLEYA66myLux3PwAqAehx3",
		"EWRwQdeQsazhoRwDtYVuvoPfPBS1QtF3CQ", "EU9KNLqSYgpbf5L4MchWjK86gQAaf1ZSTR",
		"EW726qGLG4MhsQmndKiV2bdsL7kT38G26e", "EYxKXWAn4dvegPQVbPDkdxK8S5PKRVV6cn",
		"EWjzyqkkyfM8wp9dciGoKmrUwgj1bDvGED", "EfLrtn62BtR8znFnHG37PPvpSR8bnvvxVV",
		"EKdFX23ecRdomNaVysD7HK7DgGR7JhhFDv", "ERSVLboVaCrKdAdnkTtyQsnrUyN6L8b3NE",
		"EbVK7JZrVBoJEEiSYjk4KHQs7wqoyWyZX6", "EKvYbARYUBv8Ag5rfQHyqFQhhBk5DbfuRN",
		"Eg145uDGWJLrm3Y3sBMAHaw4f6gGPidVQW", "Ee4ZkbJ18GNAg2hLBXU3mnYUxva37wZceW",
		"EWeWQ6tshcWff73iLgcf34av4bM3qeDJVR", "EapeEszy9jCA3BjtM7FANiXachEXzXAWSv",
		"Eeovie7A7rFYMXr319s4fcfhJJtF3oMEpN", "Ef3Ms9jGv3WjKTrW1KrNea9CYGSLeD3Eq5",
		"EJH3Czmos2wiyVHLMARYKRDmCweKZNHmCg", "ENH6nod23d2VWSZj6dK7ZNizRMw3FjSiyH",
		"EPou9v5YR74GmeduAjJEkTj7q61rgAKcnk", "EP2pF82S7v3iwXnb3QtYnxeqQoMMPrJrRK",
		"EbWCoQDwx32BtPkh46Kd9Hi9E4V8r1qhkb", "EKZiCuPdXJ7ogpyz8HApoZ22cMZGoaH2pd",
		"Ef6GC3fg2kSyAMW29erpkHoGZxbne55kcY", "ENhGERpFomx1nVTqEKA3fAT2DiBGpDGzB6",
		"EZW71tNLLF7JWAGaXLZVjFnqt98UmRmmYN", "EVvvR14niNz4kcF62CgfKfJn9MJ6nrijnG",
		"ETnt8U1wG6ZfYKtzpPZny1QADbaDZpEfpx", "EX7TRQ9FQxJkdHe5StWfY7iowg13F1M4JH",
		"EZGGsVia1CBpZqFp5xWHR4fgxX85DQkwwm", "ENnZMuLwcFG1EMPoQiLTMuvpxRNwbzpbeH",
		"EZhgNMdQqiDBSS3TELh7deRazt3xvh3Gau", "ESzmHudPSw5Fy7dWF8pHLVRP7Rm7dSnBRf",
		"EYQBC3tdFomzT5CwPFVbGDw4tU8KYzUWhp", "Ea7nVzJvdp4963BJRpAVqZsfzzKVenZGeD",
		"EKEdxohENmqeeFtLLRTYNdakbCvB8Kbp2i", "EfjTkSd6bf8mQyHwm73975FKubkr7SSjzV",
		"EUrtUoupDczRPsAgevXLTMi5rUqgKRBmFc", "ERAQMuoVo6uEjyx92qnPhma3qamEa9ZU2N",
		"EfhAuH4N11iQ1u5U5RxfQjGBxoPur5M2uw", "EXGTapWjawRk4YTfJZ5PGW5eyQn7bgQWf8",
		"EaZcMAsgpaPLj8yTasP5BcJz9Kh8yxcVnc", "EbBaGPVFaaCHCpYSdTCR5BxFu2xeZGucm3",
		"EcHjbEcbdiAB3kwNPYEmUUTVR43nxMbEZW", "EYq3boB86WyhdQTTtyXkwaCjxHFDo42oy3",
		"EfmJSa1mX46T9gong8jkLgAtX1oVQXTYXJ", "EQXXcc7c3sUd8wkypVXeQXk4GdW1JNs1UQ",
		"EHiLPXy6QaJKLHCR22YJzAhkH55mpHzWE5", "EWBwESDmtCPi6BkYSqRa2UZ3rNM9zEBDwJ",
		"Eg66iN6dZ4RG6PFvZJ5Ftk5KddALTpSWr2", "ELcU6F1kC1zySBYsSqcFxAACkbBgYk6jAv",
		"EV49axTEr2YLKTZhxbtBRPDYZiiWtZmoq6", "EYe9QrDipf9eRoErXMG1GVVC4HNbwUfni7",
		"EgUrBmyfUNNest2aSTWDhiEKnWo2LUrmLV", "EMjVTZUgZ5jxdqvmPgeARkewp5hQbKn7ud",
		"EJLJJVMJDtbSc9kVgXYPCL4ZAb5wWeZycH", "EV4PB5ziCUE2MwYXn9BLV59hBbfTsJXWqR",
		"EK8AL6ALWZAqGPwvkcEV2bXvwDxtu9XN44", "EHodR8kdKUvUp4SP1hLbfPdbK22sp5HKwp",
		"EUCHAcHJesrGHyDeKH7KrytysL6AGD9rhs", "EZvw8kgkz1QHaJuiMWyh6dFsZ6jcpT1BR3",
		"ESHuJAnTfLLsFbeXfdPdsewrwbbNE2bqVH", "Ea7qKGjiYhyJtswidvZDjaCbn4hUfXpdRg",
		"EXuUBovM8eeLq427cRKTrV11Z4RjbjZzPL", "Edr1SqXbJwojrzb1MXKgUJh8juS1VD28UP",
		"EKHJekTwvEYJbF4WFhttqoSb2vNyY1TN2r", "EbejVaAY5Hxhzu3d6Apo8tXQAFzT1pkq3h",
		"EaZdRHe8N7YywHwJBya8AtamCuMDAyTLcC", "EQMWpr6aG8geWq1sdviTxmnh4vBMeqDj7z",
		"EKjKD2RLcrX5Ts5f6ZR76MSWsK9B5jzTu4", "ETzhzF8PZ389i8z8JPuZeFyhuquNjPsCzD",
		"ETRFXbgQLcgEMA5wyp2tmTqe7jZt9LLBHd", "EZXqx7gWtrsnsZXUHJDK423XeXqEt8yMkD",
		"EXQFFy5ADXVPvnCJjqyiD2tBWwk1EESH8r", "EazhFLTRBBp9QERToZa9KUN5NQMaez4mC9",
		"Eb9LdLh3bx99ApF7acLz2yLd9ektMVhrZ6", "ELMV4y4T1e5DtoyzAiCX9CVKuknkvrvq32",
		"EXJJ8BVFHK1HZRSvqTdbWKz298M6aaCY3a", "ESUc53tPGqHDUxoQsdRzN4RPd9wbs9WTGA",
		"EfczqPAR9B8sqbUXySMek7ExZaMBFENi5u", "EbzJGum9sTajrYgJ3BGbjPqtGHVL5NPJ6k",
		"Efp2dDUBfNKwzLABwN8f3HjyC3CZ7teZZy", "EfgD4Dceyp94cWGstKvjnM3S7kmw2H1mLb",
		"ERoRaD7UAxmxPgaFuhFdRrfvF5HQkvxo5L", "Eg3dfic9yxWvf6FhFu2MEgFpJMxfFnULsJ",
		"EdTqLWQVxXPNFHdV5cyqLovr6ZzXQaZZCE", "EYyfHbdzkQA91mgiUMaJqFPGQqEpKzSTYw",
		"EfjcZP8wCLfxNNJjEe6DfU7jC2p967Gcmz", "ELGxrcgNhXTmc1jhK413sXrd1CwG8SEaDe",
		"EPxKGsLnqShD3vSzbCDKFPeWJqJNapf4oD", "EaQPigirujnVKUUYrRuNsXKZdjt9TFA7hW",
		"EbV9SszETPKSp3s5eQExwjgPXM9nF2bJb4", "EVyt4XMrXwvUbi1W9FtLGeGJLcxaaaKEs5",
		"EMunuvxsunBC2qiPdqtdoXHyc9Y9NXkwdw", "EQ6cbu1hhxRhhKqT49LhBE2ymvvqSWPFUg",
		"EJ7DaRBaUfyfHs6ribtgzsVnnFrgkvQSmH", "EZHb9aEgYEGJrNi9XTyWU8mTtPSoVZ9UnY",
		"ESTFdKkerY6MMmNbjDWFEZYwwEZ6Cxmj4v", "EV8toPRHbf2eRo8u8PDaf9i7MfvnKmgnjb",
		"EVjBnhaYkMYuea5t1pnA9Y4EcWy2uj5qCp", "EVTSg4MHdtGrcJZwSvsrDdbHkdtxLvYVhS",
		"EPBMidZNoKjcqvUkHgKd6W5ZT8jyA1ht7L", "EMqTQRrHWHrVJUhjjYekGfUrGcgW9hMgRt",
		"EVfh7WQmeXgex6RGMBPrRyaK5M1YJZnH5f"])"_json;

	SECTION("filter test") {
		std::vector<Address> addrs;
		for (nlohmann::json::iterator it = addrJsonArray.begin(); it != addrJsonArray.end(); it++) {
			addrs.push_back(Address((*it).get<std::string>()));
		}

		REQUIRE(addrs.size() == addrJsonArray.size());

		for (size_t rnd = 0; rnd < 1; rnd++) {
			BloomFilterPtr filter = BloomFilterPtr(
				new BloomFilter(BLOOM_DEFAULT_FALSEPOSITIVE_RATE, addrs.size() + 100,
								(uint32_t) 0x12345678, BLOOM_UPDATE_ALL));

			bytes_t hash;
			for (size_t i = 0; i < addrs.size(); ++i) {
				hash = addrs[i].ProgramHash().bytes();
				REQUIRE(!filter->ContainsData(hash));
				filter->InsertData(hash);
			}

			for (size_t i = 0; i < addrs.size(); ++i) {
				hash = addrs[i].ProgramHash().bytes();
				REQUIRE(filter->ContainsData(hash));
			}
		}
	}

}

