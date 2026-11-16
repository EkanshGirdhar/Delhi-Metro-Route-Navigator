-- =====================================================================
-- Metro Route Navigation System - MySQL Database Schema & Seed Data
-- =====================================================================

CREATE DATABASE IF NOT EXISTS `metro_db` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE `metro_db`;

-- Drop existing tables if re-initializing
DROP TABLE IF EXISTS `connections`;
DROP TABLE IF EXISTS `stations`;

-- 1. Stations Table
CREATE TABLE `stations` (
    `id` INT NOT NULL,
    `name` VARCHAR(100) NOT NULL,
    `x` DOUBLE NOT NULL,
    `y` DOUBLE NOT NULL,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 2. Connections (Tracks) Table
CREATE TABLE `connections` (
    `id` INT NOT NULL,
    `u` INT NOT NULL,
    `v` INT NOT NULL,
    `weight` INT NOT NULL COMMENT 'Travel time in minutes',
    `line` VARCHAR(50) NOT NULL COMMENT 'Line name (Blue, Yellow, Orange, Green, Pink)',
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    CONSTRAINT `fk_station_u` FOREIGN KEY (`u`) REFERENCES `stations` (`id`) ON DELETE CASCADE,
    CONSTRAINT `fk_station_v` FOREIGN KEY (`v`) REFERENCES `stations` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- =====================================================================
-- Seed Data: 17 Delhi Metro Stations
-- =====================================================================
INSERT INTO `stations` (`id`, `name`, `x`, `y`) VALUES
(0,  'Dwarka',              70,  330),
(1,  'Janakpuri',            220, 330),
(2,  'Rajiv Chowk',          500, 330),
(3,  'Mandi House',          720, 330),
(4,  'Noida Sector',         930, 330),
(5,  'Kashmere Gate',        220, 180),
(6,  'Civil Lines',          360, 100),
(7,  'Azadpur',              540, 100),
(8,  'Model Town',           720, 120),
(9,  'Anand Vihar',          860, 200),
(10, 'Welcome',              220, 480),
(11, 'Hauz Khas',            360, 560),
(12, 'Saket',                540, 560),
(13, 'Kalkaji',              720, 510),
(14, 'Botanical Garden',     880, 450),
(15, 'Central Secretariat',  500, 210),
(16, 'Lajpat Nagar',         500, 450);

-- =====================================================================
-- Seed Data: 21 Interconnecting Tracks
-- =====================================================================
INSERT INTO `connections` (`id`, `u`, `v`, `weight`, `line`) VALUES
(0,  0,  1,  4,  'Blue'),
(1,  1,  10, 4,  'Orange'),
(2,  10, 11, 4,  'Orange'),
(3,  11, 12, 5,  'Orange'),
(4,  12, 16, 4,  'Green'),
(5,  16, 2,  5,  'Green'),
(6,  12, 13, 5,  'Orange'),
(7,  13, 14, 4,  'Orange'),
(8,  14, 4,  6,  'Orange'),
(9,  1,  2,  14, 'Blue'),
(10, 2,  3,  12, 'Blue'),
(11, 3,  4,  14, 'Blue'),
(12, 1,  5,  3,  'Yellow'),
(13, 5,  6,  3,  'Yellow'),
(14, 6,  7,  4,  'Yellow'),
(15, 7,  8,  4,  'Yellow'),
(16, 8,  9,  3,  'Yellow'),
(17, 9,  4,  4,  'Yellow'),
(18, 2,  15, 4,  'Pink'),
(19, 15, 7,  5,  'Pink'),
(20, 3,  13, 7,  'Pink');
