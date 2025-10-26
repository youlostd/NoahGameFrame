# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import logging

import app
import player

import localeInfo
import ui
import uiCommon
import uiScriptLocale


class RouletteWindow(ui.ScriptWindow):
    edge_effect_pos_tuple = (
        (25, 46),
        (69, 46),
        (113, 46),
        (157, 46),
        (201, 46),
        (245, 46),
        (245, 90),
        (245, 134),
        (245, 178),
        (245, 222),
        (245, 266),
        (201, 266),
        (157, 266),
        (113, 266),
        (69, 266),
        (25, 266),
        (25, 222),
        (25, 178),
        (25, 134),
        (25, 90),
    )

    spin_time_before_request = ((40, 60), (20, 40, 60))
    spin_count_before_request = ((20, 5), (20, 15, 5))
    beginning_spin_time_after_request = (60, 120)
    middle_spin_time_after_request = (
        (
            160,
            200,
            240,
            280,
            320,
            340,
            350,
            360,
            370,
            380
        ),
        (
            160,
            200,
            240,
            280,
            320,
            340,
            340,
            340,
            340,
            340
        ),
    )
    last_spin_time_after_request = (
        400, 600
    )

    ROULETTE_SLOT_MAX = 20
    USE_LIMIT_RANGE = 2000
    RESPONSE_TIME_OUT = 10000

    def __init__(self):
        ui.ScriptWindow.__init__(self)

        self.is_loaded = 0
        self.board = None
        self.item_slot = None
        self.spin_button = None
        self.slot_edge_effect = None
        self.item_tooltip = None
        self.popup_dlg = None
        self.item_data_tuple = None
        self.special_end_time = 0
        self.start_pos_x = 0
        self.start_pos_y = 0
        self.index_generator = None
        self.reward_index = -1

        self.__SpinInitialize()
        self.__LoadWindow()

    def Destroy(self):
        pass

    def __SpinInitialize(self):
        self.once = 0
        self.is_spin = 0
        self.spin_lock = 0
        self.request_time = 0
        self.start_spin_time = 0
        self.move_time = 0
        self.update_time = 0
        self.update_gap = 0
        self.total_gap = 0
        self.is_request = 0
        self.cur_index = 0
        self.index_generator = self.__SlotIndexGenerator()
        self.spin_data = []
        if self.slot_edge_effect:
            (x, y) = self.edge_effect_pos_tuple[self.cur_index]
            self.slot_edge_effect.SetPosition(x, y)

    def __LoadWindow(self):
        if self.is_loaded:
            return

        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/MiniGameRouletteWindow.py")

            self.board = self.GetChild("board")
            self.board.SetCloseEvent(self.Close)

            self.item_slot = self.GetChild("item_slot")
            self.item_slot.SetOverInItemEvent(self.OverInSlot)
            self.item_slot.SetOverOutItemEvent(self.OverOutSlot)

            self.spin_button = self.GetChild("spin_button")
            self.spin_button.SetEvent(self.ClickSpinButton)

            self.slot_edge_effect = self.GetChild("slot_edge_effect")

            self.popup_dlg = uiCommon.PopupDialog()
            self.popup_dlg.SetText(localeInfo.ROULETTE_REWARD_TEXT)
            self.popup_dlg.Hide()

            self.is_loaded = True
        except Exception as e:
            logging.exception(e)

    def SetItemToolTip(self, tooltip):
        self.item_tooltip = tooltip

    def OnPressEscapeKey(self):
        self.Close()

    # so basically a generator that starts with zero and on each call grows by 1 and when it reaches the last
    # possible position it starts from zero again
    def __SlotIndexGenerator(self):
        slot_index_max = self.ROULETTE_SLOT_MAX
        slot_index_min = 0
        cur_slot_index = slot_index_min
        while True:
            if cur_slot_index < slot_index_max:
                yield cur_slot_index
                cur_slot_index += 1
            else:
                cur_slot_index = slot_index_min

    def Close(self, is_force=False, is_send_packet=False):
        if not is_force:
            if self.is_spin:
                return

        if self.popup_dlg and self.popup_dlg.IsShow():
            self.popup_dlg.Close()

        self.item_data_tuple = None
        self.Hide()
        self.__SpinInitialize()
        if is_send_packet:
            pass
            # appInst.instance().GetNet().SendMiniGameRouletteClose()

    def Show(self):
        self.__LoadWindow()
        self.__SpinInitialize()
        ui.ScriptWindow.Show(self)
        self.SetCenterPosition()
        # (self.start_pos_x, self.start_pos_y) = player.GetMainCharacterPosition()

    def ClickSpinButton(self):
        if self.spin_lock:
            return

        if self.is_spin:
            return

        if self.popup_dlg and self.popup_dlg.IsShow():
            return

        # appInst.instance().GetNet().SendMiniGameRouletteStart()
        self.__StartSpin()

    def OnUpdate(self):
        if self.special_end_time:
            self.__TitleUpdate()

        # self.__RangeCheck()
        self.__RouletteUpdate()

    def __TitleUpdate(self):
        if 0 < self.special_end_time:
            if self.special_end_time > app.GetGlobalTimeStamp():
                self.board.SetTitleName(uiScriptLocale.ROULETTE_SPECIAL_TITLE.format(localeInfo.SecondToColonTypeMS(
                    self.special_end_time - app.GetGlobalTimeStamp())))
        else:
            self.board.SetTitleName(uiScriptLocale.ROULETTE_TITLE)

    def __RangeCheck(self):
        (x, y, z) = player.GetMainCharacterPosition()
        if abs(x - self.start_pos_x) > self.USE_LIMIT_RANGE or abs(
                y - self.start_pos_y) > self.USE_LIMIT_RANGE:
            self.Close()

    def __RouletteUpdate(self):
        if not self.index_generator:
            logging.debug("no generator")
            return

        if not self.is_spin:
            return

        now = app.GetGlobalTime()
        self.update_gap = now - self.update_time
        self.update_time = now

        if not self.is_request:
            self.__RouletteUpdateBeforeRequest(now)
        else:
            self.__RouletteUpdateAfterRequest(now)

    def __RouletteUpdateBeforeRequest(self, cur_time):
        spinCount = len(self.spin_data)
        if spinCount > 0:
            gap = cur_time - self.move_time
            if self.__MoveNext(cur_time, self.spin_data[0]):
                if self.once == 0:
                    self.total_gap = self.total_gap + self.spin_data[0] - gap
                    self.spin_data.pop()

            if not len(self.spin_data):
                if 0 < self.total_gap:
                    if not self.once:
                        if len(self.spin_data):
                            return
                        self.request_time = app.GetGlobalTime()
                        self.__SpinFix(app.GetRandom(1, 10))
                        # appInst.instance().GetNet().SendMiniGameRouletteRequest()
                        self.is_request = True
                    else:
                        self.once = True
                        for i in xrange(self.total_gap / 60):
                            self.spin_data.append(60)
                        self.total_gap = 0

    def __RouletteUpdateAfterRequest(self, cur_time):
        if self.reward_index == -1:
            self.__MoveNext(cur_time, 60)
            if self.RESPONSE_TIME_OUT < self.request_time:
                self.is_spin = False
            else:
                return
        else:
            if len(self.spin_data) <= 0:
                self.__MoveNext(cur_time, 600)
                if self.cur_index == self.reward_index:
                    # appInst.instance().GetNet().SendMiniGameRouletteEnd()
                    self.__SpinEnd(True)
                    self.is_spin = False
                    self.spin_lock = True
                    return
                else:
                    return

            if self.__MoveNext(cur_time, self.spin_data[0]):
                self.spin_data.pop(0)

    def __MoveNext(self, cur_time, pivot_time):
        diff_time = cur_time - self.move_time
        if pivot_time - self.update_gap < diff_time:
            index = next(self.index_generator)
            (x, y) = self.edge_effect_pos_tuple[index]
            self.slot_edge_effect.SetPosition(x, y)
            self.move_time = cur_time
            self.cur_index = index
            return True

        return False

    def __Open(self, item_info):
        self.item_data_tuple = None
        if self.__SetItemList(item_info):
            self.__TitleUpdate()
            self.Show()

    def __SetItemList(self, item_data_tuple):
        itemCount = len(item_data_tuple)
        if itemCount != self.ROULETTE_SLOT_MAX:
            return False

        self.item_data_tuple = item_data_tuple
        for i in xrange(self.ROULETTE_SLOT_MAX):
            self.item_slot.SetItemSlot(i, item_data_tuple[i][0], item_data_tuple[i][1])
        return True

    def OverInSlot(self, slot_index):
        if not self.item_tooltip:
            return

        if self.item_data_tuple is None:
            return

        if len(self.item_data_tuple) != self.ROULETTE_SLOT_MAX:
            return

        if 0 < slot_index <= self.ROULETTE_SLOT_MAX:
            return

        self.item_tooltip.SetItemToolTip(self.item_data_tuple[slot_index][0])

    def OverOutSlot(self):
        if self.item_tooltip:
            self.item_tooltip.HideToolTip()

    def __StartSpin(self):
        self.once = 0
        self.is_spin = True
        self.request_time = 0
        now = app.GetGlobalTime()
        self.start_spin_time = now
        self.move_time = now
        self.update_time = now
        self.update_gap = 0
        self.total_gap = 0
        self.is_request = 0
        rand = app.GetRandom(0, 1)
        self.spin_data = []

        for v in zip(self.spin_time_before_request[rand], self.spin_count_before_request[rand]):
            it = [v[0]] * v[1]
            self.spin_data.extend(it)

    def __SpinFix(self, index):
        self.reward_index = index
        temp = 0
        if self.cur_index <= index:
            temp = self.reward_index - self.cur_index
        else:
            temp = self.ROULETTE_SLOT_MAX - self.cur_index
            temp + self.reward_index

        if 11 > temp:
            temp = temp + self.ROULETTE_SLOT_MAX

        for i in xrange(temp - 10):
            if i < temp - 10 - 3:
                self.spin_data.append(self.beginning_spin_time_after_request[0])
            else:
                self.spin_data.append(self.beginning_spin_time_after_request[1])

        t = app.GetRandom(0, 1)
        index = app.GetRandom(1, 5)
        for i in xrange(10 - index):
            self.spin_data.append(self.middle_spin_time_after_request[t][i])

        t2 = app.GetRandom(0, 1)

        self.spin_data.append(self.last_spin_time_after_request[t2])

    def __SpinEnd(self, result):
        if result:
            if self.popup_dlg:
                self.popup_dlg.Open()

        self.spin_lock = False

    def RouletteProcess(self, type, data):
        if player.ROULETTE_GC_OPEN == type:
            (end_time, items) = data
            self.special_end_time = end_time
            self.__Open(items)
        if player.ROULETTE_GC_START == type:
            self.__StartSpin()
        if player.ROULETTE_GC_REQUEST == type:
            self.__SpinFix(data)
        if player.ROULETTE_GC_END == type:
            self.__SpinEnd(data)
        if player.ROULETTE_GC_CLOSE == type:
            self.Close(True, False)
