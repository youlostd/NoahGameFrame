# -*- coding: utf-8 -*-

import chr
import chrmgr
import player

import localeInfo

EMOTION_VERSION = 2

EMOTION_DICT = {
    player.EMOTION_CLAP: {"name": localeInfo.EMOTION_CLAP, "desc": "", "command": "/clap"},
    player.EMOTION_CONGRATULATION: {"name": localeInfo.EMOTION_CONGRATULATION,"desc": "", "command": "/congratulation"},
    player.EMOTION_FORGIVE: {"name": localeInfo.EMOTION_FORGIVE,"desc": "", "command": "/forgive"},
    player.EMOTION_ANGRY: {"name": localeInfo.EMOTION_ANGRY,"desc": "", "command": "/angry"},
    player.EMOTION_ATTRACTIVE: {"name": localeInfo.EMOTION_ATTRACTIVE,"desc": "", "command": "/attractive"},
    player.EMOTION_SAD: {"name": localeInfo.EMOTION_SAD,"desc": "", "command": "/sad"},
    player.EMOTION_SHY: {"name": localeInfo.EMOTION_SHY,"desc": "", "command": "/shy"},
    player.EMOTION_CHEERUP: {"name": localeInfo.EMOTION_CHEERUP,"desc": "", "command": "/cheerup"},
    player.EMOTION_BANTER: {"name": localeInfo.EMOTION_BANTER,"desc": "", "command": "/banter"},
    player.EMOTION_JOY: {"name": localeInfo.EMOTION_JOY,"desc": "", "command": "/joy"},
    player.EMOTION_THROW_MONEY: {"name": localeInfo.EMOTION_THROW_MONEY,"desc": "", "command": "/throw_money"},
    player.EMOTION_CHEERS_1: {"name": localeInfo.EMOTION_CHEERS_1,"desc": "", "command": "/cheer1"},
    player.EMOTION_CHEERS_2: {"name": localeInfo.EMOTION_CHEERS_2,"desc": "", "command": "/cheer2"},
    player.EMOTION_KISS: {"name": localeInfo.EMOTION_CLAP_KISS,"desc": "", "command": "/kiss"},
    player.EMOTION_FRENCH_KISS: {"name": localeInfo.EMOTION_FRENCH_KISS,"desc": "", "command": "/french_kiss"},
    player.EMOTION_SLAP: {"name": localeInfo.EMOTION_SLAP,"desc": "", "command": "/slap"},
    player.EMOTION_DANCE_1: {"name": localeInfo.EMOTION_DANCE_1, "desc": "", "command": "/dance1"},
    player.EMOTION_DANCE_2: {"name": localeInfo.EMOTION_DANCE_2, "desc": "", "command": "/dance2"},
    player.EMOTION_DANCE_3: {"name": localeInfo.EMOTION_DANCE_3, "desc": "", "command": "/dance3"},
    player.EMOTION_DANCE_4: {"name": localeInfo.EMOTION_DANCE_4, "desc": "", "command": "/dance4"},
    player.EMOTION_DANCE_5: {"name": localeInfo.EMOTION_DANCE_5, "desc": "", "command": "/dance5"},
    player.EMOTION_DANCE_6: {"name": localeInfo.EMOTION_DANCE_6, "desc": "", "command": "/dance6"},
    player.EMOTION_DANCE_7: {"name": localeInfo.EMOTION_DANCE_7, "desc": "", "command": "/dance7"},
    player.EMOTION_PUSH_UP: {"name": localeInfo.EMOTION_PUSH_UP, "desc": "", "command": "/pushup"},
    player.EMOTION_EXERCISE: {"name": localeInfo.EMOTION_EXERCISE, "desc": "", "command": "/exercise"},
    player.EMOTION_DOZE: {"name": localeInfo.EMOTION_DOZE, "desc": "", "command": "/doze"},
    player.EMOTION_SELFIE: {"name": localeInfo.EMOTION_SELFIE, "desc": "", "command": "/selfie"},
    player.EMOTION_CHARGING: {"name": localeInfo.EMOTION_CHARGING, "desc": "", "command": "(charging)"},
    player.EMOTION_WEATHER_1: {"name": localeInfo.EMOTION_WEATHER_1, "desc": "", "command": "(weather1)"},
    player.EMOTION_WEATHER_2: {"name": localeInfo.EMOTION_WEATHER_2, "desc": "", "command": "(weather2)"},
    player.EMOTION_WEATHER_3: {"name": localeInfo.EMOTION_WEATHER_3, "desc": "", "command": "(weather3)"},
    player.EMOTION_HUNGRY: {"name": localeInfo.EMOTION_HUNGRY, "desc": "", "command": "(hungry)"},
    player.EMOTION_SIREN: {"name": localeInfo.EMOTION_SIREN, "desc": "", "command": "(siren)"},
    player.EMOTION_LETTER: {"name": localeInfo.EMOTION_LETTER, "desc": "", "command": "(letter)"},
    player.EMOTION_CALL: {"name": localeInfo.EMOTION_CALL, "desc": "", "command": "(call)"},
    player.EMOTION_CELEBRATION: {"name": localeInfo.EMOTION_CELEBRATION, "desc": "", "command": "(celebration)"},
    player.EMOTION_ALCOHOL: {"name": localeInfo.EMOTION_ALCOHOL, "desc": "", "command": "(alcohol)"},
    player.EMOTION_BUSY: {"name": localeInfo.EMOTION_BUSY, "desc": "", "command": "(busy)"},
    player.EMOTION_NOSAY: {"name": localeInfo.EMOTION_NOSAY, "desc": "", "command": "(nosay)"},
    player.EMOTION_WHIRL: {"name": localeInfo.EMOTION_WHIRL, "desc": "", "command": "(whirl)"},

}

ICON_DICT = {
    player.EMOTION_CLAP: "d:/ymir work/ui/game/windows/emotion_clap.sub",
    player.EMOTION_CHEERS_1: "d:/ymir work/ui/game/windows/emotion_cheers_1.sub",
    player.EMOTION_CHEERS_2: "d:/ymir work/ui/game/windows/emotion_cheers_2.sub",
    player.EMOTION_CONGRATULATION: "icon/action/congratulation.tga",
    player.EMOTION_FORGIVE: "icon/action/forgive.tga",
    player.EMOTION_ANGRY: "icon/action/angry.tga",
    player.EMOTION_ATTRACTIVE: "icon/action/attractive.tga",
    player.EMOTION_SAD: "icon/action/sad.tga",
    player.EMOTION_SHY: "icon/action/shy.tga",
    player.EMOTION_CHEERUP: "icon/action/cheerup.tga",
    player.EMOTION_BANTER: "icon/action/banter.tga",
    player.EMOTION_JOY: "icon/action/joy.tga",
    player.EMOTION_THROW_MONEY: "icon/action/throw_money.tga",
    player.EMOTION_DANCE_1: "icon/action/dance1.tga",
    player.EMOTION_DANCE_2: "icon/action/dance2.tga",
    player.EMOTION_DANCE_3: "icon/action/dance3.tga",
    player.EMOTION_DANCE_4: "icon/action/dance4.tga",
    player.EMOTION_DANCE_5: "icon/action/dance5.tga",
    player.EMOTION_DANCE_6: "icon/action/dance6.tga",
    player.EMOTION_DANCE_7: "icon/action/dance7.tga",

    player.EMOTION_PUSH_UP: "icon/action/pushup.tga",
    player.EMOTION_EXERCISE: "icon/action/exercise.tga",
    player.EMOTION_DOZE: "icon/action/doze.tga",
    player.EMOTION_SELFIE: "icon/action/selfie.tga",
    player.EMOTION_CHARGING: "icon/action/charging.tga",
    player.EMOTION_WEATHER_1: "icon/action/weather1.tga",
    player.EMOTION_WEATHER_2: "icon/action/weather2.tga",
    player.EMOTION_WEATHER_3: "icon/action/weather3.tga",
    player.EMOTION_HUNGRY: "icon/action/hungry.tga",
    player.EMOTION_SIREN: "icon/action/siren.tga",
    player.EMOTION_LETTER: "icon/action/letter.tga",
    player.EMOTION_CALL: "icon/action/call.tga",
    player.EMOTION_CELEBRATION: "icon/action/celebration.tga",
    player.EMOTION_ALCOHOL: "icon/action/alcohol.tga",
    player.EMOTION_BUSY: "icon/action/busy.tga",
    player.EMOTION_NOSAY: "icon/action/nosay.tga",
    player.EMOTION_WHIRL: "icon/action/whirl.tga",

    player.EMOTION_KISS: "d:/ymir work/ui/game/windows/emotion_kiss.sub",
    player.EMOTION_FRENCH_KISS: "d:/ymir work/ui/game/windows/emotion_french_kiss.sub",
    player.EMOTION_SLAP: "d:/ymir work/ui/game/windows/emotion_slap.sub",
}

ANI_DICT = {
    chr.MOTION_CLAP: "clap.msa",
    chr.MOTION_CHEERS_1: "cheers_1.msa",
    chr.MOTION_CHEERS_2: "cheers_2.msa",
    chr.MOTION_DANCE_1: "dance_1.msa",
    chr.MOTION_DANCE_2: "dance_2.msa",
    chr.MOTION_DANCE_3: "dance_3.msa",
    chr.MOTION_DANCE_4: "dance_4.msa",
    chr.MOTION_DANCE_5: "dance_5.msa",
    chr.MOTION_DANCE_6: "dance_6.msa",
    chr.MOTION_DANCE_7: "dance_7.msa",

    chr.MOTION_EMOTION_PUSH_UP: "pushup.msa",
    chr.MOTION_EMOTION_EXERCISE: "exercise.msa",
    chr.MOTION_EMOTION_DOZE: "doze.msa",
    chr.MOTION_EMOTION_SELFIE: "selfie.msa",

    chr.MOTION_CONGRATULATION: "congratulation.msa",
    chr.MOTION_FORGIVE: "forgive.msa",
    chr.MOTION_ANGRY: "angry.msa",
    chr.MOTION_ATTRACTIVE: "attractive.msa",
    chr.MOTION_SAD: "sad.msa",
    chr.MOTION_SHY: "shy.msa",
    chr.MOTION_CHEERUP: "cheerup.msa",
    chr.MOTION_BANTER: "banter.msa",
    chr.MOTION_JOY: "joy.msa",
    chr.MOTION_THROW_MONEY: "ridack_animoney.msa",
    chr.MOTION_FRENCH_KISS_WITH_WARRIOR: "french_kiss_with_warrior.msa",
    chr.MOTION_FRENCH_KISS_WITH_ASSASSIN: "french_kiss_with_assassin.msa",
    chr.MOTION_FRENCH_KISS_WITH_SURA: "french_kiss_with_sura.msa",
    chr.MOTION_FRENCH_KISS_WITH_SHAMAN: "french_kiss_with_shaman.msa",
    chr.MOTION_KISS_WITH_WARRIOR: "kiss_with_warrior.msa",
    chr.MOTION_KISS_WITH_ASSASSIN: "kiss_with_assassin.msa",
    chr.MOTION_KISS_WITH_SURA: "kiss_with_sura.msa",
    chr.MOTION_KISS_WITH_SHAMAN: "kiss_with_shaman.msa",
    chr.MOTION_SLAP_HIT_WITH_WARRIOR: "slap_hit.msa",
    chr.MOTION_SLAP_HIT_WITH_ASSASSIN: "slap_hit.msa",
    chr.MOTION_SLAP_HIT_WITH_SURA: "slap_hit.msa",
    chr.MOTION_SLAP_HIT_WITH_SHAMAN: "slap_hit.msa",
    chr.MOTION_SLAP_HURT_WITH_WARRIOR: "slap_hurt.msa",
    chr.MOTION_SLAP_HURT_WITH_ASSASSIN: "slap_hurt.msa",
    chr.MOTION_SLAP_HURT_WITH_SURA: "slap_hurt.msa",
    chr.MOTION_SLAP_HURT_WITH_SHAMAN: "slap_hurt.msa",
}

def __RegisterSharedEmotionAnis(mode, path):
    chrmgr.SetPathName(path)
    chrmgr.RegisterMotionMode(mode)

    for key, val in ANI_DICT.items():
        chrmgr.RegisterMotionData(mode, key, val)


def RegisterEmotionAnis(path):
    actionPath = path + "action/"
    weddingPath = path + "wedding/"

    __RegisterSharedEmotionAnis(chr.MOTION_MODE_GENERAL, actionPath)
    __RegisterSharedEmotionAnis(chr.MOTION_MODE_WEDDING_DRESS, actionPath)

    chrmgr.SetPathName(weddingPath)
    chrmgr.RegisterMotionMode(chr.MOTION_MODE_WEDDING_DRESS)
    chrmgr.RegisterMotionData(chr.MOTION_MODE_WEDDING_DRESS, chr.MOTION_WAIT, "wait.msa")
    chrmgr.RegisterMotionData(chr.MOTION_MODE_WEDDING_DRESS, chr.MOTION_WALK, "walk.msa")
    chrmgr.RegisterMotionData(chr.MOTION_MODE_WEDDING_DRESS, chr.MOTION_RUN, "walk.msa")

