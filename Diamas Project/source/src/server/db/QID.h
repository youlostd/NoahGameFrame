
/**
 * @version	05/06/10 Bang2ni - ������ �������� ���� �߰�(QID_ITEMPRICE_XXX)
 */

#ifndef METIN2_SERVER_DB_QID_H
#define METIN2_SERVER_DB_QID_H


#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


enum QID
{
    QID_PLAYER,				// 0
    QID_ITEM,				// 1
    QID_QUEST,				// 2
    QID_AFFECT,				// 3
	QID_AUTH_LOGIN,			// 4
	QID_SAFEBOX_LOAD,			// 5
    QID_SAFEBOX_CHANGE_SIZE,		// 6
    QID_SAFEBOX_CHANGE_PASSWORD,	// 7
    QID_SAFEBOX_CHANGE_PASSWORD_SECOND,	// 8
    QID_SAFEBOX_SAVE,			// 9
    QID_ITEM_SAVE,			// 10
    QID_ITEM_DESTROY,			// 11
    QID_QUEST_SAVE,			// 12
    QID_PLAYER_SAVE,			// 13
	QID_PLAYER_LIST,			// 14

    QID_HIGHSCORE_REGISTER,		// 15
    QID_PLAYER_DELETE,			// 16
    QID_ITEM_AWARD_LOAD,		// 17
	QID_ITEM_AWARD_TAKEN,		// 19
    QID_GUILD_RANKING,			// 20

	// MYSHOP_PRICE_LIST
    QID_ITEMPRICE_SAVE,			///< 21, ������ �������� ���� ����
    QID_ITEMPRICE_DESTROY,		///< 22, ������ �������� ���� ����
    QID_ITEMPRICE_LOAD_FOR_UPDATE,	///< 23, �������� ������Ʈ�� ���� ������ �������� �ε� ����
    QID_ITEMPRICE_LOAD,			///< 24, ������ �������� �ε� ����
	// END_OF_MYSHOP_PRICE_LIST

	QID_PET,					// 25
	QID_SKILL,
	QID_QUICKSLOT,
	QID_TITLE,
	QID_MESSENGER_BLOCK,
	QID_GET_HGUARD_INFO,
	QID_VALIDATE_HGUARD_CODE,
	QID_SWITCHBOT_SLOT_DATA,
#ifdef ENABLE_BATTLE_PASS
	QID_BATTLE_PASS,
#endif
    QID_HUNTING_MISSIONS,
    QID_DUNGEON_INFO,
    QID_AUTH_REGISTER_NAME_CHECK,
    QID_AUTH_REGISTER,
};
#endif /* METIN2_SERVER_DB_QID_H */
