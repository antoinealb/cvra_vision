"python wrappers for the vision library"

import ctypes

VISION = ctypes.cdll.LoadLibrary("./build/libvision.so")

YELLOW = ctypes.c_ubyte.in_dll(VISION, "YELLOW").value
RED = ctypes.c_ubyte.in_dll(VISION, "RED").value
ERROR = ctypes.c_ubyte.in_dll(VISION, "ERROR").value

class CTriangle(ctypes.Structure):
    "ctypes wrapper for Triangle struct in vision"
    pass

class CTriangleIterator(ctypes.Structure):
    "ctypes wrapper for opaque TriangleIterator"
    pass


HASNEXT = VISION.hasNext
HASNEXT.argtypes = [ctypes.POINTER(CTriangleIterator)]
HASNEXT.restype = ctypes.c_bool

NEXT = VISION.next
NEXT.argtypes = [ctypes.POINTER(CTriangleIterator)]
NEXT.restype = CTriangle

GETTRIANGLES = VISION.getTriangles
GETTRIANGLES.argtypes = []
GETTRIANGLES.restype = ctypes.POINTER(CTriangleIterator)

DELETEITER = VISION.deleteTriangleIterator
DELETEITER.argtypes = [ctypes.POINTER(CTriangleIterator)]
DELETEITER.restype = None

CHECKCOLOR = VISION.vision_check_color
CHECKCOLOR.argtypes = []
CHECKCOLOR.restype = ctypes.c_ubyte

GETX = VISION.getX
GETX.argtypes = [ctypes.POINTER(CTriangle)]
GETX.restype = ctypes.c_float

GETY = VISION.getY
GETY.argtypes = [ctypes.POINTER(CTriangle)]
GETY.restype = ctypes.c_float

GETZ = VISION.getZ
GETZ.argtypes = [ctypes.POINTER(CTriangle)]
GETZ.restype = ctypes.c_float

GETCOLOR = VISION.getColor
GETCOLOR.argtypes = [ctypes.POINTER(CTriangle)]
GETCOLOR.restype = ctypes.c_ubyte

GETHORIZ = VISION.getHorizontal
GETHORIZ.argtypes = [ctypes.POINTER(CTriangle)]
GETHORIZ.restype = ctypes.c_bool

class Triangle(object):
    "python triangle class"

    def __init__(self, ctriangleref):
        "initialize from CTriangle"
        self.xpos = GETX(ctriangleref).value
        self.ypos = GETY(ctriangleref).value
        self.zpos = GETZ(ctriangleref).value
        self.color = GETCOLOR(ctriangleref).value
        self.horizontal = GETHORIZ(ctriangleref).value

def get_triangles():
    "Iterator over all triangles as Python object.."
    iterator = GETTRIANGLES()
    while HASNEXT(iterator):
        yield Triangle(NEXT(iterator))

    DELETEITER(iterator)

def check_color():
    "wrap c checkcolor funtion"
    return CHECKCOLOR().value
