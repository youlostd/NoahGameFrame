# -*- coding: utf-8 -*-

import logging

import app
import grpImage
import item
import player
import skill
import wndMgr
from pygame.app import appInst
from pygame.player import playerInst

import ui
import ui_event


## Mouse Controler

class CursorImage(object):
    def __init__(self, imageName):
        self.handle = None
        self.LoadImage(imageName)

    def LoadImage(self, imageName):
        self.handle = grpImage.Generate(imageName)

    def DeleteImage(self):
        if self.handle:
            del self.handle

    def IsImage(self):
        if self.handle:
            return True

        return False

    def SetPosition(self, x, y):
        if self.handle:
            grpImage.SetPosition(self.handle, x, y)

    def Render(self):
        if self.handle:
            grpImage.Render(self.handle)

class CMouseController(object):

    def __init__(self):


        self.x = 0
        self.y = 0

        self.IsSoftwareCursor = False
        self.curCursorName = ""
        self.curCursorImage = 0
        self.cursorPosX = 0
        self.cursorPosY = 0

        self.AttachedIconHandle = 0
        self.AttachedOwner = 0
        self.AttachedFlag = False
        self.AttachedType = 0
        self.AttachedSlotPosition = 0
        self.RealAttachedSlotNumber = 0
        self.AttachedCount = 1
        self.AttachedIconHalfWidth = 0
        self.AttachedIconHalfHeight = 0
        self.LastAttachedSlotNumber = 0

        self.countNumberLine = None

        self.DeattachObject()

        self.callbackDict = {}

    def Destroy(self):
        del self.countNumberLine
        self.countNumberLine = None
        self.callbackDict = {}

    def CreateNumberLine(self):
        self.countNumberLine = ui.NumberLine("GAME")
        self.countNumberLine.SetHorizontalAlignCenter()
        self.countNumberLine.Hide()

    def Create(self):
        self.IsSoftwareCursor = appInst.instance().GetSettings().IsSoftwareCursor()

        self.cursorPosDict = {
            app.NORMAL            : (0, 0),
            app.TARGET            : (0, 0),
            app.ATTACK            : (0, 0),
            app.TALK            : (0, 0),
            app.CANT_GO            : (0, 0),
            app.PICK            : (0, 0),
            app.DOOR            : (0, 0),
            app.CHAIR            : (0, 0),
            app.MAGIC            : (0, 0),
            app.BUY                : (0, 0),
            app.SELL            : (0, 0),
            app.CAMERA_ROTATE    : (0, 0),
            app.HSIZE            : (-16, -16),
            app.VSIZE            : (-16, -16),
            app.HVSIZE            : (-16, -16),
            app.FISHING: (0, 0),

        }
        self.cursorDict = {
            app.NORMAL            : CursorImage("D:/Ymir Work/UI/Cursor/cursor.sub"),
            app.ATTACK            : CursorImage("D:/Ymir Work/UI/Cursor/cursor_attack.sub"),
            app.TARGET            : CursorImage("D:/Ymir Work/UI/Cursor/cursor_attack.sub"),
            app.TALK            : CursorImage("D:/Ymir Work/UI/Cursor/cursor_talk.sub"),
            app.CANT_GO            : CursorImage("D:/Ymir Work/UI/Cursor/cursor_no.sub"),
            app.PICK            : CursorImage("D:/Ymir Work/UI/Cursor/cursor_pick.sub"),
            app.DOOR            : CursorImage("D:/Ymir Work/UI/Cursor/cursor_door.sub"),
            app.CHAIR            : CursorImage("D:/Ymir Work/UI/Cursor/cursor_chair.sub"),
            app.MAGIC            : CursorImage("D:/Ymir Work/UI/Cursor/cursor_chair.sub"),
            app.BUY                : CursorImage("D:/Ymir Work/UI/Cursor/cursor_buy.sub"),
            app.SELL            : CursorImage("D:/Ymir Work/UI/Cursor/cursor_sell.sub"),
            app.CAMERA_ROTATE    : CursorImage("D:/Ymir Work/UI/Cursor/cursor_camera_rotate.sub"),
            app.HSIZE            : CursorImage("D:/Ymir Work/UI/Cursor/cursor_hsize.sub"),
            app.VSIZE            : CursorImage("D:/Ymir Work/UI/Cursor/cursor_vsize.sub"),
            app.HVSIZE            : CursorImage("D:/Ymir Work/UI/Cursor/cursor_hvsize.sub"),
            app.FISHING            : CursorImage("D:/Ymir Work/UI/Cursor/fishing_mouse_cursor.sub"),
        }

        app.SetCursor(app.NORMAL)

        return True

    # Cursor Control
    def ChangeCursor(self, cursorNum):
        try:
            self.curCursorNum = cursorNum
            self.curCursorImage = self.cursorDict[cursorNum]
            (self.cursorPosX, self.cursorPosY) = self.cursorPosDict[cursorNum]

            if not self.curCursorImage.IsImage():
                self.curCursorNum = app.NORMAL
                self.curCursorImage = self.cursorDict[app.NORMAL]

        except KeyError:
            logging.exception("mouseModule.MouseController.SetCursor Cursor: [%d]" % cursorNum)
            self.curCursorName = app.NORMAL
            self.curCursorImage = self.cursorDict[app.NORMAL]

    # Attaching
    def AttachObject(self, Owner, Type, SlotNumber, ItemIndex, count = 0):

        self.LastAttachedSlotNumber = self.AttachedSlotPosition

        self.AttachedFlag = True
        self.AttachedOwner = Owner
        self.AttachedType = Type
        self.AttachedSlotPosition = SlotNumber
        self.AttachedItemIndex = ItemIndex
        self.AttachedCount = count
        self.countNumberLine.SetNumber("")
        self.countNumberLine.Hide()

        if count > 1:
            self.countNumberLine.SetNumber(str(count))
            self.countNumberLine.Show()

        try:

            width = 1
            height = 1

            if Type == player.SLOT_TYPE_INVENTORY or\
                Type == player.SLOT_TYPE_PRIVATE_SHOP or\
                Type == player.SLOT_TYPE_SHOP or\
                Type == player.SLOT_TYPE_SAFEBOX or\
                Type == player.SLOT_TYPE_MALL or\
                Type == player.SLOT_TYPE_DRAGON_SOUL_INVENTORY or\
                Type == player.SLOT_TYPE_MYSHOP or \
                Type == player.SLOT_TYPE_OFFLINE_SHOP or \
                Type == player.SLOT_TYPE_ACCE:

                item.SelectItem(self.AttachedItemIndex)
                self.AttachedIconHandle = item.GetIconInstance()

                if not self.AttachedIconHandle:
                    self.AttachedIconHandle = 0
                    self.DeattachObject()
                    return

                (width, height) = item.GetItemSize()

            elif Type == player.SLOT_TYPE_SKILL:
                skillGrade = playerInst().GetSkillGrade(SlotNumber)
                self.AttachedIconHandle = skill.GetIconInstanceNew(self.AttachedItemIndex, skillGrade)

            elif Type == player.SLOT_TYPE_EMOTION:
                import emotion
                icon = ""
                try:
                    icon = emotion.ICON_DICT[ItemIndex]
                except KeyError:
                    return

                self.AttachedIconHandle = grpImage.Generate(icon)
            elif Type == player.SLOT_TYPE_QUICK_SLOT:
                quickSlot = playerInst().GetGlobalQuickSlot(SlotNumber)

                if quickSlot.Type == player.SLOT_TYPE_INVENTORY:

                    itemIndex = player.GetItemIndex(quickSlot.Position)
                    item.SelectItem(itemIndex)
                    self.AttachedIconHandle = item.GetIconInstance()
                    (width, height) = item.GetItemSize()

                elif quickSlot.Type == player.SLOT_TYPE_SKILL:
                    skillIndex = playerInst().GetSkillIndex(quickSlot.Position)
                    skillGrade = playerInst().GetSkillGrade(quickSlot.Position)
                    self.AttachedIconHandle = skill.GetIconInstanceNew(skillIndex, skillGrade)

                elif quickSlot.Type == player.SLOT_TYPE_EMOTION:
                    import emotion
                    try:
                        icon = emotion.ICON_DICT[ItemIndex]
                    except KeyError:
                        return

                    self.AttachedIconHandle = grpImage.Generate(icon)

            if app.ENABLE_CHANGE_LOOK_SYSTEM:
                if Type == player.SLOT_TYPE_CHANGE_LOOK:
                    item.SelectItem(self.AttachedItemIndex)
                    self.AttachedIconHandle = item.GetIconInstance()

                    if not self.AttachedIconHandle:
                        self.AttachedIconHandle = 0
                        self.DeattachObject()
                        return

                    (width, height) = item.GetItemSize()

            if not self.AttachedIconHandle:
                self.DeattachObject()
                return

            self.AttachedIconHalfWidth = grpImage.GetWidth(self.AttachedIconHandle) / 2
            self.AttachedIconHalfHeight = grpImage.GetHeight(self.AttachedIconHandle) / 2
            self.AttachedIconHalfWidth = grpImage.GetWidth(self.AttachedIconHandle) / 2
            self.AttachedIconHalfHeight = grpImage.GetHeight(self.AttachedIconHandle) / 2

            wndMgr.SetAttachingRealSlotNumber(-1)

            if isinstance(self.AttachedSlotPosition, tuple):
                wndMgr.AttachIcon(self.AttachedType, self.AttachedItemIndex, self.AttachedSlotPosition[1], width, height)
            else:
                wndMgr.AttachIcon(self.AttachedType, self.AttachedItemIndex, self.AttachedSlotPosition, width, height)

        except Exception as e:
            logging.exception("mouseModule.py: AttachObject : " + str(e))
            self.AttachedIconHandle = 0

    def SetRealAttachedSlotNumber(self, slotNumber):
        self.RealAttachedSlotNumber = slotNumber
        wndMgr.SetAttachingRealSlotNumber(slotNumber)

    def IsAttachedMoney(self):
        return self.isAttached() and player.ITEM_MONEY == self.GetAttachedItemIndex()

    def GetAttachedMoneyAmount(self):
        if self.isAttached() and player.ITEM_MONEY == self.GetAttachedItemIndex():
            return self.GetAttachedItemCount()

        return 0

    def AttachMoney(self, owner, type, count):
        self.LastAttachedSlotNumber = self.AttachedSlotPosition

        self.AttachedFlag = True
        self.AttachedOwner = owner
        self.AttachedType = type
        self.AttachedSlotPosition = -1
        self.AttachedItemIndex = player.ITEM_MONEY
        self.AttachedCount = count
        self.AttachedIconHandle = grpImage.Generate("icon/item/money.tga")
        self.AttachedIconHalfWidth = grpImage.GetWidth(self.AttachedIconHandle) / 2
        self.AttachedIconHalfHeight = grpImage.GetHeight(self.AttachedIconHandle) / 2
        wndMgr.AttachIcon(self.AttachedType, self.AttachedItemIndex, self.AttachedSlotPosition, 1, 1)

        if count > 1:
            self.countNumberLine.SetNumber(str(count))
            self.countNumberLine.Show()

    def DeattachObject(self):
        self.ClearCallBack()
        self.LastAttachedSlotNumber = self.AttachedSlotPosition

        if self.AttachedIconHandle is not None:
            del self.AttachedIconHandle
            self.AttachedIconHandle = 0

        self.AttachedFlag = False
        self.AttachedType = -1
        self.AttachedItemIndex = -1
        self.AttachedSlotPosition = -1
        self.RealAttachedSlotNumber = -1
        self.AttachedIconHandle = 0
        wndMgr.SetAttachingFlag(False)

        if self.countNumberLine:
            self.countNumberLine.Hide()

    def isAttached(self):
        return self.AttachedFlag

    def GetAttachedOwner(self):
        if not self.isAttached():
            return 0

        return self.AttachedOwner

    def GetAttachedType(self):
        if not self.isAttached():
            return player.SLOT_TYPE_NONE

        return self.AttachedType

    def GetAttachedSlotPosition(self):
        if not self.isAttached():
            return 0

        return self.AttachedSlotPosition

    def GetRealAttachedSlotNumber(self):
        if not self.isAttached():
            return 0

        # Fallback
        if isinstance(self.AttachedSlotPosition, int):
            if self.RealAttachedSlotNumber <= 0 <= self.AttachedSlotPosition:
                return self.AttachedSlotPosition
        else:
            if self.RealAttachedSlotNumber <= 0 <= self.AttachedSlotPosition[1]:
                return self.AttachedSlotPosition[1]

        return self.RealAttachedSlotNumber

    def GetLastAttachedSlotNumber(self):
        return self.LastAttachedSlotNumber

    def GetAttachedItemIndex(self):
        if not self.isAttached():
            return 0

        return self.AttachedItemIndex

    def GetAttachedItemCount(self):
        if not self.isAttached():
            return 0

        return self.AttachedCount

    # Update
    def Update(self, x, y):
        self.x = x
        self.y = y

        if self.isAttached():
            if 0 != self.AttachedIconHandle:
                grpImage.SetDiffuseColor(self.AttachedIconHandle, 1.0, 1.0, 1.0, 0.5)
                grpImage.SetPosition(self.AttachedIconHandle, self.x - self.AttachedIconHalfWidth,
                                     self.y - self.AttachedIconHalfHeight)
                self.countNumberLine.SetPosition(self.x, self.y - self.AttachedIconHalfHeight - 3)

        if self.IsSoftwareCursor:
            if 0 != self.curCursorImage:
                self.curCursorImage.SetPosition(self.x + self.cursorPosX, self.y + self.cursorPosY)

    # Render
    def Render(self):
        if self.isAttached():
            if 0 != self.AttachedIconHandle:
                grpImage.Render(self.AttachedIconHandle)

        if self.IsSoftwareCursor:
            if app.IsShowCursor():
                if 0 != self.curCursorImage:
                    self.curCursorImage.Render()
        else:
            if not app.IsShowCursor():
                if app.IsLiarCursorOn():
                    if 0 != self.curCursorImage:
                        self.curCursorImage.SetPosition(self.x + self.cursorPosX, self.y + self.cursorPosY)
                        self.curCursorImage.Render()

    def SetCallBack(self, type, event=None):
        self.callbackDict[type] = ui_event.MakeEvent(event)

    def RunCallBack(self, type, *args):
        if type not in self.callbackDict:
            self.DeattachObject()
            return

        fn = self.callbackDict[type]
        if not fn:
            self.DeattachObject()
            return

        if len(args) > 0:
            passArgs = ()
            for arg in args:
                passArgs = passArgs + (arg,)

            fn(*passArgs)
        else:
            fn()


    def ClearCallBack(self):
        self.callbackDict = {}

mouseController = CMouseController()
