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

#include <stdio>
#include <cstring>

using namespace std;

#include <X11/Xlib.h>
#include "../General/glxew.h"
#include <Platforms/xlib/XLIBmain.h>
#include <Graphics_Systems/graphics_mandatory.h>

namespace enigma
{

	void GetXVisual( XVisualInfo** vi )
	{
        GLint att[] = {
            GLX_RGBA,
            GLX_DOUBLEBUFFER,
            GLX_DEPTH_SIZE,		24,
            None
        };

        *vi = glXChooseVisual( x11::disp, 0, att );

        if( *vi == NULL )
        {
            show_error( "Failed to get a legacy XVisualInfo!\n", 1 );
        }
	}

	void EnableDrawing( XVisualInfo** vi, GLXContext* ctx )
	{
        // -- Give us a GL context
        if ( (*ctx = glXCreateContext( x11::disp, *vi, NULL, True)) == 0 )
        {
            show_error( "GL3 Failed to get Legacy context\n", 1 );
            return;
        }

        // -- Make the context current to this thread
        glXMakeCurrent( x11::disp, x11::win, *ctx );

        // -- Clear the back buffers
        glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ACCUM_BUFFER_BIT|GL_STENCIL_BUFFER_BIT );
	}

	void DisableDrawing()
	{
	}

};


// NOTE: Changes/fixes that applies to this likely also applies to the OpenGL3 version.

namespace enigma {
  namespace swaphandling {
    static bool has_checked_extensions = false;
    static bool ext_swapcontrol_supported;
    static bool mesa_swapcontrol_supported;

    static void investigate_swapcontrol_support() {
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

  static bool is_ext_swapcontrol_supported() {
    swaphandling::investigate_swapcontrol_support();
    return swaphandling::ext_swapcontrol_supported;
  }
  static bool is_mesa_swapcontrol_supported() {
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

