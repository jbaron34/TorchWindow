import ctypes
import torch
import os

torchWindowDir = os.path.dirname(__file__)
interfaceLibraryPath = torchWindowDir + "/libtwinterface.so"

tw = ctypes.CDLL(interfaceLibraryPath)

createWindow = tw.createWindow
createWindow.argtypes = [
    ctypes.c_int,
    ctypes.c_int,
    ctypes.c_int,
    ctypes.c_int,
    ctypes.c_char_p,
    ctypes.c_char_p
]
createWindow.restype = ctypes.c_void_p

isOpen = tw.isOpen
isOpen.argtypes = [ctypes.c_void_p]
isOpen.restype = ctypes.c_int

destroyWindow = tw.destroyWindow
destroyWindow.argtypes = [ctypes.c_void_p]
destroyWindow.restype = None

draw = tw.draw
draw.argtypes = [
    ctypes.c_void_p,
    ctypes.c_void_p,
    ctypes.c_int,
    ctypes.c_int,
    ctypes.c_int
]
draw.restypes = None

class Window:

    def __init__(self, winWidth, winHeight, name="Torch Window", texWidth=None, texHeight=None):
        assert isinstance(winWidth, int) and winWidth > 0, "winWidth must be an int of value > 0"
        assert isinstance(winHeight, int) and winHeight > 0, "winHeight must be an int of value > 0"

        if (texWidth == None):
            texWidth = winWidth
        if (texHeight == None):
            texHeight = winHeight

        assert isinstance(texWidth, int) and texWidth > 0, "texWidth must be an int of value > 0"
        assert isinstance(texHeight, int) and texHeight > 0, "texHeight must be an int of value > 0"

        super(Window, self).__init__()

        self.winWidth = winWidth
        self.winHeight = winHeight
        self.texWidth = texWidth
        self.texHeight = texHeight
        self.name = name

        self.c_window_handle = createWindow(winWidth, winHeight, texWidth, texHeight, ('\"'+name+'\"\0').encode('ascii'), torchWindowDir.encode('ascii'))
        self.closed = False

    def draw(self, tensor, layout='hwc'):
        if self.closed or not isOpen(self.c_window_handle):
            self.closed = True
        else:
            assert tensor.dtype == torch.float32, "tensor dtype must be torch.float32"
            assert tensor.dim() == 3, "tensor must have 3 dimensions"
            assert tensor.is_contiguous, "tensor must be contiguous"

            # TODO remove this requirement once support for other layouts has been added
            assert layout == 'hwc', "tensor layout must be 'hwc' (height, width, channels)"

            draw(self.c_window_handle, tensor.data_ptr(), *tensor.size())

    def close(self):
        if not self.closed:
            destroyWindow(self.c_window_handle)
            self.closed = True

    def __del__(self):
        self.close()
