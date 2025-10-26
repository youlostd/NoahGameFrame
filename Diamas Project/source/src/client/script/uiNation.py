# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import ui


class NationWindow(ui.ScriptWindow):

    def __init__(self): 
        ui.ScriptWindow.__init__(self) 
        self.isLoaded = 0

    def __LoadWindow(self):
        if self.isLoaded == 1:
            return

        self.isLoaded = 1
        self.__LoadScript("UIScript/NationWarDialog.py")

        self.live = self.GetChild("livevalue")
        self.empire1 = self.GetChild("empirevalue1")
        self.empire2 = self.GetChild("empirevalue2")
        self.empire3 = self.GetChild("empirevalue3")
        self.empiretext1 = self.GetChild("empiretext1")
        self.empiretext2 = self.GetChild("empiretext2")
        self.empiretext3 = self.GetChild("empiretext3")

    def RefreshNation(self):
        if self.isLoaded==0:
            return

    def Show(self):
        self.__LoadWindow()
        ui.ScriptWindow.Show(self)

    #def OnPressEscapeKey(self):
    #    self.Hide()
    #    return True

    def __LoadScript(self, fileName):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, fileName)
