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
-- Table structure for table `chain_block_transaction_history`
--

DROP TABLE IF EXISTS `chain_block_transaction_history`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chain_block_transaction_history` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `address` varchar(34) COLLATE utf8mb4_bin NOT NULL,
  `txid` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `type` varchar(24) COLLATE utf8mb4_bin NOT NULL COMMENT 'address receiving/spending coin ,value accordingly is ''income''/''spend''',
  `value` bigint(20) NOT NULL COMMENT 'unit is sela,not ela',
  `createTime` int(11) NOT NULL,
  `height` int(11) NOT NULL,
  `fee` bigint(20) NOT NULL COMMENT 'unit is sela,not ela',
  `inputs` mediumtext COLLATE utf8mb4_bin,
  `outputs` mediumtext COLLATE utf8mb4_bin,
  `memo` mediumtext COLLATE utf8mb4_bin,
  `local_system_time` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='synced transaction history based on address';
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2018-12-10 10:32:03

CREATE INDEX idx_chain_block_transaction_history_address ON chain_block_transaction_history (address);
CREATE INDEX idx_chain_block_transaction_history_txid ON chain_block_transaction_history (txid);
ALTER TABLE chain_block_transaction_history ADD txType VARCHAR(24) NOT NULL;
CREATE INDEX idx_chain_block_transaction_history_txType_height ON chain_block_transaction_history (height,txType);
