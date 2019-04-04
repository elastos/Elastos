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
-- Table structure for table `chain_vote_info`
--

DROP TABLE IF EXISTS `chain_vote_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chain_vote_info` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `producer_public_key` varchar(66) COLLATE utf8mb4_bin NOT NULL,
  `vote_type` varchar(24) COLLATE utf8mb4_bin NOT NULL COMMENT 'CRC or Delegate',
  `txid` varchar(64) COLLATE utf8mb4_bin NOT NULL,
  `n` tinyint(2) NOT NULL COMMENT 'Index of output',
  `value` varchar(24) COLLATE utf8mb4_bin NOT NULL COMMENT 'the number of vote',
  `outputlock` int(11) NOT NULL,
  `address` varchar(34) COLLATE utf8mb4_bin NOT NULL,
  `block_time` int(11) NOT NULL,
  `height` int(11) NOT NULL,
  `local_system_time` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2019-03-06 13:53:23
alter table chain.chain_vote_info add is_valid varchar(10) COMMENT 'YES | NO' DEFAULT 'YES';
alter table chain.chain_vote_info add cancel_height int(11) COMMENT 'invalid height';
CREATE INDEX idx_chain_vote_info_address ON chain_vote_info (address);
CREATE INDEX idx_chain_vote_info_producer_public_key ON chain_vote_info (producer_public_key);
