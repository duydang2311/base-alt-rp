-- --------------------------------------------------------
-- Máy chủ:                      127.0.0.1
-- Server version:               10.4.8-MariaDB - mariadb.org binary distribution
-- Server OS:                    Win64
-- HeidiSQL Phiên bản:           10.2.0.5599
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- Dumping database structure for alt-rp
CREATE DATABASE IF NOT EXISTS `alt-rp` /*!40100 DEFAULT CHARACTER SET latin1 */;
USE `alt-rp`;

-- Dumping structure for table alt-rp.items
CREATE TABLE IF NOT EXISTS `items` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `model` int(11) NOT NULL DEFAULT 0,
  `name` varchar(64) NOT NULL DEFAULT '',
  `description` varchar(64) NOT NULL DEFAULT '',
  `type` tinyint(4) NOT NULL DEFAULT 0,
  `capacity` tinyint(4) NOT NULL DEFAULT 1,
  `mergeable` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=45 DEFAULT CHARSET=latin1;

-- Dumping data for table alt-rp.items: ~43 rows (approximately)
/*!40000 ALTER TABLE `items` DISABLE KEYS */;
INSERT INTO `items` (`id`, `model`, `name`, `description`, `type`, `capacity`, `mergeable`) VALUES
	(1, 331, 'Brass Knuckles', 'Vu khi can chien', 1, 1, 0),
	(2, 333, 'Golf Club', 'Vu khi can chien', 1, 1, 0),
	(3, 334, 'Nightstick', 'Vu khi can chien', 1, 1, 0),
	(4, 335, 'Knife', 'Vu khi can chien', 1, 1, 0),
	(5, 336, 'Baseball Bat', 'Vu khi can chien', 1, 1, 0),
	(6, 337, 'Shovel', 'Vu khi can chien', 1, 1, 0),
	(7, 338, 'Pool Cue', 'Vu khi can chien', 1, 1, 0),
	(8, 339, 'Katana', 'Vu khi can chien', 1, 1, 0),
	(9, 341, 'Chainsaw', 'Vu khi can chien', 1, 1, 0),
	(10, 321, 'Purple Dildo', 'Vu khi can chien', 1, 1, 0),
	(11, 322, 'Dildo', 'Vu khi can chien', 1, 1, 0),
	(12, 323, 'Vibrator', 'Vu khi can chien', 1, 1, 0),
	(13, 324, 'Silver Vibrator', 'Vu khi can chien', 1, 1, 0),
	(14, 325, 'Flowers', 'Vu khi can chien', 1, 1, 0),
	(15, 326, 'Cane', 'Vu khi can chien', 1, 1, 0),
	(16, 342, 'Grenade', 'Bom', 1, 1, 0),
	(17, 343, 'Tear Gas', 'Bom khoi', 1, 1, 0),
	(18, 344, 'Molotov Cocktail', 'Bom lua', 1, 1, 0),
	(19, 346, '9mm', 'Sung luc', 1, 1, 0),
	(20, 347, 'Silenced 9mm', 'Sung luc', 1, 1, 0),
	(21, 348, 'Desert Eagle', 'Sung luc', 1, 1, 0),
	(22, 349, 'Shotgun', 'Sung Shotgun', 1, 1, 0),
	(23, 350, 'Sawnoff Shotgun', 'Sung Shotgun', 1, 1, 0),
	(24, 351, 'Combat Shotgun', 'Sung Shotgun', 1, 1, 0),
	(25, 352, 'Uzi', 'Sung tieu lien', 1, 1, 0),
	(26, 353, 'MP5', 'Sung tieu lien', 1, 1, 0),
	(27, 355, 'AK-47', 'Sung truong', 1, 1, 0),
	(28, 356, 'M4', 'Sung truong', 1, 1, 0),
	(29, 372, 'Tec-9', 'Sung tieu lien', 1, 1, 0),
	(30, 357, 'Country Rifle', 'Sung ban tia', 1, 1, 0),
	(31, 358, 'Sniper Rifle', 'Sung ban tia', 1, 1, 0),
	(32, 359, 'RPG', 'Sung chong tang', 1, 1, 0),
	(33, 360, 'HS Rocket', 'Sung chong tang', 1, 1, 0),
	(34, 361, 'Flamethrower', 'Sung thoi lua', 1, 1, 0),
	(35, 362, 'Minigun', 'Sung may', 1, 1, 0),
	(36, 363, 'Satchel Charge', 'Bom kich hoat', 1, 1, 0),
	(37, 364, 'Detonator', 'Thiet bi kich hoat bom', 1, 1, 0),
	(38, 365, 'Spraycan', 'Binh xit', 1, 1, 0),
	(39, 366, 'Fire Extinguisher', 'Binh chua chay', 1, 1, 0),
	(40, 367, 'Camera', 'Thiet bi chup hinh', 1, 1, 0),
	(41, 368, 'Night Vision Goggles', 'Kinh nhin bong toi', 1, 1, 0),
	(42, 369, 'Thermal Goggles', 'Kinh nhin nhiet do', 1, 1, 0),
	(43, 1212, 'Money', 'Tien mat, co the gop vat pham', 0, 1, 1),
	(44, 11746, 'Chia khoa', 'Mo khoa phuong tien, nha cua,...', 0, 1, 0);
/*!40000 ALTER TABLE `items` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
