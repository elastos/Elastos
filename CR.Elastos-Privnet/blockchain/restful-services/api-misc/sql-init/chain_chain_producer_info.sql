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
-- Table structure for table `chain_producer_info`
--

DROP TABLE IF EXISTS `chain_producer_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chain_producer_info` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `ownerpublickey` varchar(66) COLLATE utf8mb4_bin NOT NULL,
  `nodepublickey` varchar(66) COLLATE utf8mb4_bin NOT NULL,
  `nickname` text COLLATE utf8mb4_bin NOT NULL,
  `url` varchar(256) COLLATE utf8mb4_bin NOT NULL,
  `location` int(11) NOT NULL,
  `active` tinyint(1) NOT NULL,
  `votes` varchar(24) COLLATE utf8mb4_bin NOT NULL,
  `netaddress` varchar(124) COLLATE utf8mb4_bin NOT NULL,
  `state` varchar(24) COLLATE utf8mb4_bin NOT NULL,
  `registerheight` int(11) NOT NULL,
  `cancelheight` int(11) NOT NULL,
  `inactiveheight` int(11) NOT NULL,
  `illegalheight` int(11) NOT NULL,
  `index` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `idx_chain_producer_info` (`ownerpublickey`)
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

-- Dump completed on 2019-04-02 15:52:47
CREATE INDEX idx_chain_producer_info ON chain_producer_info (ownerpublickey);