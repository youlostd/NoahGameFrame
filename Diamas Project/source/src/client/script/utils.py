# -*- coding: utf-8 -*-


from pack import GetVfsFile

def clamp_index(new_index, container):
    return max(0, min(new_index, len(container) - 1))

from colorsys import rgb_to_hls, hls_to_rgb

def adjust_color_lightness(r, g, b, factor):
    h, l, s = rgb_to_hls(r / 255.0, g / 255.0, b / 255.0)
    l = max(min(l * factor, 1.0), 0.0)
    r, g, b = hls_to_rgb(h, l, s)
    return int(r * 255), int(g * 255), int(b * 255)

def lighten_color(r, g, b, factor=0.1):
    return adjust_color_lightness(r, g, b, 1 + factor)

def darken_color(r, g, b, factor=0.1):
    return adjust_color_lightness(r, g, b, 1 - factor)

class Sandbox(object):
    # DO NOT REMOVE ANY ENTRY OF WHITE LIST
    # THIS ENTIES ARE USED FOR Sandbox.
    #     (If sys.modules['__main__'] is changed, you can see the hell.)
    WHITE_LIST = ['__builtin__', 'types', __name__, '__main__', 'sys']

    def __init__(self, prevent_imported_modules=False, allowed_modules=[], prevented_modules=[], allowed_paths=[]):
        self.prevent_imported_modules = prevent_imported_modules
        self.allowed_modules = allowed_modules
        self.prevented_modules = prevented_modules
        self.allowed_paths = allowed_paths

    def add_allowed_modules(self, allowed_modules):
        self.allowed_modules = self.allowed_modules + allowed_modules

    def add_prevented_modules(self, prevented_modules):
        self.prevented_modules = self.prevented_modules + prevented_modules

    def execfile(self, filename, dic):
        import sys
        import copy

        for allowed_module_name in self.allowed_modules:
            try:
                exec 'import {0}'.format(allowed_module_name)
            except:
                # Just now, pass the exception.
                # (filename could not use this module)
                pass
        # shallow copy of sys.modules
        old_modules = copy.copy(sys.modules)

        old_path = sys.path
        sys.path = self.allowed_paths

        # set current imported modules None
        if self.prevent_imported_modules:
            import types
            for k, v in old_modules.items():
                if type(v) == types.ModuleType:
                    if not (k in self.WHITE_LIST or k in self.allowed_modules):
                        sys.modules[k] = None

        # set prevented modules None.
        for prevented_module_name in self.prevented_modules:
            sys.modules[prevented_module_name] = None

        try:
            data = GetVfsFile(filename)
            if not data:
                raise IOError("Cannot find file " + filename)


            code = compile(data, filename, 'exec')
            exec code in dic
        finally:
            # Restore original settings.
            # sys.modules = old_modules# <- This is not effective.
            # I don't know why, but I guess some where got old sys.modules reference and access old one.
            for k, v in sys.modules.items():
                if not k in old_modules:
                    del sys.modules[k]
            for k, v in old_modules.items():
                sys.modules[k] = v

            sys.path = old_path
