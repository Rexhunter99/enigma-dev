/** Copyright (C) 2013 forthevin
*** Copyright (C) 2013 Rexhunter99
***
*** This file is a part of the ENIGMA Development Environment.
***
*** ENIGMA is free software: you can redistribute it and/or modify it under the
*** terms of the GNU General Public License as published by the Free Software
*** Foundation, version 3 of the license or any later version.
***
*** This application and its source code is distributed AS-IS, WITHOUT ANY
*** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
*** FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
*** details.
***
*** You should have received a copy of the GNU General Public License along
*** with this code. If not, see <http://www.gnu.org/licenses/>
**/

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

#include <X11/Xlib.h>
#include "../General/glxew.h"
#include "Widget_Systems/widgets_mandatory.h"
#include "Platforms/xlib/XLIBmain.h"

namespace enigma
{
    namespace x11
    {
        GLXFBConfig     fbconfig;
        int				glx[2];
    }

    void InitGLXFuncs()
    {
		x11::glx[0] = x11::glx[1] = 0;

		if ( !glXQueryVersion( x11::disp, &x11::glx[0], &x11::glx[1] ) )
		{
			printf( "glX :: Failed to call QueryVersion()!\n" );
			return; // glXQueryVersion failed for some reason or other
		}


		// -- GLX 1.3+
		if ( (x11::glx[0] == 1 && x11::glx[1] >= 3) || x11::glx[0] >= 1 )
		{
			glXCreateNewContext					= (PFNGLXCREATENEWCONTEXTPROC)glXGetProcAddress( (const GLubyte*)"glXCreateNewContext" );
			glXChooseFBConfig					= (PFNGLXCHOOSEFBCONFIGPROC)glXGetProcAddress( (const GLubyte*)"glXChooseFBConfig" );
			glXGetVisualFromFBConfig			= (PFNGLXGETVISUALFROMFBCONFIGPROC)glXGetProcAddress( (const GLubyte*)"glXGetVisualFromFBConfig" );
		}
		// -- GLX 1.4+
		if ( (x11::glx[0] == 1 && x11::glx[1] >= 4) || x11::glx[0] >= 1 )
		{
			glXCreateContextAttribsARB			= (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddressARB( (const GLubyte*)"glXCreateContextAttribsARB" );
		}
    }

	void GetXVisual( XVisualInfo** vi )
	{
        // -- Check the GLX version, 1.3+ added FBConfigs
		if ( ( x11::glx[0] == 1 && x11::glx[1] < 3 ) || ( x11::glx[0] < 1 ) )
		{
            GLint att[] = {
                GLX_RGBA,
                GLX_DOUBLEBUFFER,
                GLX_RED_SIZE,		8,
                GLX_GREEN_SIZE,		8,
                GLX_BLUE_SIZE,		8,
                GLX_DEPTH_SIZE,		24,
                None
            };

            if( (*vi = glXChooseVisual( x11::disp, 0, att )) == NULL )
            {
                printf( "Failed to get a legacy XVisualInfo!\n" );
                exit( EXIT_FAILURE );
            }
        }
        else
        {

            // -- Make an FBConfig attribute list
            int visual_attribs[] =
            {
                GLX_X_RENDERABLE    , True,
                GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
                GLX_RENDER_TYPE     , GLX_RGBA_BIT,
                GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
                GLX_RED_SIZE        , 8,
                GLX_GREEN_SIZE      , 8,
                GLX_BLUE_SIZE       , 8,
                GLX_DEPTH_SIZE      , 24,
                GLX_DOUBLEBUFFER    , True,
                // -- MSAA
                //GLX_SAMPLE_BUFFERS  , 1,
                //GLX_SAMPLES         , 4,
                None
            };

            // -- Get a list of matching FrameBuffer Configs
            int fbcount;
            GLXFBConfig *fbc = glXChooseFBConfig( x11::disp, DefaultScreen( x11::disp ), visual_attribs, &fbcount );
            if ( !fbc )
            {
                printf( "Failed to retrieve a framebuffer config\n" );
                exit( EXIT_FAILURE );
            }
            x11::fbconfig = fbc[0];
            XFree( fbc );

            // -- Get the VisualInfo associated with this Config
            if ( (*vi = glXGetVisualFromFBConfig( x11::disp, x11::fbconfig )) == 0 )
            {
                printf( "Failed to get the XVisual from the FBConfig!\n" );
                exit( EXIT_FAILURE );
            }
        }
	}

	void EnableDrawing( XVisualInfo** vi, GLXContext* ctx )
	{
		// -- Check the GLX version, 1.3+ added FBConfigs
		if ( ( x11::glx[0] == 1 && x11::glx[1] < 3 ) || ( x11::glx[0] < 1 ) )
		{
			// -- Give us a GL context
            if ( (*ctx = glXCreateContext( x11::disp, *vi, NULL, True)) == 0 )
            {
                printf( "GL3 Failed to get Legacy context\n" );
                exit( EXIT_FAILURE );
            }
		}
		else
        {
			printf( "EnableDrawing() :: GLX 1.3+ Context Creation\n" );

            if ( !glXCreateContextAttribsARB )
            {
                printf( "glXCreateContextAttribsARB() not found ... using old-style GLX context\n" );
                *ctx = glXCreateNewContext( x11::disp, x11::fbconfig, GLX_RGBA_TYPE, 0, True );
            }
            else
            {
                int context_attribs[] =
                {
                    GLX_CONTEXT_MAJOR_VERSION_ARB,	3,
                    GLX_CONTEXT_MINOR_VERSION_ARB,	0,
                    GLX_CONTEXT_PROFILE_MASK_ARB,	GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
					#if defined( _DEBUG ) || defined( DEBUG ) || defined( ENIGMA_DEBUG )
					GLX_CONTEXT_FLAGS_ARB,			GLX_CONTEXT_DEBUG_BIT_ARB,
					#endif
                    None
                };

                if ( (*ctx = glXCreateContextAttribsARB( x11::disp, x11::fbconfig, 0, True, context_attribs )) == 0 )
				{
					printf( "GL3 Failed to get Core context\n" );
					exit( EXIT_FAILURE );
				}
            }
        }

		// -- Make the context current to this thread
		glXMakeCurrent( x11::disp, x11::win, *ctx );

        // -- Clear the back buffers
        glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ACCUM_BUFFER_BIT|GL_STENCIL_BUFFER_BIT );
	}

	void DisableDrawing( GLXContext* ctx )
	{
		//TODO: (Rexhunter99#1#): Context cleanup
	}

};

// NOTE: Changes/fixes that applies to this likely also applies to the OpenGL1 version.

namespace enigma {
  namespace swaphandling {
    bool has_checked_extensions = false;
    bool ext_swapcontrol_supported;
    bool mesa_swapcontrol_supported;

    void investigate_swapcontrol_support() {

      if (has_checked_extensions) return; // Already calculated, no need to calculate it more.

      // TODO: The second argument to glXQueryExtensionsString is screen number,
      // and it is unknown if the value 0 is generally correct for calling that function.
      // For more information, see the following pages:
      // http://pic.dhe.ibm.com/infocenter/aix/v6r1/index.jsp?topic=%2Fcom.ibm.aix.opengl%2Fdoc%2Fopenglrf%2FglXQueryExtensionsString.htm
      const char *glx_extensions = glXQueryExtensionsString(enigma::x11::disp, 0);

      ext_swapcontrol_supported = strstr(glx_extensions, "GLX_EXT_swap_control");
      mesa_swapcontrol_supported = strstr(glx_extensions, "GLX_MESA_swap_control");

      has_checked_extensions = true;
    }
  }

  bool is_ext_swapcontrol_supported() {
    swaphandling::investigate_swapcontrol_support();
    return swaphandling::ext_swapcontrol_supported;
  }
  bool is_mesa_swapcontrol_supported() {
    swaphandling::investigate_swapcontrol_support();
    return swaphandling::mesa_swapcontrol_supported;
  }
}

#include <Platforms/xlib/XLIBwindow.h> // window_set_caption
#include <Universal_System/roomsystem.h> // room_caption, update_mouse_variables

namespace enigma_user {
  void set_synchronization(bool enable) {

    // General notes:
    // Setting swapping on and off is platform-dependent and requires platform-specific extensions.
    // Platform-specific extensions are even more bothersome than regular extensions.
    // What functions and features to use depends on which version of OpenGL is used.
    // For more information, see the following pages:
    // http://www.opengl.org/wiki/Load_OpenGL_Functions
    // http://www.opengl.org/wiki/OpenGL_Loading_Library
    // http://www.opengl.org/wiki/Swap_Interval
    // http://en.wikipedia.org/wiki/GLX
    // Also note that OpenGL version >= 3.0 does not use glGetString for getting extensions.

    if (enigma::x11::disp != 0) {
      GLXDrawable drawable = glXGetCurrentDrawable();

      int interval = enable ? 1 : 0;

      if (enigma::is_ext_swapcontrol_supported()) {
        glXSwapIntervalEXT(enigma::x11::disp, drawable, interval);
      }
      else if (enigma::is_mesa_swapcontrol_supported()) {
        glXSwapIntervalMESA(interval);
      }
      // NOTE: GLX_SGI_swap_control, which is not used here, does not seem
      // to support disabling of synchronization, since its argument may not
      // be zero or less, so therefore it is not used here.
      // See http://www.opengl.org/registry/specs/SGI/swap_control.txt for more information.
    }
  }

  void screen_refresh() {
    glXSwapBuffers(enigma::x11::disp, enigma::x11::win);
    enigma::update_mouse_variables();
    window_set_caption(room_caption);
  }
}

