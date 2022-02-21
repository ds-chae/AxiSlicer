'''OpenGL extension INGR.color_clamp

Automatically generated by the get_gl_extensions script, do not edit!
'''
from OpenGL import platform, constants, constant, arrays
from OpenGL import extensions
from OpenGL.GL import glget
import ctypes
EXTENSION_NAME = 'GL_INGR_color_clamp'
_DEPRECATED = False
GL_RED_MIN_CLAMP_INGR = constant.Constant( 'GL_RED_MIN_CLAMP_INGR', 0x8560 )
glget.addGLGetConstant( GL_RED_MIN_CLAMP_INGR, (1,) )
GL_GREEN_MIN_CLAMP_INGR = constant.Constant( 'GL_GREEN_MIN_CLAMP_INGR', 0x8561 )
glget.addGLGetConstant( GL_GREEN_MIN_CLAMP_INGR, (1,) )
GL_BLUE_MIN_CLAMP_INGR = constant.Constant( 'GL_BLUE_MIN_CLAMP_INGR', 0x8562 )
glget.addGLGetConstant( GL_BLUE_MIN_CLAMP_INGR, (1,) )
GL_ALPHA_MIN_CLAMP_INGR = constant.Constant( 'GL_ALPHA_MIN_CLAMP_INGR', 0x8563 )
glget.addGLGetConstant( GL_ALPHA_MIN_CLAMP_INGR, (1,) )
GL_RED_MAX_CLAMP_INGR = constant.Constant( 'GL_RED_MAX_CLAMP_INGR', 0x8564 )
glget.addGLGetConstant( GL_RED_MAX_CLAMP_INGR, (1,) )
GL_GREEN_MAX_CLAMP_INGR = constant.Constant( 'GL_GREEN_MAX_CLAMP_INGR', 0x8565 )
glget.addGLGetConstant( GL_GREEN_MAX_CLAMP_INGR, (1,) )
GL_BLUE_MAX_CLAMP_INGR = constant.Constant( 'GL_BLUE_MAX_CLAMP_INGR', 0x8566 )
glget.addGLGetConstant( GL_BLUE_MAX_CLAMP_INGR, (1,) )
GL_ALPHA_MAX_CLAMP_INGR = constant.Constant( 'GL_ALPHA_MAX_CLAMP_INGR', 0x8567 )
glget.addGLGetConstant( GL_ALPHA_MAX_CLAMP_INGR, (1,) )


def glInitColorClampINGR():
    '''Return boolean indicating whether this extension is available'''
    return extensions.hasGLExtension( EXTENSION_NAME )
