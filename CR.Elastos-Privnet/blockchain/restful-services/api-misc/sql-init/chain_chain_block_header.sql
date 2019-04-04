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
-- Table structure for table `chain_block_header`
--

DROP TABLE IF EXISTS `chain_block_header`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chain_block_header` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `hash` varchar(64) COLLATE utf8mb4_bin NOT NULL COMMENT 'block hash',
  `weight` int(64) NOT NULL,
  `height` int(64) NOT NULL COMMENT 'block height',
  `version` int(2) NOT NULL,
  `merkleroot` varchar(64) COLLATE utf8mb4_bin NOT NULL COMMENT 'merkle root',
  `time` int(64) NOT NULL COMMENT 'block time',
  `nonce` BIGINT(64) NOT NULL,
  `bits` int(64) NOT NULL,
  `difficulty` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `chainwork` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `previous_block_hash` varchar(64) COLLATE utf8mb4_bin NOT NULL COMMENT 'previous block hash',
  `next_block_hash` varchar(64) COLLATE utf8mb4_bin NOT NULL COMMENT 'next block hash',
  `miner_info` varchar(64) COLLATE utf8mb4_bin NOT NULL COMMENT ' miner name',
  `local_system_time` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='block header';
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2019-01-09 14:53:22

CREATE INDEX idx_chain_block_header_height ON chain_block_header (height);
CREATE INDEX idx_chain_block_header_hash ON chain_block_header (hash);
alter table chain_block_header add size int(64) default 0;