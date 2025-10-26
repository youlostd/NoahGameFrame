# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import app
import grp
import guild
import messenger
import player
from pygame.app import appInst

import colorInfo
import constInfo
import localeInfo
import ui
import uiCommon
import uiToolTip
from ui_event import MakeEvent, Event

FRIEND = 0
GUILD = 1
TEAM = 2


class MessengerItem(ui.Window):
    def __init__(self, getParentEvent):
        ui.Window.__init__(self)

        self.SetParent(getParentEvent().groupContainer)
        self.AddFlag("float")

        self.name = ""

        self.image = ui.ImageBox()
        self.image.AddFlag("not_pick")
        self.image.SetParent(self)
        self.image.Show()

        self.text = ui.TextLine()
        self.text.SetParent(self)
        self.text.SetPosition(20, 2)
        self.text.SetFontName("Verdana:11")
        self.text.Show()

        if isinstance(self, MessengerGroupItem):
            self.image.SetPosition(15, 0)
            self.image.LoadImage(self.IMAGE_FILE_NAME["CLOSE"])
            self.SetSize(self.image.GetWidth(), self.image.GetHeight() - 20)
            self.text.SetPosition(35, 0)

        if isinstance(self, MessengerMemberItem):
            self.image.SetPosition(15, 3)
            self.text.SetPosition(35, 0)

        self.whisperButton = None

        if isinstance(self, MessengerMemberItem):
            self.whisperButton = ui.Button()
            self.whisperButton.SetParent(self)
            self.whisperButton.SetPosition(0, 3)
            self.whisperButton.SetHorizontalAlignRight()
            self.whisperButton.Show()

        self.lovePoint = -1
        self.lovePointToolTip = None

        self.isSelected = False
        self.SetSize(157, 15)

        self.getParentEvent = MakeEvent(getParentEvent)

    def SetName(self, name):
        self.name = name
        if name:
            import emoji_config

            if len(self.name) > 16:
                name = name[:16] + "..."

            name = emoji_config.ReplaceEmoticons(name)
            self.text.SetText(name)
            if isinstance(self, MessengerGroupItem):
                self.memberCountText.SetPosition(
                    self.text.GetLocalPosition()[0] + self.text.GetWidth() + 3, 0
                )

    def SetLovePoint(self, lovePoint):
        self.lovePoint = lovePoint

    def Select(self):
        self.isSelected = True

    def UnSelect(self):
        self.isSelected = False

    def GetName(self):
        return self.name

    def GetStepWidth(self):
        return 0

    # Whisper
    def CanWhisper(self):
        return False

    def IsOnline(self):
        return False

    def OnWhisper(self):
        pass

    # Remove
    def CanRemove(self):
        return False

    def OnRemove(self):
        return False

    # Warp
    def CanWarp(self):
        return False

    def OnWarp(self):
        pass

    def OnMouseOverIn(self):
        if -1 != self.lovePoint:
            if not self.lovePointToolTip:
                self.lovePointToolTip = uiToolTip.ToolTip(100)
                self.lovePointToolTip.SetTitle(self.name)
                self.lovePointToolTip.AppendTextLine(
                    localeInfo.AFFECT_LOVE_POINT.format(self.lovePoint)
                )
                self.lovePointToolTip.ResizeToolTip()
            self.lovePointToolTip.ShowToolTip()
            return True
        return False

    def OnMouseOverOut(self):
        if self.lovePointToolTip:
            self.lovePointToolTip.HideToolTip()

    def OnMouseLeftButtonDown(self):
        self.getParentEvent().OnSelectItem(self)

    def OnMouseLeftButtonDoubleClick(self):
        self.getParentEvent().OnDoubleClickItem()

    def OnRender(self):
        if self.isSelected:
            x, y = self.GetGlobalPosition()
            grp.SetColor(colorInfo.UI_COLOR_SELECTED)
            grp.RenderBar(x + 16, y, self.GetWidth() - 16, self.GetHeight())


class MessengerMemberItem(MessengerItem):
    STATE_OFFLINE = 0
    STATE_ONLINE = 1

    IMAGE_FILE_NAME = {
        "ONLINE": "d:/ymir work/ui/game/windows/messenger_list_online.sub",
        "OFFLINE": "d:/ymir work/ui/game/windows/messenger_list_offline.sub",
    }

    def __init__(self, getParentEvent):
        MessengerItem.__init__(self, getParentEvent)
        self.key = None
        self.state = self.STATE_OFFLINE

        self.whisperButton.SetEvent(self.OnWhisper)
        self.Offline()

    def GetStepWidth(self):
        return 0
        # 15

    def SetKey(self, key):
        self.key = key

    def IsSameKey(self, key):
        return self.key == key

    def IsOnline(self):
        if self.STATE_ONLINE == self.state:
            return True

        return False

    def Online(self):
        self.image.LoadImage(self.IMAGE_FILE_NAME["ONLINE"])
        self.state = self.STATE_ONLINE

    def Offline(self):
        self.image.LoadImage(self.IMAGE_FILE_NAME["OFFLINE"])
        self.state = self.STATE_OFFLINE

    def CanWhisper(self):
        if self.IsOnline():
            return True

        return False

    def OnWhisper(self):
        self.getParentEvent().whisperButtonEvent(self.GetName())
        return True

    def Select(self):
        MessengerItem.Select(self)


class MessengerGroupItem(MessengerItem):
    IMAGE_FILE_NAME = {
        "OPEN": "d:/ymir work/ui/game/windows/messenger_list_open.sub",
        "CLOSE": "d:/ymir work/ui/game/windows/messenger_list_close.sub",
    }

    def __init__(self, getParentEvent):
        self.isOpen = False
        self.memberList = []

        MessengerItem.__init__(self, getParentEvent)
        self.memberCountText = ui.TextLine()
        self.memberCountText.SetFontName("Verdana:11")
        self.memberCountText.SetParent(self)
        self.memberCountText.SetHorizontalAlignLeft()
        self.memberCountText.SetText("({}/{})".format(0, len(self.memberList)))
        self.memberCountText.Show()
        self.memberCountText.SetPosition(
            self.text.GetLocalPosition()[0] + self.text.GetWidth() + 3, 0
        )

    def AppendMember(self, member, key, name):
        member.SetKey(key)
        member.SetName(name)
        self.memberList.append(member)
        self.memberCountText.SetText(
            "({}/{})".format(len(self.GetLoginMemberList()), len(self.memberList))
        )
        return member

    def RemoveMember(self, item):
        for i in xrange(len(self.memberList)):
            if item == self.memberList[i]:
                del self.memberList[i]
                return

        self.memberCountText.SetText(
            "({}/{})".format(len(self.GetLoginMemberList()), len(self.memberList))
        )

    def ClearMember(self):
        self.memberList = []

    def FindMember(self, key):
        list = filter(
            lambda argMember, argKey=key: argMember.IsSameKey(argKey), self.memberList
        )
        if list:
            return list[0]

        return None

    def GetLoginMemberList(self):
        return filter(MessengerMemberItem.IsOnline, self.memberList)

    def GetLogoutMemberList(self):
        return filter(lambda arg: not arg.IsOnline(), self.memberList)

    def IsOpen(self):
        return self.isOpen

    def UpdateMemberCount(self):
        self.memberCountText.SetText(
            "({}/{})".format(len(self.GetLoginMemberList()), len(self.memberList))
        )

    def Open(self):
        self.image.LoadImage(self.IMAGE_FILE_NAME["OPEN"])
        self.memberCountText.SetText(
            "({}/{})".format(len(self.GetLoginMemberList()), len(self.memberList))
        )
        self.isOpen = True

    def Close(self):
        self.image.LoadImage(self.IMAGE_FILE_NAME["CLOSE"])
        self.memberCountText.SetText(
            "({}/{})".format(len(self.GetLoginMemberList()), len(self.memberList))
        )
        self.isOpen = False

        map(ui.Window.Hide, self.memberList)

    def Select(self):

        if self.IsOpen():
            self.Close()
        else:
            self.Open()

        MessengerItem.Select(self)
        self.getParentEvent().OnRefreshList()


class MessengerFriendItem(MessengerMemberItem):
    def __init__(self, getParentEvent):
        MessengerMemberItem.__init__(self, getParentEvent)

    def CanRemove(self):
        return True

    def OnRemove(self):
        messenger.RemoveFriend(self.key)
        appInst.instance().GetNet().SendMessengerRemovePacket(self.key, self.name)
        return True


class MessengerTeamItem(MessengerMemberItem):
    def __init__(self, getParentEvent):
        MessengerMemberItem.__init__(self, getParentEvent)
        self.lang = None

    def SetName(self, name):
        self.name = name

        if name and self.lang:
            emojiString = ""
            languages = self.lang.split(" | ")
            for lang in languages:
                emojiString = emojiString + "({})".format(lang)
            import emoji_config

            self.text.SetText(
                "{} {}".format(name, emoji_config.ReplaceEmoticons(emojiString))
            )
        elif name:
            self.text.SetText(name)

    def SetLang(self, lang):
        self.lang = lang

    def CanRemove(self):
        return False


class MessengerGuildItem(MessengerMemberItem):
    def __init__(self, getParentEvent):
        MessengerMemberItem.__init__(self, getParentEvent)

    def CanWarp(self):
        if not self.IsOnline():
            return False
        return True

    def OnWarp(self):
        appInst.instance().GetNet().SendGuildUseSkillPacket(155, self.key)

    def CanRemove(self):
        for i in xrange(guild.ENEMY_GUILD_SLOT_MAX_COUNT):
            if guild.GetEnemyGuildName(i) != "":
                return False

        if guild.MainPlayerHasAuthority(long(guild.AUTH_REMOVE_MEMBER)):
            if guild.IsMemberByName(self.name):
                return True

        return False

    def OnRemove(self):
        appInst.instance().GetNet().SendGuildRemoveMemberPacket(self.key)
        return True


class MessengerFriendGroup(MessengerGroupItem):
    def __init__(self, getParentEvent):
        MessengerGroupItem.__init__(self, getParentEvent)
        self.SetName(localeInfo.MESSENGER_FRIEND)

    def AppendMember(self, key, name):
        item = MessengerFriendItem(self.getParentEvent)
        return MessengerGroupItem.AppendMember(self, item, key, name)


class MessengerTeamGroup(MessengerGroupItem):
    def __init__(self, getParentEvent):
        MessengerGroupItem.__init__(self, getParentEvent)
        self.SetName("Oyun Takımı")

    def AppendMember(self, key, name):
        item = MessengerTeamItem(self.getParentEvent)
        return MessengerGroupItem.AppendMember(self, item, key, name)


class MessengerGuildGroup(MessengerGroupItem):
    def __init__(self, getParentEvent):
        MessengerGroupItem.__init__(self, getParentEvent)
        self.SetName(localeInfo.MESSENGER_GUILD)
        self.AddFlag("float")

    def AppendMember(self, key, name):
        item = MessengerGuildItem(self.getParentEvent)
        return MessengerGroupItem.AppendMember(self, item, key, name)


class MessengerFamilyGroup(MessengerGroupItem):
    def __init__(self, getParentEvent):
        MessengerGroupItem.__init__(self, getParentEvent)
        self.SetName(localeInfo.MESSENGER_FAMILY)
        self.AddFlag("float")

        self.lover = None

    def AppendMember(self, key, name):
        item = MessengerFriendItem(self.getParentEvent)
        self.lover = item
        return MessengerGroupItem.AppendMember(self, item, key, name)

    def GetLover(self):
        return self.lover

    def ClearMember(self):
        self.lover = None
        MessengerGroupItem.ClearMember(self)


class MessengerBlockGroup(MessengerGroupItem):
    def __init__(self, getParentEvent):
        MessengerGroupItem.__init__(self, getParentEvent)
        self.SetName(localeInfo.MESSENGER_BLOCK)

    def AppendMember(self, name, mode):
        for member in self.memberList:
            if member.GetName() == name:
                member.SetMode(mode)
                return

        item = MessengerBlockItem(self.getParentEvent)
        item.SetName(name)
        item.SetMode(mode)
        self.memberList.append(item)

    def RemoveMember(self, name):
        for i in xrange(len(self.memberList)):
            member = self.memberList[i]
            if member.GetName() == name:
                del self.memberList[i]
                return


class MessengerBlockItem(MessengerItem):
    def __init__(self, getParentEvent):
        MessengerItem.__init__(self, getParentEvent)
        self.mode = 0
        self.dialog = None

    def CanRemove(self):
        return True

    def SetMode(self, mode):
        self.mode = mode

    def OnRemove(self):
        messenger.RemoveBlock(self.name)
        appInst.instance().GetNet().SendMessengerSetBlockPacket(self.name, 0)
        return True

    def OnMouseLeftButtonDoubleClick(self):
        self.dialog = BlockModeDialog(self.OnAccept, self.OnCancel)
        self.dialog.SetTitle(
            "{0} - {1}".format(localeInfo.MESSENGER_BLOCK, self.GetName())
        )
        self.dialog.SetMode(self.mode)
        self.dialog.Open()

    def OnAccept(self):
        appInst.instance().GetNet().SendMessengerSetBlockPacket(
            self.GetName(), self.dialog.mode
        )
        self.dialog.Hide()
        self.dialog = None

    def OnCancel(self):
        self.dialog.Hide()
        self.dialog = None


class BlockModeDialog(ui.ScriptWindow):
    def __init__(self, onAccept, onCancel):
        ui.ScriptWindow.__init__(self)
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/BlockModeDialog.py")

        self.board = self.GetChild("Board")

        self.acceptButton = self.GetChild("AcceptButton")
        self.acceptButton.SetEvent(onAccept)

        self.cancelButton = self.GetChild("CancelButton")
        self.cancelButton.SetEvent(onCancel)

        self.block_exchange = self.GetChild("block_exchange_button")
        self.block_exchange.SetToggleDownEvent(Event(self.Set, player.BLOCK_EXCHANGE))
        self.block_exchange.SetToggleUpEvent(Event(self.Unset, player.BLOCK_EXCHANGE))
        self.block_party = self.GetChild("block_party_button")
        self.block_party.SetToggleDownEvent(Event(self.Set, player.BLOCK_PARTY))
        self.block_party.SetToggleUpEvent(Event(self.Unset, player.BLOCK_PARTY))
        self.block_guild = self.GetChild("block_guild_button")
        self.block_guild.SetToggleDownEvent(Event(self.Set, player.BLOCK_GUILD))
        self.block_guild.SetToggleUpEvent(Event(self.Unset, player.BLOCK_GUILD))
        self.block_whisper = self.GetChild("block_whisper_button")
        self.block_whisper.SetToggleDownEvent(Event(self.Set, player.BLOCK_WHISPER))
        self.block_whisper.SetToggleUpEvent(Event(self.Unset, player.BLOCK_WHISPER))
        self.block_friend = self.GetChild("block_friend_button")
        self.block_friend.SetToggleDownEvent(Event(self.Set, player.BLOCK_FRIEND))
        self.block_friend.SetToggleUpEvent(Event(self.Unset, player.BLOCK_FRIEND))
        self.block_party_request = self.GetChild("block_party_request_button")
        self.block_party_request.SetToggleDownEvent(
            Event(self.Set, player.BLOCK_PARTY_REQUEST)
        )
        self.block_party_request.SetToggleUpEvent(
            Event(self.Unset, player.BLOCK_PARTY_REQUEST)
        )
        self.block_duel_request = self.GetChild("block_duel_request_button")
        self.block_duel_request.SetToggleDownEvent(
            Event(self.Set, player.BLOCK_DUEL_REQUEST)
        )
        self.block_duel_request.SetToggleUpEvent(
            Event(self.Unset, player.BLOCK_DUEL_REQUEST)
        )

        self.mode = 0
        self.name = ""

    def Open(self):
        self.SetCenterPosition()
        self.SetTop()
        self.Show()

    def Set(self, mode):
        self.mode |= mode

    def Unset(self, mode):
        self.mode &= ~mode

    def SetMode(self, mode):
        self.mode = mode
        if mode & player.BLOCK_EXCHANGE:
            self.block_exchange.Down()
        else:
            self.block_exchange.SetUp()

        if mode & player.BLOCK_PARTY:
            self.block_party.Down()
        else:
            self.block_party.SetUp()

        if mode & player.BLOCK_GUILD:
            self.block_guild.Down()
        else:
            self.block_guild.SetUp()

        if mode & player.BLOCK_WHISPER:
            self.block_whisper.Down()
        else:
            self.block_whisper.SetUp()

        if mode & player.BLOCK_FRIEND:
            self.block_friend.Down()
        else:
            self.block_friend.SetUp()

        if mode & player.BLOCK_PARTY_REQUEST:
            self.block_party_request.Down()
        else:
            self.block_party_request.SetUp()

        if mode & player.BLOCK_DUEL_REQUEST:
            self.block_duel_request.Down()
        else:
            self.block_duel_request.SetUp()

    def SetTitle(self, name):
        self.board.SetTitleName(name)


class AddDialog(uiCommon.InputDialog):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/MessengerAdd.py")

        self.board = self.GetChild("Board")
        self.acceptButton = self.GetChild("AcceptButton")
        self.cancelButton = self.GetChild("CancelButton")
        self.friendButton = self.GetChild("friend")
        self.blockButton = self.GetChild("block")
        self.type = 0
        self.typeBtns = ui.RadioButtonGroup.Create(
            [
                [self.friendButton, Event(self.SetType, 0), None],
                [self.blockButton, Event(self.SetType, 1), None],
            ]
        )
        self.typeBtns.OnClick(0)
        self.typeBtns.Show()
        self.inputSlot = self.GetChild("InputSlot")
        self.inputValue = self.GetChild("InputValue")

    def SetType(self, t):
        self.type = t


###################################################################################################
###################################################################################################
###################################################################################################


class MessengerWindow(ui.ScriptWindow):
    START_POSITION = 3

    def __init__(self, interface):
        ui.ScriptWindow.__init__(self)
        messenger.SetMessengerHandler(self)
        self.AddFlag("animated_board")
        from _weakref import proxy

        self.interface = proxy(interface)
        self.board = None
        self.groupList = []
        self.showingItemList = []
        self.selectedItem = None
        self.whisperButtonEvent = None
        self.familyGroup = None
        self.teamGroup = None

        self.guildButtonEvent = None

        self.friendNameBoard = None
        self.questionDialog = None
        self.popupDialog = None
        self.inputDialog = None
        self.familyGroup = None
        self.teamGroup = None

        self.removeButton = None
        self.blockNameBoard = None
        self.scrollWindow = None
        self.groupContainer = None
        self.isLoaded = 0
        self.__LoadWindow()

        self.__AddGroup()
        messenger.RefreshGuildMember()

    def Show(self):
        if self.isLoaded == 0:
            self.isLoaded = 1

            self.OnRefreshList()

        ui.ScriptWindow.Show(self)

    def __LoadWindow(self):

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/MessengerWindow.py")

        self.board = self.GetChild("board")
        self.whisperButton = self.GetChild("WhisperButton")
        self.removeButton = self.GetChild("RemoveButton")
        self.addFriendButton = self.GetChild("AddFriendButton")
        self.guildButton = self.GetChild("GuildButton")

        self.board.SetCloseEvent(self.Close)
        self.removeButton.SetEvent(self.OnPressRemoveButton)

        self.whisperButton.SetEvent(self.OnPressWhisperButton)
        self.removeButton.SetEvent(self.OnPressRemoveButton)
        self.addFriendButton.SetEvent(self.OnPressAddFriendButton)
        self.guildButton.SetEvent(self.OnPressGuildButton)

        self.groupContainer = ui.Window()
        self.groupContainer.SetSize(184, 999)
        self.groupContainer.AddFlag("not_pick")
        self.groupContainer.Show()

        self.scrollWindow = ui.ScrollWindow()
        self.scrollWindow.SetSize(151, 215)
        self.scrollWindow.SetContentWindow(self.groupContainer)
        self.scrollWindow.AddFlag("not_pick")
        self.scrollWindow.SetParent(self)
        self.scrollWindow.SetPosition(7, 40)
        self.scrollWindow.Show()
        self.scrollWindow.FitWidth(151)

        self.removeButton.Disable()

        for group in self.groupList:
            group.SetTop()

    def Destroy(self):
        self.board = None
        self.friendNameBoard = None
        self.questionDialog = None
        self.popupDialog = None
        self.inputDialog = None
        self.familyGroup = None
        self.teamGroup = None
        self.blockGroup = None
        self.removeButton = None
        messenger.SetMessengerHandler(None)

    def OnCloseQuestionDialog(self):
        self.questionDialog.Close()
        self.questionDialog = None
        return True

    def Close(self):
        self.questionDialog = None
        self.Hide()

    def SetSize(self, width, height):
        ui.ScriptWindow.SetSize(self, width, height)
        if self.board:
            self.board.SetSize(width, height)

    def __LocateMember(self):

        if self.isLoaded == 0:
            return

        yPos = self.START_POSITION

        map(ui.Window.Hide, self.showingItemList)

        for item in self.showingItemList:
            item.SetPosition(0 + item.GetStepWidth(), yPos)
            item.SetTop()
            item.Show()

            # if isinstance(item, MessengerGroupItem):
            #     yPos += 10

            yPos += 20
            self.groupContainer.SetSize(self.groupContainer.GetWidth(), yPos)

    def __AddGroup(self):
        member = MessengerFriendGroup(self.GetSelf)
        member.Open()
        member.Show()
        self.groupList.append(member)

        member = MessengerGuildGroup(self.GetSelf)
        member.Open()
        member.Show()
        self.groupList.append(member)

        block = MessengerBlockGroup(self.GetSelf)
        block.Open()
        block.Show()
        self.blockGroup = block

        member = MessengerTeamGroup(self.GetSelf)
        member.Open()
        member.Show()
        self.teamGroup = member
        self.groupList.append(member)

    def __AddFamilyGroup(self):
        member = MessengerFamilyGroup(self.GetSelf)
        member.Open()
        member.Show()

        self.familyGroup = member

    def ClearGuildMember(self):
        self.groupList[GUILD].ClearMember()

    def SetWhisperButtonEvent(self, event):
        self.whisperButtonEvent = MakeEvent(event)

    def SetGuildButtonEvent(self, event):
        self.guildButtonEvent = MakeEvent(event)

    def OnCloseInputDialog(self):
        self.inputDialog.Close()
        self.inputDialog = None
        return True

    def OnPressGuildButton(self):
        self.guildButtonEvent()

    def OnPressAddFriendButton(self):
        nameBoard = AddDialog()
        nameBoard.SetTitle(localeInfo.MESSENGER_ADD_FRIEND)
        nameBoard.SetAcceptEvent(self.OnAdd)
        nameBoard.SetCancelEvent(self.OnCancelAdd)
        nameBoard.Open()
        self.nameBoard = nameBoard

    def OnAdd(self):
        text = self.nameBoard.GetText()
        if text:
            if self.nameBoard.type == 0:
                appInst.instance().GetNet().SendMessengerAddByNamePacket(text)
            else:
                self.blockDialog = BlockModeDialog(
                    self.OnBlockAccept, self.OnCancelBlock
                )
                self.blockDialog.name = text
                self.blockDialog.SetTitle(
                    "{0} - {1}".format(localeInfo.MESSENGER_BLOCK, text)
                )
                self.blockDialog.Open()
        self.nameBoard.Close()
        self.nameBoard = None
        return True

    def OnCancelAdd(self):
        self.nameBoard.Close()
        self.nameBoard = None
        return True

    def OnBlockAccept(self):
        appInst.instance().GetNet().SendMessengerSetBlockPacket(
            self.blockDialog.name, self.blockDialog.mode
        )
        self.blockDialog.Hide()
        self.blockDialog = None

    def OnCancelBlock(self):
        self.blockDialog.Hide()
        self.blockDialog = None
        return True

    def OnPressWhisperButton(self):
        if self.selectedItem:
            self.selectedItem.OnWhisper()

    def OnPressRemoveButton(self):
        if self.selectedItem:
            if self.selectedItem.CanRemove():
                self.questionDialog = uiCommon.QuestionDialog()
                self.questionDialog.SetText(localeInfo.MESSENGER_DO_YOU_DELETE )
                self.questionDialog.SetAcceptEvent(self.OnRemove)
                self.questionDialog.SetCancelEvent(self.OnCloseQuestionDialog)
                self.questionDialog.Open()

    def OnRemove(self):
        if self.selectedItem:
            if self.selectedItem.CanRemove():
                map(
                    lambda arg, argDeletingItem=self.selectedItem: arg.RemoveMember(
                        argDeletingItem
                    ),
                    self.groupList,
                )
                self.selectedItem.OnRemove()
                self.selectedItem.UnSelect()
                self.selectedItem = None
                self.OnRefreshList()

        self.OnCloseQuestionDialog()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Hide()
            return True

    # CallBack
    def OnSelectItem(self, item):

        if self.selectedItem:
            if item != self.selectedItem:
                self.selectedItem.UnSelect()

        self.selectedItem = item

        if self.selectedItem:
            self.selectedItem.Select()

            if self.selectedItem.CanRemove():
                self.removeButton.Enable()
            else:
                self.removeButton.Disable()

    def OnDoubleClickItem(self):
        if not self.selectedItem:
            return

        self.OnPressWhisperButton()

    def GetSelf(self):
        return self

    def OnRefreshList(self):
        self.showingItemList = []

        if self.familyGroup:
            self.familyGroup.UpdateMemberCount()
            self.showingItemList.append(self.familyGroup)
            if self.familyGroup.IsOpen() and self.familyGroup.GetLover():
                self.showingItemList.append(self.familyGroup.GetLover())

        for group in self.groupList:

            group.UpdateMemberCount()

            self.showingItemList.append(group)

            if group.IsOpen():

                loginMemberList = group.GetLoginMemberList()
                logoutMemberList = group.GetLogoutMemberList()

                if loginMemberList or logoutMemberList:
                    for member in loginMemberList:
                        self.showingItemList.append(member)
                    for member in logoutMemberList:
                        self.showingItemList.append(member)

                else:
                    item = MessengerItem(self.GetSelf)
                    item.SetName(localeInfo.MESSENGER_EMPTY_LIST)
                    self.showingItemList.append(item)

        self.showingItemList.append(self.blockGroup)
        if self.blockGroup.IsOpen():
            for member in self.blockGroup.memberList:
                self.showingItemList.append(member)

        self.__LocateMember()

    def RefreshMessenger(self):
        self.OnRefreshList()

    # EventHandler
    def __AddList(self, groupIndex, key, name):
        group = self.groupList[groupIndex]
        member = group.FindMember(key)
        if not member:
            member = group.AppendMember(key, name)
            self.OnSelectItem(None)
        return member

    def OnRemoveList(self, groupIndex, key):
        group = self.groupList[groupIndex]
        group.RemoveMember(group.FindMember(key))
        self.OnRefreshList()

    def OnRemoveAllList(self, groupIndex):
        group = self.groupList[groupIndex]
        group.ClearMember()
        self.OnRefreshList()

    def OnTeamLogin(self, groupIndex, key, lang):
        member = self.__AddList(groupIndex, key, key)
        # member.SetLang(lang) # Language Set
        member.SetName(key)
        member.Online()
        self.OnRefreshList()
        if appInst.instance().GetSettings().IsShowGMNotifications():
            self.NotifyOnline(groupIndex, key)

    def OnTeamLogout(self, groupIndex, key, lang):
        member = self.__AddList(groupIndex, key, key)
        # member.SetLang(lang)
        member.SetName(key)
        member.Offline()
        constInfo.REAL_LOGOUT_LIST.append((key, app.GetGlobalTimeStamp()))

        self.OnRefreshList()

    def OnLogin(self, groupIndex, key, name=None):
        if not name:
            name = key
        member = self.__AddList(groupIndex, key, name)
        member.SetName(name)
        member.Online()
        self.OnRefreshList()

        if (
            groupIndex == 0
            and not appInst.instance().GetSettings().IsShowGMNotifications()
        ):
            return

        if (
            groupIndex == 1
            and not appInst.instance().GetSettings().IsShowGuildNotifications()
        ):
            return

        self.NotifyOnline(groupIndex, name)

    def NotifyOnline(self, groupIndex, name):
        if name not in constInfo.ALREADY_NOTIFY_LIST[groupIndex]:
            self.onlinePopup = uiCommon.OnlinePopup()
            self.onlinePopup.SetUserType(groupIndex)
            self.onlinePopup.SetUserName(name)
            self.onlinePopup.SetEvent(Event(self.OpenWhisper, name))
            self.onlinePopup.SlideIn()

            constInfo.ALREADY_NOTIFY_LIST[groupIndex].append(name)

    def OpenWhisper(self, userName):
        self.whisperButtonEvent(userName)

    def OnLogout(self, groupIndex, key, name=None):
        member = self.__AddList(groupIndex, key, name)
        if not name:
            name = key
        member.SetName(name)
        member.Offline()
        constInfo.REAL_LOGOUT_LIST.append((name, app.GetGlobalTimeStamp()))

        self.OnRefreshList()

    def OnAddTeam(self, name):
        member = self.teamGroup.AppendMember(0, name)

        member.SetName(name)
        member.Offline()
        self.OnRefreshList()

    def OnAddLover(self, name, lovePoint):
        if not self.familyGroup:
            self.__AddFamilyGroup()

        member = self.familyGroup.AppendMember(0, name)

        member.SetName(name)
        member.SetLovePoint(lovePoint)
        member.Offline()
        self.OnRefreshList()

    def OnUpdateLovePoint(self, lovePoint):
        if not self.familyGroup:
            return

        lover = self.familyGroup.GetLover()
        if not lover:
            return

        lover.SetLovePoint(lovePoint)

    def OnLoginLover(self):
        if not self.familyGroup:
            return

        lover = self.familyGroup.GetLover()
        if not lover:
            return

        lover.Online()

    def OnLogoutLover(self):
        if not self.familyGroup:
            return

        lover = self.familyGroup.GetLover()
        if not lover:
            return

        lover.Offline()

    def ClearLoverInfo(self):
        if not self.familyGroup:
            return

        self.familyGroup.ClearMember()
        self.familyGroup = None
        self.OnRefreshList()

    def OnAppendBlock(self, name, mode):
        self.blockGroup.AppendMember(name, mode)
        self.OnRefreshList()

    def OnRemoveBlock(self, name):
        self.blockGroup.RemoveMember(name)
        self.OnRefreshList()
