'''OpenGL extension APPLE.rgb_422

Automatically generated by the get_gl_extensions script, do not edit!
'''
from OpenGL import platform, constants, constant, arrays
from OpenGL import extensions
from OpenGL.GL import glget
import ctypes
EXTENSION_NAME = 'GL_APPLE_rgb_422'
_DEPRECATED = False
GL_RGB_422_APPLE = constant.Constant( 'GL_RGB_422_APPLE', 0x8A1F )


def glInitRgb422APPLE():
    '''Return boolean indicating whether this extension is available'''
    return extensions.hasGLExtension( EXTENSION_NAME )
