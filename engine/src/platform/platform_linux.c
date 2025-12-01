#include "platform.h"
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

#define TPLATFORM_WINDOW_X11
#if TPLATFORM_LINUX
#include "../core/logger.h"
#include "../core/input.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(TPLATFORM_WINDOW_WAYLAND)
//Wayland

//temporary stuff
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <wayland-util.h>

u32 globalWidth;
u32 globalHeight;

i32 shm_alloc(u64 size) {
	char name[8];
	name[0] = '/';
	name[7] = 0;
	for (uint8_t i = 1; i < 6; i++) {
		name[i] = (rand() & 23) + 97;
	}

	i32 fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH);
	shm_unlink(name);
	ftruncate(fd, size);

	return fd;
}

struct wl_shm *shm;
struct wl_buffer* buffer;
u8 *pixels;

void resize(u16 width, u16 height) {
  i32 fd = shm_alloc(width * height * 4);
  pixels = mmap(0, width * height * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  struct wl_shm_pool* pool = wl_shm_create_pool(shm, fd, width * height * 4);
  buffer = wl_shm_pool_create_buffer(pool, 0, width, height, width * 4, WL_SHM_FORMAT_ARGB8888);
  wl_shm_pool_destroy(pool);
  close(fd);
}

void draw(struct wl_surface* surface, u16 width, u16 height) {
  memset(pixels, 0xFFFFFFFF, width * height * 4);
  wl_surface_attach(surface, buffer, 0, 0);
  wl_surface_damage_buffer(surface, 0, 0, width, height);
  wl_surface_commit(surface);
}

void format(void* data, struct wl_shm *shmLocal, u32 name) {
}

struct wl_shm_listener shmListener = {
  .format=format
};
//end of temporary stuff

#include "../core/asserts.h"
#include "wayland-protocols/fractional-scale.h"
#include "wayland-protocols/pointer-constraints.h"
#include "wayland-protocols/xdg-decoration.h"
#include "wayland-protocols/xdg-relative-pointer.h"
#include "wayland-protocols/xdg-shell-protocol.h"

typedef struct PlatformState {
  struct {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_surface *surface;
    struct wl_seat *seat;
    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;
    struct wp_fractional_scale_v1 *fractionalScale;
    struct zwp_relative_pointer_v1 *relativePointer;
    struct zwp_pointer_constraints_v1 *pointerConstraint;

    struct wp_fractional_scale_manager_v1 *fractionalScaleManager;
    struct zwp_relative_pointer_manager_v1 *relativePointerManager;
  } wayland;

  struct {
    struct xdg_wm_base *shell;
    struct xdg_surface *surface;
    struct xdg_toplevel *top;
    struct zxdg_decoration_manager_v1 *decorationManager;
    struct zxdg_toplevel_decoration_v1 *decoration;
  } xdg;
  b8 open;
} PlatformState;

void registry_global(void *data, struct wl_registry *registry, u32 name, const char* interface, u32 version);
void registry_global_remove(void *data, struct wl_registry *registry, u32 name);
void noop();
void surface_enter();
void surface_leave();
void pointer_enter();
void pointer_leave();
void pointer_motion();
void pointer_button();
void pointer_axis(); 
void pointer_frame();
void relative_pointer_relative_motion(
    void *data,
    struct zwp_relative_pointer_v1 *relativePointer,
    u32 utimeHigh,
    u32 utimeLow,
    wl_fixed_t dx,
    wl_fixed_t dy,
    wl_fixed_t dxAcceleration,
    wl_fixed_t dyAcceleration);
void xdg_shell_ping(void* data, struct xdg_wm_base *shell, u32 serial);
void xdg_surface_configure(void* data, struct xdg_surface *surface, u32 serial);
void xdg_toplevel_configure(void* data, struct xdg_toplevel *top, i32 newWidth, i32 newHeight, struct wl_array *array);
void xdg_toplevel_close(void* data, struct xdg_toplevel *top);
void xdg_decoration_configure(void *data, struct zxdg_toplevel_decoration_v1 *decoration, u32 mode);

struct wl_registry_listener registryListener = {
  .global = registry_global,
  .global_remove = registry_global_remove
};

struct wl_surface_listener surfaceListener = {
  .enter = surface_enter,
  .leave = surface_leave,
  .preferred_buffer_scale = noop,
  .preferred_buffer_transform = noop
};

struct wl_seat_listener seatListener = {
  .capabilities=noop,
  .name=noop
};

struct wl_pointer_listener pointerListener = {
  .enter=pointer_enter,
  .leave=pointer_leave,
  .motion=pointer_motion,
  .button=pointer_button,
  .axis=pointer_axis,
  .frame=pointer_frame,
  .axis_source=noop,
  .axis_stop=noop,
  .axis_discrete=noop,
  .axis_value120=noop,
  .axis_relative_direction=noop
};

struct wl_keyboard_listener keyboardListener = {
  .keymap=noop,
  .enter=noop,
  .leave=noop,
  .key=noop,
  .modifiers=noop,
  .repeat_info=noop
};

struct zwp_relative_pointer_v1_listener relativePointerListener = {
  .relative_motion=relative_pointer_relative_motion
};

struct wp_fractional_scale_v1_listener fractionalScaleListener = {
  .preferred_scale=noop
};

struct xdg_wm_base_listener xdgShellListener = {
  .ping = xdg_shell_ping
};

struct xdg_surface_listener xdgSurfaceListener = {
  .configure = xdg_surface_configure
};

struct xdg_toplevel_listener xdgTopListener = {
  .configure = xdg_toplevel_configure,
  .close = xdg_toplevel_close,
  .configure_bounds = noop,
  .wm_capabilities = noop,
};

struct zxdg_toplevel_decoration_v1_listener zxdgDecorationListener = {
  .configure=xdg_decoration_configure
};

b8 platform_startup(void **platformState, const char *applicationName, i32 x, i32 y, i32 width, i32 height) {
  globalWidth = width;
  globalHeight = height;
  
  pixels = NULL;
  buffer = NULL;
  shm = NULL;

  *platformState = malloc(sizeof(PlatformState));
  PlatformState *state = (PlatformState*)*platformState;
  state->open = true;
  //wayland
  TASSERT(state->wayland.display = wl_display_connect(NULL));
  TASSERT(state->wayland.registry =  wl_display_get_registry(state->wayland.display));
  TASSERT(wl_registry_add_listener(state->wayland.registry, &registryListener, state) != -1);

  TASSERT(wl_display_roundtrip(state->wayland.display) != -1);

  TASSERT(state->wayland.compositor);
  TASSERT(state->wayland.surface = wl_compositor_create_surface(state->wayland.compositor));
  TASSERT(wl_surface_add_listener(state->wayland.surface, &surfaceListener, state) != -1);

  TASSERT(state->wayland.seat);
  TASSERT(wl_seat_add_listener(state->wayland.seat, &seatListener, state) != -1);

  TASSERT(state->wayland.pointer = wl_seat_get_pointer(state->wayland.seat));
  TASSERT(wl_pointer_add_listener(state->wayland.pointer, &pointerListener, state) != -1);
  TASSERT(state->wayland.keyboard = wl_seat_get_keyboard(state->wayland.seat));
  TASSERT(wl_keyboard_add_listener(state->wayland.keyboard, &keyboardListener, state) != -1);

  TASSERT(state->wayland.fractionalScaleManager);
  TASSERT(state->wayland.fractionalScale = wp_fractional_scale_manager_v1_get_fractional_scale(state->wayland.fractionalScaleManager, state->wayland.surface));
  TASSERT(wp_fractional_scale_v1_add_listener(state->wayland.fractionalScale, &fractionalScaleListener, state) != -1);

  TASSERT(state->wayland.relativePointerManager);
  TASSERT(state->wayland.relativePointer = zwp_relative_pointer_manager_v1_get_relative_pointer(state->wayland.relativePointerManager, state->wayland.pointer));
  TASSERT(zwp_relative_pointer_v1_add_listener(state->wayland.relativePointer, &relativePointerListener, state) != -1);

  //xdg
  TASSERT(state->xdg.shell);
  TASSERT(xdg_wm_base_add_listener(state->xdg.shell, &xdgShellListener, state) != -1);
  TASSERT(state->xdg.surface = xdg_wm_base_get_xdg_surface(state->xdg.shell, state->wayland.surface));
  TASSERT(xdg_surface_add_listener(state->xdg.surface, &xdgSurfaceListener,state) != -1);
  TASSERT(state->xdg.top = xdg_surface_get_toplevel(state->xdg.surface));
  TASSERT(xdg_toplevel_add_listener(state->xdg.top, &xdgTopListener, state) != -1);

  TASSERT(state->xdg.decorationManager);
  TASSERT(state->xdg.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(state->xdg.decorationManager, state->xdg.top));
  TASSERT(zxdg_toplevel_decoration_v1_add_listener(state->xdg.decoration, &zxdgDecorationListener, state) != -1);

  xdg_toplevel_set_title(state->xdg.top, applicationName);
  wl_surface_commit(state->wayland.surface);
  
  return true;
}

void platform_shutdown(void *platformState) {
  PlatformState *state = platformState;

  xdg_surface_destroy(state->xdg.surface);
  xdg_wm_base_destroy(state->xdg.shell);
  xdg_toplevel_destroy(state->xdg.top);
  zxdg_decoration_manager_v1_destroy(state->xdg.decorationManager);
  zxdg_toplevel_decoration_v1_destroy(state->xdg.decoration);

  wp_fractional_scale_manager_v1_destroy(state->wayland.fractionalScaleManager);
  wp_fractional_scale_v1_destroy(state->wayland.fractionalScale);
  zwp_pointer_constraints_v1_destroy(state->wayland.pointerConstraint);
  zwp_relative_pointer_manager_v1_destroy(state->wayland.relativePointerManager);
  zwp_relative_pointer_v1_destroy(state->wayland.relativePointer);

  wl_keyboard_destroy(state->wayland.keyboard);
  wl_pointer_destroy(state->wayland.pointer);
  wl_seat_destroy(state->wayland.seat);
  wl_surface_destroy(state->wayland.surface);
  wl_compositor_destroy(state->wayland.compositor);
  wl_registry_destroy(state->wayland.registry);
  wl_display_disconnect(state->wayland.display);
  free(state);
}

b8 platform_pump_message(void *platformState) {
  PlatformState *state = (PlatformState*)platformState;
  i32 result = wl_display_roundtrip(state->wayland.display);
  return state->open && (result != -1);
}

void registry_global(void *data, struct wl_registry *registry, u32 name, const char* interface, u32 version) {
  PlatformState *state = data;
  if(strcmp(interface, wl_compositor_interface.name) == 0) {
    TASSERT(state->wayland.compositor = wl_registry_bind(state->wayland.registry, name, &wl_compositor_interface, version));
  }
  else if(strcmp(interface, wl_seat_interface.name) == 0) {
    TASSERT(state->wayland.seat = wl_registry_bind(state->wayland.registry, name, &wl_seat_interface, version));
  }
  else if(strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0) {
    TASSERT(state->wayland.pointerConstraint = wl_registry_bind(state->wayland.registry, name, &zwp_pointer_constraints_v1_interface, version));
  }
  else if(strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0) {
    TASSERT(state->wayland.relativePointerManager = wl_registry_bind(state->wayland.registry, name, &zwp_relative_pointer_manager_v1_interface, version));
  }
  else if(strcmp(interface, wp_fractional_scale_manager_v1_interface.name) == 0) {
    TASSERT(state->wayland.fractionalScaleManager = wl_registry_bind(state->wayland.registry, name, &wp_fractional_scale_manager_v1_interface, version));
  }
  else if(strcmp(interface, xdg_wm_base_interface.name) == 0) {
    TASSERT(state->xdg.shell = wl_registry_bind(state->wayland.registry, name, &xdg_wm_base_interface, version));
  }
  else if(strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
    TASSERT(state->xdg.decorationManager = wl_registry_bind(state->wayland.registry, name, &zxdg_decoration_manager_v1_interface, version));
  }
  //temporary
  else if(strcmp(interface, wl_shm_interface.name) == 0) {
    TASSERT(shm = wl_registry_bind(state->wayland.registry, name, &wl_shm_interface, version));
  }
}

void registry_global_remove(void *data, struct wl_registry *registry, u32 name) {
  //NO OP
}

void noop() {
  //NO OPERATION
}

void surface_enter() {
}

void surface_leave() {
}

void pointer_enter() {
}

void pointer_leave() {
}

void pointer_motion() {
}

void pointer_button() {
}

void pointer_axis() {
}

void pointer_frame() {
}

void relative_pointer_relative_motion(void *data, struct zwp_relative_pointer_v1 *relativePointer, u32 utimeHigh, u32 utimeLow,
  wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dxAcceleration, wl_fixed_t dyAcceleration) {
}

void xdg_shell_ping(void* data, struct xdg_wm_base *shell, u32 serial) {
  xdg_wm_base_pong(shell, serial);
}

void xdg_surface_configure(void* data, struct xdg_surface *surface, u32 serial) {
  PlatformState *state = data;
  xdg_surface_ack_configure(state->xdg.surface, serial);
  if(!pixels) {
    resize(globalWidth, globalHeight);
  }
  draw(state->wayland.surface, globalWidth, globalHeight);
}

void xdg_toplevel_configure(void* data, struct xdg_toplevel *top, i32 newWidth, i32 newHeight, struct wl_array *array) {
  if(newWidth > 0 && newHeight > 0) {
    globalWidth = newWidth;
    globalHeight = newHeight;
    resize(globalWidth, globalHeight);
  }
}

void xdg_toplevel_close(void* data, struct xdg_toplevel *top) {
  PlatformState *state = data;
  state->open = false;
}

void xdg_decoration_configure(void *data, struct zxdg_toplevel_decoration_v1 *decoration, u32 mode) {
}

#elif defined(TPLATFORM_WINDOW_X11)
//X11
#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>  // sudo apt-get install libx11-dev
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>  // sudo apt-get install libxkbcommon-x11-dev
#include <sys/time.h>

typedef struct PlatformState {
  Display* display;
  xcb_connection_t* connection;
  xcb_window_t window;
  xcb_screen_t* screen;
  xcb_atom_t wmProtocols;
  xcb_atom_t wmDeleteWin;
} PlatformState;

Keys translate_keycode(u32 xKeycode);

b8 platform_startup(void** platformState, const char* applicationName, i32 x, i32 y, i32 width, i32 height) {
  *platformState = malloc(sizeof(PlatformState));
  PlatformState *state = (PlatformState*)*platformState;

  state->display = XOpenDisplay(NULL);
  XAutoRepeatOff(state->display);

 state->connection = XGetXCBConnection(state->display);

  if (xcb_connection_has_error(state->connection)) {
    TFATAL("Failed to connect to X server via XCB.");
    return false;
  }

  // Get data from the X server
  const struct xcb_setup_t* setup = xcb_get_setup(state->connection);

  // Loop through screens using iterator
  xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
  int screenP = 0;
  for (i32 s = screenP; s > 0; s--) {
      xcb_screen_next(&it);
  }

  // After screens have been looped through, assign it.
  state->screen = it.data;

  // Allocate a XID for the window to be created.
  state->window = xcb_generate_id(state->connection);

  // Register event types.
  // XCB_CW_BACK_PIXEL = filling then window bg with a single colour
  // XCB_CW_EVENT_MASK is required.
  u32 eventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

  // Listen for keyboard and mouse buttons
  u32 eventValues = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                     XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                     XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
                     XCB_EVENT_MASK_STRUCTURE_NOTIFY;

  // Values to be sent over XCB (bg colour, events)
  u32 valueList[] = {state->screen->black_pixel, eventValues};

  // Create the window
  xcb_void_cookie_t cookie = xcb_create_window(
      state->connection,
      XCB_COPY_FROM_PARENT,  // depth
      state->window,
      state->screen->root,            // parent
      x,                              //x
      y,                              //y
      width,                          //width
      height,                         //height
      0,                              // No border
      XCB_WINDOW_CLASS_INPUT_OUTPUT,  //class
      state->screen->root_visual,
      eventMask,
      valueList);

  // Change the title
  xcb_change_property(
      state->connection,
      XCB_PROP_MODE_REPLACE,
      state->window,
      XCB_ATOM_WM_NAME,
      XCB_ATOM_STRING,
      8,  // data should be viewed 8 bits at a time
      strlen(applicationName),
      applicationName);

  // Tell the server to notify when the window manager
  // attempts to destroy the window.
  xcb_intern_atom_cookie_t wmDeleteCookie = xcb_intern_atom(
      state->connection,
      0,
      strlen("WM_DELETE_WINDOW"),
      "WM_DELETE_WINDOW");
  xcb_intern_atom_cookie_t wmProtocolsCookie = xcb_intern_atom(
      state->connection,
      0,
      strlen("WM_PROTOCOLS"),
      "WM_PROTOCOLS");
  xcb_intern_atom_reply_t* wmDeleteReply = xcb_intern_atom_reply(
      state->connection,
      wmDeleteCookie,
      NULL);
  xcb_intern_atom_reply_t* wmProtocolsReply = xcb_intern_atom_reply(
      state->connection,
      wmProtocolsCookie,
      NULL);
  state->wmDeleteWin = wmDeleteReply->atom;
  state->wmProtocols = wmProtocolsReply->atom;

  xcb_change_property(
      state->connection,
      XCB_PROP_MODE_REPLACE,
      state->window,
      wmProtocolsReply->atom,
      4,
      32,
      1,
      &wmDeleteReply->atom);

  // Map the window to the screen
  xcb_map_window(state->connection, state->window);

  // Flush the stream
  i32 stream_result = xcb_flush(state->connection);
  if (stream_result <= 0) {
    TFATAL("An error occurred when flusing the stream: %d", stream_result);
    free(wmDeleteReply);
    free(wmProtocolsReply);
    return false;
  }
  free(wmDeleteReply);
  free(wmProtocolsReply);
  return true;
}
 
void platform_shutdown(void* platformState) {
  PlatformState *state = (PlatformState*)platformState;

  // Turn key repeats back on since this is global for the OS... just... wow.
  XAutoRepeatOn(state->display);

  XCloseDisplay(state->display);
  xcb_destroy_window(state->connection, state->window);
  free(state);
}

b8 platform_pump_message(void* platformState) {
  PlatformState *state = (PlatformState*)platformState;
  xcb_generic_event_t* event = NULL;
  xcb_client_message_event_t* cm;

  b8 quitFlagged = false;

  // Poll for events until null is returned.
  while((event = xcb_poll_for_event(state->connection))) {
    if (event == 0) {
      break;
    }

    // Input events
    switch (event->response_type & ~0x80) {
      case XCB_KEY_PRESS:
      case XCB_KEY_RELEASE: {
        xcb_key_press_event_t *kbEvent = (xcb_key_press_event_t *)event;
        b8 pressed = event->response_type == XCB_KEY_PRESS;
        xcb_keycode_t code = kbEvent->detail;
        KeySym keySymbol = XkbKeycodeToKeysym(state->display, (KeyCode)code, 0, code & ShiftMask ? 1 : 0);

        Keys key = translate_keycode(keySymbol);

        input_process_key(key, pressed);
      } break;
      case XCB_BUTTON_PRESS:
      case XCB_BUTTON_RELEASE: {
        // TODO: Mouse button presses and releases
        xcb_button_press_event_t *mouseEvent = (xcb_button_press_event_t *)event;
        b8 pressed = event->response_type == XCB_BUTTON_PRESS;
        Buttons mouseButton = BUTTON_MAX_BUTTONS;
        switch (mouseEvent->detail) {
          case XCB_BUTTON_INDEX_1:
            mouseButton = BUTTON_LEFT;
          break;
          case XCB_BUTTON_INDEX_2:
            mouseButton = BUTTON_MIDDLE;
          break;
          case XCB_BUTTON_INDEX_3:
            mouseButton = BUTTON_RIGHT;
          break;
        }

        // Pass over to the input subsystem.
        if (mouseButton != BUTTON_MAX_BUTTONS) {
          input_process_button(mouseButton, pressed);
        }
      } break;
      case XCB_MOTION_NOTIFY: {
        xcb_motion_notify_event_t *moveEvent = (xcb_motion_notify_event_t *)event;
        // Pass over to the input subsystem.
        input_process_move_move(moveEvent->event_x, moveEvent->event_y);
      } break;
      case XCB_CONFIGURE_NOTIFY: {
        // TODO: Resizing
      }
      case XCB_CLIENT_MESSAGE: {
        cm = (xcb_client_message_event_t*)event;

        // Window close
        if (cm->data.data32[0] == state->wmDeleteWin) {
          quitFlagged = true;
        }
      } break;
      default:
        // Something else
        break;
    }
    free(event);
  }
  return !quitFlagged;
}

Keys translate_keycode(u32 xKeycode) {
  switch(xKeycode) {
    case XK_BackSpace:
      return KEY_BACKSPACE;
    case XK_Return:
      return KEY_ENTER;
    case XK_Tab:
      return KEY_TAB;
        //case XK_Shift: return KEY_SHIFT;
        //case XK_Control: return KEY_CONTROL;

    case XK_Pause:
      return KEY_PAUSE;
    case XK_Caps_Lock:
      return KEY_CAPITAL;

    case XK_Escape:
      return KEY_ESCAPE;

        // Not supported
        // case : return KEY_CONVERT;
        // case : return KEY_NONCONVERT;
        // case : return KEY_ACCEPT;

    case XK_Mode_switch:
      return KEY_MODECHANGE;

    case XK_space:
      return KEY_SPACE;
    case XK_Prior:
      return KEY_PRIOR;
    case XK_Next:
      return KEY_NEXT;
    case XK_End:
      return KEY_END;
    case XK_Home:
      return KEY_HOME;
    case XK_Left:
      return KEY_LEFT;
    case XK_Up:
      return KEY_UP;
    case XK_Right:
      return KEY_RIGHT;
    case XK_Down:
      return KEY_DOWN;
    case XK_Select:
      return KEY_SELECT;
    case XK_Print:
      return KEY_PRINT;
    case XK_Execute:
      return KEY_EXECUTE;
    // case XK_snapshot: return KEY_SNAPSHOT; // not supported
    case XK_Insert:
      return KEY_INSERT;
    case XK_Delete:
      return KEY_DELETE;
    case XK_Help:
      return KEY_HELP;

    case XK_Meta_L:
      return KEY_LWIN;  // TODO: not sure this is right
    case XK_Meta_R:
      return KEY_RWIN;
        // case XK_apps: return KEY_APPS; // not supported

        // case XK_sleep: return KEY_SLEEP; //not supported

    case XK_KP_0:
      return KEY_NUMPAD0;
    case XK_KP_1:
      return KEY_NUMPAD1;
    case XK_KP_2:
      return KEY_NUMPAD2;
    case XK_KP_3:
      return KEY_NUMPAD3;
    case XK_KP_4:
      return KEY_NUMPAD4;
    case XK_KP_5:
      return KEY_NUMPAD5;
    case XK_KP_6:
      return KEY_NUMPAD6;
    case XK_KP_7:
      return KEY_NUMPAD7;
    case XK_KP_8:
      return KEY_NUMPAD8;
    case XK_KP_9:
      return KEY_NUMPAD9;
    case XK_multiply:
      return KEY_MULTIPLY;
    case XK_KP_Add:
      return KEY_ADD;
    case XK_KP_Separator:
      return KEY_SEPARATOR;
    case XK_KP_Subtract:
      return KEY_SUBTRACT;
    case XK_KP_Decimal:
      return KEY_DECIMAL;
    case XK_KP_Divide:
      return KEY_DIVIDE;
    case XK_F1:
      return KEY_F1;
    case XK_F2:
      return KEY_F2;
    case XK_F3:
      return KEY_F3;
    case XK_F4:
      return KEY_F4;
    case XK_F5:
      return KEY_F5;
    case XK_F6:
      return KEY_F6;
    case XK_F7:
      return KEY_F7;
    case XK_F8:
      return KEY_F8;
    case XK_F9:
      return KEY_F9;
    case XK_F10:
      return KEY_F10;
    case XK_F11:
      return KEY_F11;
    case XK_F12:
      return KEY_F12;
    case XK_F13:
      return KEY_F13;
    case XK_F14:
      return KEY_F14;
    case XK_F15:
      return KEY_F15;
    case XK_F16:
      return KEY_F16;
    case XK_F17:
      return KEY_F17;
    case XK_F18:
      return KEY_F18;
    case XK_F19:
      return KEY_F19;
    case XK_F20:
      return KEY_F20;
    case XK_F21:
      return KEY_F21;
    case XK_F22:
      return KEY_F22;
    case XK_F23:
      return KEY_F23;
    case XK_F24:
      return KEY_F24;

    case XK_Num_Lock:
      return KEY_NUMLOCK;
    case XK_Scroll_Lock:
      return KEY_SCROLL;

    case XK_KP_Equal:
      return KEY_NUMPAD_EQUAL;

    case XK_Shift_L:
      return KEY_LSHIFT;
    case XK_Shift_R:
      return KEY_RSHIFT;
    case XK_Control_L:
      return KEY_LCONTROL;
    case XK_Control_R:
      return KEY_RCONTROL;
    // case XK_Menu: return KEY_LMENU;
    case XK_Menu:
      return KEY_RMENU;

    case XK_semicolon:
      return KEY_SEMICOLON;
    case XK_plus:
      return KEY_PLUS;
    case XK_comma:
      return KEY_COMMA;
    case XK_minus:
      return KEY_MINUS;
    case XK_period:
      return KEY_PERIOD;
    case XK_slash:
      return KEY_SLASH;
    case XK_grave:
      return KEY_GRAVE;

    case XK_a:
    case XK_A:
      return KEY_A;
    case XK_b:
    case XK_B:
      return KEY_B;
    case XK_c:
    case XK_C:
      return KEY_C;
    case XK_d:
    case XK_D:
      return KEY_D;
    case XK_e:
    case XK_E:
      return KEY_E;
    case XK_f:
    case XK_F:
      return KEY_F;
    case XK_g:
    case XK_G:
      return KEY_G;
    case XK_h:
    case XK_H:
      return KEY_H;
    case XK_i:
    case XK_I:
      return KEY_I;
    case XK_j:
    case XK_J:
      return KEY_J;
    case XK_k:
    case XK_K:
      return KEY_K;
    case XK_l:
    case XK_L:
      return KEY_L;
    case XK_m:
    case XK_M:
      return KEY_M;
    case XK_n:
    case XK_N:
      return KEY_N;
    case XK_o:
    case XK_O:
      return KEY_O;
    case XK_p:
    case XK_P:
      return KEY_P;
    case XK_q:
    case XK_Q:
      return KEY_Q;
    case XK_r:
    case XK_R:
      return KEY_R;
    case XK_s:
    case XK_S:
      return KEY_S;
    case XK_t:
    case XK_T:
      return KEY_T;
    case XK_u:
    case XK_U:
      return KEY_U;
    case XK_v:
    case XK_V:
      return KEY_V;
    case XK_w:
    case XK_W:
      return KEY_W;
    case XK_x:
    case XK_X:
      return KEY_X;
    case XK_y:
    case XK_Y:
      return KEY_Y;
    case XK_z:
    case XK_Z:
      return KEY_Z;
    default:
      return 0;
  }
}
#endif

void* platform_allocate(u64 size, b8 aligned) {
  return malloc(size);
}

void* platform_reallocate(void *block, u64 size, b8 aligned) {
  return realloc(block, size);
}

void platform_free(void* block, b8 aligned) {
  free(block);
}

void* platform_zero_memory(void* block, u64 size) {
  return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* source, u64 size) {
  return memcpy(dest, source, size);
}

void* platform_move_memory(void* dest, const void* source, u64 size) {
  return memmove(dest, source, size);
}

void* platform_set_memory(void* dest, i32 value, u64 size) {
  return memset(dest, value, size);
}

void platform_console_write(const char* message, u8 colour) {
  // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
  const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
  printf("\033[%sm%s\033[0m", colour_strings[colour], message);
}
void platform_console_write_error(const char* message, u8 colour) {
  // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
  const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
  printf("\033[%sm%s\033[0m", colour_strings[colour], message);
}

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>  // nanosleep
#else
#include <unistd.h>  // usleep
#endif

f64 platform_get_absolute_time() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec + now.tv_nsec * 0.000000001;
}

void platform_sleep(u64 ms) {
#if _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000 * 1000;
  nanosleep(&ts, 0);
#else
  if (ms >= 1000) {
      sleep(ms / 1000);
  }
  usleep((ms % 1000) * 1000);
#endif
}

#endif
