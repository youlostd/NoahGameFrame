# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
from ui import AlphaBox
from ui import Window, ImageBox, AutoGrowingVerticalContainer, TextLine
from ui_event import MakeEvent


# Like AutoGrowingVerticalContainer but with Element Auto Positioning
class AutoGrowingVerticalContainerEx(AutoGrowingVerticalContainer):
    def __init__(self):
        AutoGrowingVerticalContainer.__init__(self)
        self.elementStartOffsetY = 0
        self.elementCurrentOffsetY = 0
        self.elementOffsetX = 0
        self.oldItems = []

    def SetStartOffset(self, offset):
        self.elementStartOffsetY = offset
        self.height = offset

    def AppendItem(self, item):
        addHeight = item.GetHeight() + self.elementOffsetY
        self.height += addHeight
        self.containerItems.append(item)
        self.containerItems[-1].SetPosition(self.elementOffsetX, self.height - self.containerItems[-1].GetHeight())
        self.Update()

    def ShowItems(self):
        for item in self.containerItems:
            item.Show()

    def HideItems(self):
        for item in self.containerItems:
            item.Hide()

    def GetItems(self):
        return self.containerItems

    def GetSize(self):
        return len(self.containerItems)

    def RecalculateHeight(self):
        self.oldItems = self.containerItems
        self.height = self.elementStartOffsetY
        self.containerItems = []

        for item in self.oldItems:
            self.AppendItem(item)


class ToggleAbleTitledWindow(Window):
    def __init__(self, imageFileName=None, icon=None):
        Window.__init__(self)

        self.baseSize = [200, 20]
        self.eventsOpenCloseActive = True

        if imageFileName:
            self.backgroundImage = ImageBox()
            self.backgroundImage.SetClickEvent(self.OnMouseLeftButtonDoubleClick)
            self.backgroundImage.SetParent(self)
            self.backgroundImage.LoadImage(imageFileName)
            self.backgroundImage.Show()

            self.toggleIndicator = ImageBox()
            self.toggleIndicator.SetParent(self.backgroundImage)
            self.toggleIndicator.SetClickEvent(self.OnToggle)
            self.toggleIndicator.SetHorizontalAlignRight()
            self.toggleIndicator.SetPosition(10, 3)
            self.toggleIndicator.Show()

            self.titleText = TextLine()
            self.titleText.SetFontName("Verdana:12")
            self.titleText.SetPackedFontGradient(0xffcca334, 0xffca982f)
            self.titleText.SetParent(self.backgroundImage)
            self.titleText.SetPosition(30, 6)
            self.titleText.SetHorizontalAlignLeft()
            self.titleText.Show()


            if icon:
                self.icon = ImageBox()
                self.icon.SetClickEvent(self.OnMouseLeftButtonDoubleClick)
                self.icon.SetParent(self.backgroundImage)
                self.icon.LoadImage(icon)
                self.icon.SetPosition(8, 8)
                self.icon.Show()

        else:
            self.backgroundImage = AlphaBox()
            self.backgroundImage.SetOnLeftButtonDoubleClickEvent(self.OnMouseLeftButtonDoubleClick)
            self.backgroundImage.SetParent(self)
            self.backgroundImage.SetAlpha(0.5)
            self.backgroundImage.SetColor(69, 37, 8)
            self.backgroundImage.SetSize(*self.baseSize)
            self.backgroundImage.Show()

            self.toggleIndicator = ImageBox()
            self.toggleIndicator.SetParent(self.backgroundImage)
            self.toggleIndicator.SetClickEvent(self.OnToggle)
            self.toggleIndicator.SetHorizontalAlignRight()
            self.toggleIndicator.SetPosition(10, 3)
            self.toggleIndicator.Show()

            self.titleText = TextLine()
            self.titleText.SetFontName("Nunito Sans:12b")
            self.titleText.SetPackedFontColor(0xffa1a1a1)
            self.titleText.SetOutline(True)
            self.titleText.SetParent(self.backgroundImage)
            self.titleText.SetPosition(4, 0)
            self.titleText.SetHorizontalAlignLeft()
            self.titleText.Show()


        self.toggleAbleContent = AutoGrowingVerticalContainerEx()
        self.toggleAbleContent.SetParent(self)
        self.toggleAbleContent.SetStartOffset(0)
        self.toggleAbleContent.SetPosition(0, self.backgroundImage.GetHeight())
        self.toggleAbleContent.Show()

        self.eventOnToggle = None
        self.eventOnGrow = None
        self.eventOpen = None
        self.eventClose = None
        self.toggleContentIsShown = False
        self.toggleShowFilename = ""
        self.toggleHideFilename = ""
        self.baseSize[1] = self.backgroundImage.GetHeight()
        self.width = 0

    def OnMouseLeftButtonDoubleClick(self):
        self.OnToggle()
        return True

    def SetOpenCloseEventStatus(self, status):
        self.eventsOpenCloseActive = status

    def SetWidth(self, width):
        self.toggleAbleContent.SetWidth(width)
        self.width = width
        self.SetSize(width, self.GetHeight())

    def SetTitle(self, title):
        self.titleText.SetText(title)

    def SetBackgroundImage(self, filename):
        self.backgroundImage = ImageBox()
        self.backgroundImage.SetParent(self)
        self.backgroundImage.LoadImage(filename)
        self.backgroundImage.Show()

    def SetToggleShowFilename(self, filename):
        self.toggleShowFilename = filename
        if not self.toggleContentIsShown:
            self.toggleIndicator.LoadImage(self.toggleShowFilename)

    def SetToggleHideFilename(self, filename):
        self.toggleHideFilename = filename
        if self.toggleContentIsShown:
            self.toggleIndicator.LoadImage(self.toggleHideFilename)

    def SetOnToggleEvent(self, event):
        self.eventOnToggle = MakeEvent(event)

    def SetOnGrowEvent(self, event):
        self.eventOnGrow = MakeEvent(event)

    def SetOpenEvent(self, event):
        self.eventOpen = MakeEvent(event)

    def SetCloseEvent(self, event):
        self.eventClose = MakeEvent(event)

    def AppendToToggleContent(self, item):
        self.toggleAbleContent.AppendItem(item)
        item.SetParent(self.toggleAbleContent)
        if self.eventOnGrow:
            self.eventOnGrow()
        self.Update()

    def Update(self):
        if self.toggleContentIsShown:
            self.SetSize(self.baseSize[0], self.baseSize[1] + self.toggleAbleContent.GetHeight())
        else:
            self.SetSize(*self.baseSize)
        self.UpdateRect()

    def Close(self):

        self.toggleIndicator.LoadImage(self.toggleShowFilename)
        self.toggleAbleContent.Hide()
        self.toggleAbleContent.HideItems()

        self.toggleContentIsShown = False

        self.Update()

        if self.eventsOpenCloseActive and self.eventClose:
            self.eventClose()

    def Open(self):

        self.toggleIndicator.LoadImage(self.toggleHideFilename)
        self.toggleAbleContent.Show()
        self.toggleAbleContent.ShowItems()
        self.toggleContentIsShown = True

        self.Update()

        if self.eventsOpenCloseActive and self.eventOpen:
            self.eventOpen()

    def OnToggle(self):

        self.eventsOpenCloseActive = False
        if self.toggleContentIsShown:
            self.Close()
        else:
            self.Open()

        self.eventsOpenCloseActive = True

        if self.eventOnToggle:
            self.eventOnToggle()


