import grp
import player

import ui
import localeInfo
BOARD_WIDTH = 650
PLAYER_LIMIT = 10
import app

class Component:
    def __init__(self):
        pass

    @staticmethod
    def TextLine(parent, textLineText, x, y, color):
        textLine = ui.TextLine()
        if parent is not None:
            textLine.SetParent(parent)
        textLine.SetPosition(x, y)
        if color is not None:
            textLine.SetFontColor(color[0], color[1], color[2])
        textLine.SetText(textLineText)
        textLine.Show()
        return textLine

    @staticmethod
    def RGB(r, g, b):
        return r * 255, g * 255, b * 255

    @staticmethod
    def ExpandedImage(parent, x, y, img):
        image = ui.ExpandedImageBox()
        if parent is not None:
            image.SetParent(parent)
        image.SetPosition(x, y)
        image.LoadImage(img)
        image.Show()
        return image


class GuildWarPlayer(ui.Window):
    NEGATIVE_COLOR = grp.GenerateColor(0.9, 0.4745, 0.4627, 1.0)
    POSITIVE_COLOR = grp.GenerateColor(0.5411, 0.7254, 0.5568, 1.0)
    TITLE_COLOR = grp.GenerateColor(0.9490, 0.9058, 0.7568, 1.0)
    SPECIAL_POSITIVE_COLOR = grp.GenerateColor(0.8824, 0.9804, 0.8824, 1.0)

    def __init__(self):
        ui.Window.__init__(self)

        self.name = ""
        self.kills = 0
        self.deaths = 0
        self.balance = 0
        self.damage = "0"
        self.level = 0

        self.__BuildWindow()

    def __TextLine(self, text, x, color=None, centered=False):
        element = Component().TextLine(self, "", x, 0, None)
        element.SetFontName("Verdana:12")
        element.SetText(text)

        if centered:
            element.SetHorizontalAlignCenter()

        if color:
            element.SetPackedFontColor(color)

        return element

    def __BuildWindow(self):
        c = Component()
        self.component = c

        self.nameLine = self.__TextLine(self.GetName(), 5)
        self.killsLine = self.__TextLine(self.GetKills(), 208, self.POSITIVE_COLOR, False)
        self.deathsLine = self.__TextLine(self.GetDeaths(), 288, self.NEGATIVE_COLOR, False)
        self.levelLine = self.__TextLine(self.GetLevel(), 433, None, False)
        self.damageLine = self.__TextLine(self.GetDamage(), 514, None, False)

        self.Show()

    def SetName(self, name):
        self.name = str(name)

    def GetName(self):
        return self.name

    def SetKills(self, kills):
        self.kills = int(kills)
        self.CalculateBalance()

    def GetKills(self):
        return self.kills

    def SetDeaths(self, deaths):
        self.deaths = int(deaths)
        self.CalculateBalance()

    def GetDeaths(self):
        return self.deaths

    def CalculateBalance(self):
        self.balance = max(0, int(self.GetKills() - self.GetDeaths()))

    def GetBalance(self):
        return self.balance

    def SetDamage(self, damage):
        self.damage = long(damage)

    def GetDamage(self):
        return self.damage

    def SetLevel(self, level):
        self.level = int(level)

    def GetLevel(self):
        return self.level

    def Render(self):
        name = self.GetName()
        self.killsLine.SetText(str(self.GetKills()))
        self.deathsLine.SetText(str(self.GetDeaths()))
        self.levelLine.SetText(str(self.GetLevel()))

        n = self.GetDamage()
        self.damageLine.SetText("%s" % ('.'.join(
            [i - 3 < 0 and str(n)[:i] or str(n)[i - 3:i] for i in range(len(str(n)) % 3, len(str(n)) + 1, 3) if i])))

        if player.GetName() == name:
            self.nameLine.SetText("> %s <" % name)
            self.nameLine.SetPackedFontColor(self.SPECIAL_POSITIVE_COLOR)
        else:
            self.nameLine.SetText(name)
            self.nameLine.SetPackedFontColor(self.TITLE_COLOR)


class GuildWarBoard(ui.Window):
    GOLD_COLOR = grp.GenerateColor(1.0, 0.7843, 0.0, 1.0)

    def __init__(self, parent):
        self.players = []
        self.guild_id = 0
        self.guild_name = localeInfo.Get("GUILD")

        ui.Window.__init__(self)
        self.SetParent(parent)
        self.__BuildWindow()

    def SetGuildName(self, name):
        self.guild_name = name
        self.title.SetText("%s" % name)

    def GetGuildName(self):
        return self.guild_name

    def SetGuildId(self, id):
        self.guild_id = int(id)

        if self.mark:
            self.mark.SetIndex(id)

    def GetGuildId(self):
        return self.guild_id

    def __CreatePlayer(self, name):
        for player in self.players:
            if player.GetName() == name:
                return player

        player = GuildWarPlayer()
        player.SetParent(self)
        player.SetName(name)
        player.Show()

        self.players.append(player)
        return player

    def AddPlayer(self, name, kills=0, deaths=0, level=0, damage=0):
        pl = self.__CreatePlayer(name)

        pl.SetKills(kills)
        pl.SetDeaths(deaths)
        pl.SetLevel(level)
        pl.SetDamage(damage)

        self.Render()

    def ClearPlayers(self):
        self.players = []

    def GetMVP(self):
        players = sorted(self.players, key=lambda player: (player.balance, player.damage), reverse=True)

        if len(players) == 0:
            return "~"

        return players[0].GetName()

    def GetTotalKills(self):
        x = 0
        for player in self.players:
            x += player.GetKills()
        return x

    def Render(self):
        players = sorted(self.players, key=lambda player: (player.balance, player.damage), reverse=True)
        players = players[:PLAYER_LIMIT]

        y = 29

        for player in players:
            player.SetPosition(0, y)
            player.Render()
            y += 20

        self.SetSize(BOARD_WIDTH - 22, y + 5)

    def __BuildWindow(self):
        self.SetPosition(11, 15)

        mark = ui.MarkBox()
        mark.SetParent(self)
        mark.SetPosition(3, 3)
        mark.SetIndex(self.guild_id)
        mark.SetScale(1)
        mark.Show()
        self.mark = mark

        title = ui.MakeTextLineNew(self, 22, -25, "")
        title.SetPackedFontColor(self.GOLD_COLOR)
        title.SetFontName("Verdana:12")
        title.SetText("")
        self.title = title

        kills = ui.MakeTextLineNew(self, 200, -25, "")
        kills.SetFontName("Verdana:12")
        kills.SetText(localeInfo.Get("WARBOARD_KILLS"))
        self.kills = kills

        deaths = ui.MakeTextLineNew(self, 270, -25, "")
        deaths.SetFontName("Verdana:12")
        deaths.SetText(localeInfo.Get("WARBOARD_DEATHS"))
        self.deaths = deaths

        level = ui.MakeTextLineNew(self, 423, -25, "")
        level.SetFontName("Verdana:12")
        level.SetText(localeInfo.Get("WARBOARD_LEVEL"))
        self.level = level

        damage = ui.MakeTextLineNew(self, 510, -25, "")
        damage.SetFontName("Verdana:12")
        damage.SetText(localeInfo.Get("WARBOARD_TOTAL_DAMAGE"))
        self.damage = damage

        separator = ui.Line()
        separator.SetParent(self)
        separator.SetPosition(0, 25)
        separator.SetColor(0x55ff0000)
        separator.SetSize(BOARD_WIDTH - 22, 0)
        separator.Show()
        self.separator = separator

        self.Render()

        self.Show()


class WarVersusBox(ui.Window):
    GOLD_COLOR = grp.GenerateColor(1.0, 0.7843, 0.0, 1.0)
    POSITIVE_COLOR = grp.GenerateColor(0.5411, 0.7254, 0.5568, 1.0)

    def __init__(self):
        self.boards = []
        self.marks = []
        self.names = []
        self.mvps = []

        ui.Window.__init__(self)
        self.__BuildWindow()

    def __MarkBox(self, x):
        mark = ui.MarkBox()
        mark.SetParent(self)
        mark.SetPosition(x, 0)
        mark.SetIndex(0)
        mark.SetScale(3)
        mark.Show()

        self.marks.append(mark)

    def __TextLine(self, text, x, color=None, centered=False):
        element = self.component.TextLine(self, "", x, 0, None)
        element.SetFontName("Verdana:12")
        element.SetText(text)

        if centered:
            element.SetHorizontalAlignCenter()

        if color:
            element.SetPackedFontColor(color)

        return element

    def __BuildWindow(self):
        c = Component()

        self.__MarkBox(16)
        self.__MarkBox(237)

        self.names.append(c.TextLine(self, "", 68, 1, None))
        self.names.append(c.TextLine(self, "", 289, 1, None))

        for name in self.names:
            name.SetPackedFontColor(self.GOLD_COLOR)
            name.SetFontName("Verdana:12")

        self.mvps.append(c.TextLine(self, "", 68, 17, None))
        self.mvps.append(c.TextLine(self, "", 289, 17, None))

        for mvp in self.mvps:
            mvp.SetPackedFontColor(self.POSITIVE_COLOR)
            mvp.SetFontName("Verdana:12")

        self.component = c
        self.Show()

    def Render(self, y):
        self.SetPosition(0, y)

        for i, board in enumerate(self.boards):
            self.marks[i].SetIndex(board.GetGuildId())
            self.marks[i].SetScale(3)
            self.names[i].SetText(localeInfo.Get("WARBOARD_GUILD_KILLS").format(board.GetGuildName(), board.GetTotalKills()))
            self.mvps[i].SetText(localeInfo.Get("WARBOARD_MVP").format(board.GetMVP()))

    def SetBoards(self, boards):
        self.boards = boards


class WarBoardWindow(ui.ThinBoard):

    def __init__(self):
        ui.ThinBoard.__init__(self)
        self.__BuildWindow()

        self.toggle = False

    def __BuildWindow(self):
        self.boards = []

        self.boards.append(GuildWarBoard(self))
        self.boards.append(GuildWarBoard(self))

        versus = WarVersusBox()
        versus.SetParent(self)
        versus.SetBoards(self.boards)
        self.versus = versus

        self.canOpen = False

        self.Render()

    def Render(self):
        y = self.boards[0].GetHeight()
        self.boards[1].SetPosition(11, 15 + y + 5)

        height = y + self.boards[1].GetHeight() + 20
        self.versus.Render(height)

        height += 15 * 2 + 15
        self.SetSize(BOARD_WIDTH, height)

    def AddPlayer(self, guildId, player):
        for board in self.boards:
            if board.GetGuildId() == guildId:
                board.AddPlayer(*player)
                return

    def Handle(self, inputText):
        inputText = inputText.split("|")

        if len(inputText) == 0:
            return

        if inputText[0] == "toggle":
            self.toggle = bool(int(inputText[1]))
            return

        if inputText[0] == "versus":
            self.boards[0].SetGuildId(int(inputText[1]))
            self.boards[0].SetGuildName(inputText[2])
            self.boards[1].SetGuildId(int(inputText[3]))
            self.boards[1].SetGuildName(inputText[4])
            self.versus.SetBoards(self.boards)

            for board in self.boards:
                board.ClearPlayers()

            self.Render()
            return

        if inputText[0] == "update":
            self.AddPlayer(int(inputText[1]), inputText[2:])
            self.Render()
            return

    def IsOnWar(self):
        return self.toggle

    def Open(self):
        if not self.IsOnWar():
            return False

        self.SetTop()
        self.SetCenterPosition()
        self.Show()
        return True

    def Close(self):
        if not self.IsShow():
            return False

        self.Hide()
        return True

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.OnPressEscapeKey()
            return True
        return False

    def OnPressEscapeKey(self):
        self.Close()
        return True
