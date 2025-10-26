/*
 Navicat Premium Data Transfer

 Source Server         : Win64Local
 Source Server Type    : MariaDB
 Source Server Version : 110302 (11.3.2-MariaDB)
 Source Host           : localhost:3306
 Source Schema         : account

 Target Server Type    : MariaDB
 Target Server Version : 110302 (11.3.2-MariaDB)
 File Encoding         : 65001

 Date: 03/05/2024 23:35:49
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for account
-- ----------------------------
DROP TABLE IF EXISTS `account`;
CREATE TABLE `account`  (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `login` varchar(30) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  `lang` varchar(4) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'de',
  `hwid` varchar(64) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `use_hwid_prot` tinyint(1) NULL DEFAULT 0,
  `restricted_trade` tinyint(1) NOT NULL DEFAULT 0,
  `password` varchar(64) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  `old_password` varchar(64) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `social_id` varchar(13) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  `email` varchar(64) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  `hwid_activation_code` varchar(16) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT '',
  `create_time` datetime NOT NULL DEFAULT current_timestamp(),
  `is_testor` tinyint(1) NOT NULL DEFAULT 0,
  `status` varchar(8) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'OK',
  `reason` text CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `securitycode` int(11) NULL DEFAULT 0,
  `newsletter` tinyint(1) NULL DEFAULT 0,
  `empire` tinyint(4) NOT NULL DEFAULT 0,
  `name_checked` tinyint(1) NOT NULL DEFAULT 0,
  `availDt` datetime NOT NULL DEFAULT current_timestamp(),
  `mileage` int(11) NOT NULL DEFAULT 0,
  `cash` int(11) NOT NULL DEFAULT 0,
  `gold_expire` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `silver_expire` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `safebox_expire` datetime NOT NULL DEFAULT '2030-01-05 19:08:22',
  `autoloot_expire` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `fish_mind_expire` datetime NOT NULL DEFAULT '2030-01-05 19:08:22',
  `marriage_fast_expire` datetime NOT NULL DEFAULT '2030-01-05 19:08:22',
  `money_drop_rate_expire` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `shop_double_up_expire` datetime NOT NULL DEFAULT '2030-01-05 19:08:22',
  `secur_question` tinyint(1) NULL DEFAULT NULL,
  `secur_answer` varchar(255) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `total_cash` int(11) NOT NULL DEFAULT 0,
  `total_mileage` int(11) NOT NULL DEFAULT 0,
  `ip` varchar(255) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `last_pw_change` datetime NOT NULL DEFAULT current_timestamp(),
  `referrer` int(11) NULL DEFAULT NULL,
  `referral_level` smallint(6) NULL DEFAULT 0,
  `voted` tinyint(4) NULL DEFAULT NULL,
  `voted_at` datetime NULL DEFAULT NULL,
  `vote_registered_at` datetime NULL DEFAULT NULL,
  `coins` int(11) NOT NULL DEFAULT 0,
  `votecoins` int(11) NOT NULL DEFAULT 0,
  `web_admin` int(11) NOT NULL DEFAULT 0,
  `remember_token` varchar(255) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `real_name` varchar(32) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT '',
  `question1` varchar(64) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT '',
  `answer1` varchar(64) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT '',
  `lastvote` int(11) NULL DEFAULT 0,
  `got_item` smallint(6) NULL DEFAULT NULL,
  `pin_confirmation` varchar(64) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `lostpw_token` varchar(64) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `last_play` datetime NOT NULL DEFAULT current_timestamp(),
  `vote_special_expire` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `display_name` varchar(60) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT '',
  `last_vote_pinfo` datetime NULL DEFAULT NULL,
  `last_vote_pspy` datetime NULL DEFAULT NULL,
  `last_vote` datetime NULL DEFAULT NULL,
  `refer_key` varchar(255) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `refer_left_daily` int(11) NULL DEFAULT NULL,
  `ban_count` int(11) NULL DEFAULT NULL,
  `old_email` varchar(255) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT '',
  ` old_email_secure_code` varchar(255) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `login`(`login`) USING BTREE,
  INDEX `social_id`(`social_id`) USING BTREE,
  INDEX `hwid`(`hwid`) USING BTREE
) ENGINE = Aria AUTO_INCREMENT = 30 CHARACTER SET = ascii COLLATE = ascii_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of account
-- ----------------------------
INSERT INTO `account` VALUES (27, 'adalet', 'de', '925599c4bdd80c88d75258919fe72c0914fdc690710cc777c6670a11f3de2e19', 0, 0, '*A5462184B842F758C3D700C5B11FB0DE054CE8B7', NULL, '1234567', 'huso_35_355@hotmail.com', '', '2024-04-12 15:27:36', 0, 'OK', NULL, 1234, 0, 0, 0, '2024-04-12 15:27:36', 0, 82406, '0000-00-00 00:00:00', '0000-00-00 00:00:00', '2030-01-05 19:08:22', '0000-00-00 00:00:00', '2030-01-05 19:08:22', '2030-01-05 19:08:22', '0000-00-00 00:00:00', '2030-01-05 19:08:22', NULL, NULL, 0, 0, NULL, '2024-04-12 15:27:36', NULL, 0, NULL, NULL, NULL, 0, 0, 0, NULL, '', '', '', 0, NULL, NULL, NULL, '2024-04-12 15:27:36', '0000-00-00 00:00:00', '', NULL, NULL, NULL, NULL, NULL, NULL, '', NULL);
INSERT INTO `account` VALUES (28, 'adalet2', 'de', '925599c4bdd80c88d75258919fe72c0914fdc690710cc777c6670a11f3de2e19', 0, 0, '*A5462184B842F758C3D700C5B11FB0DE054CE8B7', NULL, '1234567', '', '', '2024-04-12 21:40:51', 0, 'OK', NULL, 1234, 0, 0, 0, '2024-04-12 21:40:51', 0, 52500, '0000-00-00 00:00:00', '0000-00-00 00:00:00', '2030-01-05 19:08:22', '0000-00-00 00:00:00', '2030-01-05 19:08:22', '2030-01-05 19:08:22', '0000-00-00 00:00:00', '2030-01-05 19:08:22', NULL, NULL, 0, 0, NULL, '2024-04-12 21:40:51', NULL, 0, NULL, NULL, NULL, 0, 0, 0, NULL, '', '', '', 0, NULL, NULL, NULL, '2024-04-12 21:40:51', '0000-00-00 00:00:00', '', NULL, NULL, NULL, NULL, NULL, NULL, '', NULL);
INSERT INTO `account` VALUES (29, 'adalet3', 'de', '925599c4bdd80c88d75258919fe72c0914fdc690710cc777c6670a11f3de2e19', 0, 0, '*A5462184B842F758C3D700C5B11FB0DE054CE8B7', NULL, '', '', '', '2024-04-18 20:51:31', 0, 'OK', NULL, 1234, 0, 0, 0, '2024-04-18 20:51:31', 0, 0, '0000-00-00 00:00:00', '0000-00-00 00:00:00', '2030-01-05 19:08:22', '0000-00-00 00:00:00', '2030-01-05 19:08:22', '2030-01-05 19:08:22', '0000-00-00 00:00:00', '2030-01-05 19:08:22', NULL, NULL, 0, 0, NULL, '2024-04-18 20:51:31', NULL, 0, NULL, NULL, NULL, 0, 0, 0, NULL, '', '', '', 0, NULL, NULL, NULL, '2024-04-18 20:51:31', '0000-00-00 00:00:00', '', NULL, NULL, NULL, NULL, NULL, NULL, '', NULL);

-- ----------------------------
-- Table structure for account_emails
-- ----------------------------
DROP TABLE IF EXISTS `account_emails`;
CREATE TABLE `account_emails`  (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `status` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `email` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `key` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `account_id` int(11) NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = Aria AUTO_INCREMENT = 1 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of account_emails
-- ----------------------------

-- ----------------------------
-- Table structure for account_hwid
-- ----------------------------
DROP TABLE IF EXISTS `account_hwid`;
CREATE TABLE `account_hwid`  (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `account_id` int(11) NULL DEFAULT NULL,
  `pc_name` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `hwid` int(11) NULL DEFAULT NULL,
  `status` tinyint(1) NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = Aria AUTO_INCREMENT = 1 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of account_hwid
-- ----------------------------

-- ----------------------------
-- Table structure for hguard
-- ----------------------------
DROP TABLE IF EXISTS `hguard`;
CREATE TABLE `hguard`  (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `account_id` int(11) NOT NULL,
  `hg_hash` varchar(120) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `cpu_id` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `guid` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `hdd_model` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `hdd_serial` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `mac` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `activation_code` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `validation_time` datetime NULL DEFAULT NULL,
  `creation_time` datetime NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = Aria AUTO_INCREMENT = 1 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of hguard
-- ----------------------------

-- ----------------------------
-- Table structure for hguard_mails
-- ----------------------------
DROP TABLE IF EXISTS `hguard_mails`;
CREATE TABLE `hguard_mails`  (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `hash` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL,
  `account_id` int(11) NULL DEFAULT NULL,
  `sent` tinyint(1) NULL DEFAULT NULL,
  `updated_at` datetime NULL DEFAULT NULL,
  `created_at` datetime NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = Aria AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of hguard_mails
-- ----------------------------

-- ----------------------------
-- Table structure for hwid_ban
-- ----------------------------
DROP TABLE IF EXISTS `hwid_ban`;
CREATE TABLE `hwid_ban`  (
  `hwid` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `ban_reason(sebep)` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL,
  PRIMARY KEY (`hwid`) USING BTREE
) ENGINE = Aria CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci PAGE_CHECKSUM = 1 ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of hwid_ban
-- ----------------------------

-- ----------------------------
-- Table structure for test
-- ----------------------------
DROP TABLE IF EXISTS `test`;
CREATE TABLE `test`  (
  `min_gold` bigint(20) NULL DEFAULT NULL,
  `max_gold` bigint(20) NULL DEFAULT NULL,
  `exp` bigint(20) NULL DEFAULT NULL
) ENGINE = Aria CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of test
-- ----------------------------

SET FOREIGN_KEY_CHECKS = 1;
