-- MySQL dump 10.13  Distrib 8.0.13, for macos10.14 (x86_64)
--
-- Host: localhost    Database: chain
-- ------------------------------------------------------
-- Server version	8.0.12

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
 SET NAMES utf8 ;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `chain_cmc_price`
--

DROP TABLE IF EXISTS `chain_cmc_price`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chain_cmc_price` (
  `id` varchar(256) COLLATE utf8mb4_bin NOT NULL,
  `name` varchar(256) COLLATE utf8mb4_bin NOT NULL,
  `symbol` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `rank` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `price_usd` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `price_cny` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `price_btc` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `24h_volume_usd` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `market_cap_usd` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `available_supply` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `total_supply` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `max_supply` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `percent_change_1h` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `percent_change_24h` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `percent_change_7d` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `last_updated` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `24h_volume_btc` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `market_cap_btc` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `local_system_time` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `24h_volume_cny` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `market_cap_cny` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `platform_symbol` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `platform_token_address` varchar(256) COLLATE utf8mb4_bin NOT NULL,
  `num_market_pairs` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `_id` int(11) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`_id`),
  KEY `idx_chain_cmc_price` (`symbol`)
) ENGINE=InnoDB AUTO_INCREMENT=601 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2019-02-25 18:07:06
CREATE INDEX idx_chain_cmc_price ON chain_cmc_price (symbol);