#ifndef WINDOW_H
#define WINDOW_H

// win32
//   msvc : user32.lib gdi32.lib opengl32.lib (shell32.lib)
//   mingw:

#ifdef WINDOW_VERBOSE
#  include <stdio.h>
#  define WINDOW_LOG(...) fprintf(stderr, __VA_ARGS__);
#else
#  define WINDOW_LOG(...)
#endif //WINDOW_VERBOSE

#include <stdbool.h>
#include <math.h>

#include <windows.h>
#include <GL/GL.h>

#ifndef WINDOW_DEF
#  define WINDOW_DEF static inline
#endif //WINDOW_DEF

#ifndef PI
#  define PI 3.141592653589793f
#endif //PI

#define WINDOW_DOUBLE_CLICK_TIME_MS 500

#define WINDOW_BACKSPACE 8
#define WINDOW_ESCAPE 27
#define WINDOW_SPACE 32

typedef enum{
  WINDOW_EVENT_NONE = 0,
  WINDOW_EVENT_KEYPRESS,
  WINDOW_EVENT_KEYRELEASE,
  WINDOW_EVENT_MOUSEPRESS,
  WINDOW_EVENT_MOUSERELEASE,
  WINDOW_EVENT_FILEDROP,
}Window_Event_Type;

typedef struct{
  MSG msg;
  Window_Event_Type type;
  union{
    char key;
    long long value;
  }as;
}Window_Event;

typedef struct{
  HDROP h_drop;
  char path[MAX_PATH];

  int count;
  int index;
}Window_Dragged_Files;

typedef struct{
  HWND hwnd;
  HDC dc;
  RECT rect;
  POINT point;
  LARGE_INTEGER performance_frequency;
  LARGE_INTEGER time;
  bool is_shift_down;
  
  double dt;
  int running;
  int width, height;
}Window;

#define WINDOW_RUNNING       0x1
#define WINDOW_NOT_RESIZABLE 0x2
#define WINDOW_DRAG_N_DROP   0x4
#define WINDOW_FULLSCREEN    0x8


WINDOW_DEF bool window_init(Window *w, int width, int height, const char *title, int flags);
WINDOW_DEF bool window_set_vsync(Window *w, bool use_vsync);
WINDOW_DEF bool window_peek(Window *w, Window_Event *event);
WINDOW_DEF bool window_get_mouse_position(Window *w, int *width, int *height);
WINDOW_DEF void window_swap_buffers(Window *w);
WINDOW_DEF bool window_toggle_fullscreen(Window *w);
WINDOW_DEF void window_free(Window *w);
WINDOW_DEF bool window_show_cursor(Window *w, bool show);

WINDOW_DEF bool window_dragged_files_init(Window_Dragged_Files *files, Window_Event *event);
WINDOW_DEF bool window_dragged_files_next(Window_Dragged_Files *files, char **path);
WINDOW_DEF void window_dragged_files_free(Window_Dragged_Files *files);

WINDOW_DEF bool window_compile_shader(GLuint *shader, GLenum shader_type, const char *shader_source);
WINDOW_DEF bool window_link_program(GLuint *program, GLuint vertex_shader, GLuint fragment_shader);

#ifndef WINDOW_NO_RENDERER

typedef struct{
  float x, y;
}Window_Renderer_Vec2f;

WINDOW_DEF Window_Renderer_Vec2f window_renderer_vec2f(float x, float y);

typedef struct{
  float x, y, z, w;
}Window_Renderer_Vec4f;

WINDOW_DEF Window_Renderer_Vec4f window_renderer_vec4f(float x, float y, float z, float w);

typedef struct{
  Window_Renderer_Vec2f position;
  Window_Renderer_Vec4f color;
  Window_Renderer_Vec2f uv;
}Window_Renderer_Vertex;

#define WINDOW_RENDERER_VERTEX_ATTR_POSITION 0
#define WINDOW_RENDERER_VERTEX_ATTR_COLOR 1
#define WINDOW_RENDERER_VERTEX_ATTR_UV 2

#define WINDOW_RENDERER_CAP 1024

typedef struct{
  GLuint vao, vbo;
  GLuint vertex_shader, fragment_shader;
  GLuint program;
  
  GLuint textures;
  unsigned int images_count;

#ifdef __STB_INCLUDE_STB_TRUETYPE_H__
  stbtt_bakedchar font_cdata[96]; // ASCII 32..126 is 95 glyphs
#endif //__STB_INCLUDE_STB_TRUETYPE_H__
    
  int font_index;    
  int tex_index;

  float width, height;
  Window_Renderer_Vec4f background;

  Window_Renderer_Vertex verticies[WINDOW_RENDERER_CAP];
  int verticies_count;
}Window_Renderer;

static Window_Renderer_Vec4f WHITE = {1, 1, 1, 1};
static Window_Renderer_Vec4f RED   = {1, 0, 0, 1};
static Window_Renderer_Vec4f BLUE  = {0, 0, 1, 1};
static Window_Renderer_Vec4f GREEN = {0, 1, 0, 1};
static Window_Renderer_Vec4f BLACK = {0, 0, 0, 1};

#define Vec2f Window_Renderer_Vec2f
#define vec2f(x, y) window_renderer_vec2f((x), (y))
#define Vec4f Window_Renderer_Vec4f
#define vec4f(x, y, z, w) window_renderer_vec4f((x), (y), (z), (w))
#define draw_triangle window_renderer_triangle
#define draw_solid_triangle window_renderer_solid_triangle
#define draw_solid_rect window_renderer_solid_rect
#define draw_solid_rounded_rect window_renderer_solid_rounded_rect
#define draw_solid_rounded_shaded_rect window_renderer_solid_rounded_shaded_rect
#define draw_solid_rect_angle window_renderer_solid_rect_angle
#define push_texture window_renderer_push_texture
#define draw_texture window_renderer_texture
#define draw_texture_colored window_renderer_texture_colored
#define draw_solid_circle window_renderer_solid_circle

#ifdef __STB_INCLUDE_STB_TRUETYPE_H__
#  define push_font window_renderer_push_font
#  define draw_text(cstr, pos, factor) window_renderer_text((cstr), strlen((cstr)), (pos), (factor), (WHITE))
#  define draw_text_colored(cstr, pos, factor, color) window_renderer_text((cstr), strlen((cstr)), (pos), (factor), (color))
#  define draw_text_len(cstr, cstr_len, pos, factor) window_renderer_text((cstr), (cstr_len), (pos), (factor), (WHITE))
#  define draw_text_len_colored(cstr, cstr_len, pos, factor, color) window_renderer_text((cstr), (cstr_len), (pos), (factor), (color))

#  define measure_text(cstr, factor, size) window_renderer_measure_text((cstr), strlen((cstr)), (factor), (size));
#  define measure_text_len(cstr, cstr_len, factor, size) window_renderer_measure_text((cstr), (cstr_len), (factor), (size));
#endif //__STB_INCLUDE_STB_TRUETYPE_H__

WINDOW_DEF bool window_renderer_init(Window_Renderer *r);
WINDOW_DEF void window_renderer_begin(int width, int height);
WINDOW_DEF void window_renderer_set_color(Window_Renderer_Vec4f color);
WINDOW_DEF void window_renderer_vertex(Window_Renderer_Vec2f p, Window_Renderer_Vec4f c, Window_Renderer_Vec2f uv);
WINDOW_DEF void window_renderer_triangle(Window_Renderer_Vec2f p1, Window_Renderer_Vec2f p2, Window_Renderer_Vec2f p3, Window_Renderer_Vec4f c1, Window_Renderer_Vec4f c2, Window_Renderer_Vec4f c3, Window_Renderer_Vec2f uv1, Window_Renderer_Vec2f uv2, Window_Renderer_Vec2f uv3);
WINDOW_DEF void window_renderer_solid_triangle(Window_Renderer_Vec2f p1, Window_Renderer_Vec2f p2, Window_Renderer_Vec2f p3, Window_Renderer_Vec4f c);
WINDOW_DEF void window_renderer_quad(Window_Renderer_Vec2f p1, Window_Renderer_Vec2f p2, Window_Renderer_Vec2f p3, Window_Renderer_Vec2f p4,Window_Renderer_Vec4f c1,Window_Renderer_Vec4f c2,Window_Renderer_Vec4f c3,Window_Renderer_Vec4f c4, Window_Renderer_Vec2f uv1, Window_Renderer_Vec2f uv2, Window_Renderer_Vec2f uv3, Window_Renderer_Vec2f uv4);
WINDOW_DEF void window_renderer_solid_rect(Window_Renderer_Vec2f pos, Window_Renderer_Vec2f size,Window_Renderer_Vec4f color);
WINDOW_DEF void window_renderer_solid_rounded_rect(Vec2f pos, Vec2f size, float radius, int parts, Vec4f color);
WINDOW_DEF void window_renderer_solid_rounded_shaded_rect(Window_Renderer_Vec2f pos, Window_Renderer_Vec2f size, float radius, int parts, float shade_px, Window_Renderer_Vec4f color);
WINDOW_DEF void window_renderer_solid_rect_angle(Window_Renderer_Vec2f pos, Window_Renderer_Vec2f size, float angle, Window_Renderer_Vec4f color);
WINDOW_DEF bool window_renderer_push_texture(int width, int height, const void *data, bool grey, unsigned int *index);
WINDOW_DEF void window_renderer_texture(unsigned int texture, Window_Renderer_Vec2f p, Window_Renderer_Vec2f s, Window_Renderer_Vec2f uvp, Window_Renderer_Vec2f uvs);
WINDOW_DEF void window_renderer_texture_colored(unsigned int texture, Window_Renderer_Vec2f p, Window_Renderer_Vec2f s, Window_Renderer_Vec2f uvp, Window_Renderer_Vec2f uvs, Window_Renderer_Vec4f c);
WINDOW_DEF void window_renderer_solid_circle(Window_Renderer_Vec2f pos, float start_angle, float end_angle, float radius, int parts, Window_Renderer_Vec4f color);
WINDOW_DEF void window_renderer_end();
WINDOW_DEF void window_renderer_free();

#ifdef __STB_INCLUDE_STB_TRUETYPE_H__
WINDOW_DEF bool window_renderer_push_font(const char *filepath, float pixel_height);
WINDOW_DEF void window_renderer_measure_text(const char *cstr, size_t cstr_len, float scale, Vec2f *size);
WINDOW_DEF void window_renderer_text(const char *cstr, size_t cstr_len, Window_Renderer_Vec2f pos, float scale, Window_Renderer_Vec4f color);
#endif //__STB_INCLUDE_STB_TRUETYPE_H__

#endif //WINDOW_NO_RENDERER


// opengl - functions / types / definitions
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5

#define GL_ARRAY_BUFFER 0x8892
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_ACTIVE_UNIFORMS 0x8B86

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_RGB_INTEGER 0x8D98
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define GL_MULTISAMPLE  0x809D
#define GL_SAMPLES 0x80A9

#define GL_BGRA 0x80E1
#define GL_RGB 0x1907
#define GL_BGR 0x80E0

#define GL_MULTISAMPLE 0x809D
#define GL_MULTISAMPLE_ARB 0x809D
#define GL_MULTISAMPLE_BIT 0x20000000
#define GL_MULTISAMPLE_BIT_ARB 0x20000000
#define GL_MULTISAMPLE_FILTER_HINT_NV 0x8534
#define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#define GL_SAMPLE_BUFFERS 0x80A8
#define GL_SAMPLES 0x80A9

typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
typedef char GLchar;

void glActiveTexture(GLenum texture);
void glGenVertexArrays(GLsizei n, GLuint *arrays);
void glBindVertexArray(GLuint array);
void glGenBuffers(GLsizei n, GLuint *buffers);
void glBindBuffer(GLenum target, GLuint buffer);
void glBufferData(GLenum target, GLsizeiptr size, const void * data, GLenum usage);
void glEnableVertexAttribArray(GLuint index);
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
GLuint glCreateShader(GLenum shaderType);
void glShaderSource(GLuint shader, GLsizei count, const GLchar **_string, const GLint *length);
void glCompileShader(GLuint shader);
void glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
void glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
GLuint glCreateProgram(void);
void glAttachShader(GLuint program, GLuint shader);
void glLinkProgram(GLuint program);
void glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
void glGetProgramiv(GLuint program, GLenum pname, GLint *params);
void glUseProgram(GLuint program);
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void * data);
void glUniform2f(GLint location, GLfloat v0, GLfloat v1);
void glUniform1f(GLint location, GLfloat v0);
void glUniform1fv(GLint location, GLsizei count, const GLfloat *value);
void glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
void glUniform1i(GLint location, GLint v0);
GLint glGetUniformLocation(GLuint program, const GLchar *name);
void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
void glGetUniformfv(GLuint program, GLint location, GLfloat *params);
void glGetUniformiv(GLuint program, GLint location, GLsizei bufSize, GLint *params);
void glSampleCoverage(GLfloat value, GLboolean invert);
int wglSwapIntervalEXT(GLint interval);

#ifdef WINDOW_IMPLEMENTATION

#ifndef WINDOW_NO_RENDERER
static Window_Renderer window_renderer;
static bool window_renderer_inited = false;
#endif //WINDOW_NO_RENDERER

LRESULT CALLBACK Window_Implementation_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

  if(message == WM_CLOSE ||
     message == WM_DESTROY) {
    Window *w = (Window *) GetWindowLongPtr(hWnd, 0);
    if(w != NULL) {
      w->running = 0;
    }
    PostQuitMessage(0);
    return 0;
  } else {
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  
}

WINDOW_DEF void window_win32_opengl_init();

WINDOW_DEF bool window_init(Window *w, int width, int height, const char *title, int flags) {

  STARTUPINFO startupInfo;
  GetStartupInfo(&startupInfo);
  HMODULE hInstance = GetModuleHandle(NULL);
  DWORD nCmdShow = startupInfo.wShowWindow;

#if 1
  HINSTANCE user32Lib = LoadLibrary("user32.dll");
  if(!user32Lib) {
    return false;
  }

  PROC setProcessDPIAware = GetProcAddress(user32Lib, "SetProcessDPIAware");
  if(!setProcessDPIAware) {
    return false;
  }
  setProcessDPIAware();

  FreeLibrary(user32Lib);
#else
  SetProcessDPIAware();
#endif

  WNDCLASSEX wc = {0};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = Window_Implementation_WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = title;
  wc.cbWndExtra = sizeof(LONG_PTR);
  wc.cbClsExtra = 0;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  
  HICON icon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
  wc.hIcon = icon; // ICON when tabbing
  wc.hIconSm = icon; //ICON default
  
  if(!RegisterClassEx(&wc)) {
    return false;
  }

  int screen_width = GetSystemMetrics(SM_CXSCREEN);
  int screen_height = GetSystemMetrics(SM_CYSCREEN);

  // WS_THICKFRAME :: resizable
  // WS_MAXIMIZEBOX :: maximizable

#define WINDOW_STYLE (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define WINDOW_RESIZABLE_STYLE (WS_THICKFRAME | WS_MAXIMIZEBOX)

  DWORD style = WINDOW_STYLE;
  if(!(flags & WINDOW_NOT_RESIZABLE)) {
    style |= WINDOW_RESIZABLE_STYLE;
  } else {
    width -= 10;
    height -= 10;
  }

  //add space  
  width += 16;
  height += 39;

  DWORD window_flags = 0;
  if((flags & WINDOW_DRAG_N_DROP)) {
    window_flags |= WS_EX_ACCEPTFILES;
  }

  w->hwnd = CreateWindowEx(window_flags,
			   wc.lpszClassName,
			   wc.lpszClassName,
			   style,
			   screen_width / 2 - width/2,
			   screen_height / 2 - height/2,
			   width,
			   height,
			   NULL,
			   NULL,
			   hInstance,
			   NULL);
  if(!w->hwnd) {
    return false;
  }
  w->dc = GetDC(w->hwnd);

  //BEGIN opengl
  HDC w_dc = GetDC(w->hwnd);

  PIXELFORMATDESCRIPTOR desired_format = {0};
  desired_format.nSize = sizeof(desired_format);
  desired_format.nVersion = 1;
  desired_format.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
  desired_format.cColorBits = 32;
  desired_format.cAlphaBits = 8;

  int suggested_format_index = ChoosePixelFormat(w_dc, &desired_format);
  PIXELFORMATDESCRIPTOR suggested_format;
  DescribePixelFormat(w_dc, suggested_format_index, sizeof(suggested_format), &suggested_format);
  SetPixelFormat(w_dc, suggested_format_index, &suggested_format);
  
  HGLRC opengl_rc = wglCreateContext(w_dc);
  if(!wglMakeCurrent(w_dc, opengl_rc)) {
    return false;
  }
  ReleaseDC(w->hwnd, w_dc);
  //END opengl

  LONG_PTR lptr = {0};
  memcpy(&lptr, &w, sizeof(w));  
  SetWindowLongPtr(w->hwnd, 0, lptr);  

  ShowWindow(w->hwnd, nCmdShow);
  UpdateWindow(w->hwnd);

  w->running = WINDOW_RUNNING;
  w->width = width;
  w->height = height;
  w->is_shift_down = false;

  // load non-default-opengl-functions
  window_win32_opengl_init();

#ifndef WINDOW_NO_RENDERER
  if(!window_renderer_inited) {
    if(!window_renderer_init(&window_renderer)) {
      return false;	    
    }
	
    window_renderer_inited = true;
  }
#endif //WINDOW_NO_RENDERER

  // use vsync as default
  if(!window_set_vsync(w, true)) {
    return false;
  }

  if((flags & WINDOW_FULLSCREEN) && !window_toggle_fullscreen(w)) {
    return false;
  }

  QueryPerformanceFrequency(&w->performance_frequency);
  QueryPerformanceCounter(&w->time);
  w->dt = 0;
    
  return true;
}

WINDOW_DEF bool window_set_vsync(Window *w, bool use_vsync) {
  (void) w;
  return wglSwapIntervalEXT(use_vsync ? 1 : 0);
}

WINDOW_DEF bool window_peek(Window *w, Window_Event *e) {
    
  MSG *msg = &e->msg;

  while(true) {
    if(!PeekMessage(msg, w->hwnd, 0, 0, PM_REMOVE)) {
      break;
    }

    TranslateMessage(msg);
    DispatchMessage(msg);

    e->type = WINDOW_EVENT_NONE;

    switch(msg->message) {
    case WM_DROPFILES: {
      e->type = WINDOW_EVENT_FILEDROP;
      e->as.value = msg->wParam;
    } break;
    case WM_RBUTTONUP:  {
      e->type = WINDOW_EVENT_MOUSERELEASE;
      e->as.key = 'R';
      ReleaseCapture();
    } break;
    case WM_RBUTTONDOWN: {
      e->type = WINDOW_EVENT_MOUSEPRESS;
      e->as.key = 'R';
      SetCapture(w->hwnd);
    } break;
    case WM_LBUTTONUP: {
      e->type = WINDOW_EVENT_MOUSERELEASE;
      e->as.key = 'L';
      ReleaseCapture();
    } break;
    case WM_LBUTTONDOWN: {
      e->type = WINDOW_EVENT_MOUSEPRESS;
      e->as.key = 'L';
      SetCapture(w->hwnd);
    } break;
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP: {
      bool was_down = ((msg->lParam & (1 << 30)) != 0);
      bool is_down = ((msg->lParam & (1 << 31)) == 0);

      if(was_down != is_down) {

	if(msg->wParam == VK_SHIFT) {
	  if(was_down) {
	    w->is_shift_down = false;
	  } else {
	    w->is_shift_down = true;
	  }
	  continue;
	}


	char c = (char) msg->wParam;
	if(!w->is_shift_down && 'A' <= c && c <= 'Z') {
	  c += 32;
	}	
	e->as.key = c;
	if(was_down) {
	  e->type = WINDOW_EVENT_KEYRELEASE;
	} else {
	  e->type = WINDOW_EVENT_KEYPRESS;
	}

	
      }
      
    } break;
    default: {
    } break;
    }

    if(e->type != WINDOW_EVENT_NONE) {
      return true;
    }
  }

  // width, height
  if(!(w->running & WINDOW_FULLSCREEN)) {
    if(GetClientRect(w->hwnd, &w->rect)) {
      w->width = (w->rect.right - w->rect.left);
      w->height = (w->rect.bottom - w->rect.top);
    }
  }

  //dt
  LARGE_INTEGER time;
  QueryPerformanceCounter(&time);
  w->dt = ((double) time.QuadPart - (double) w->time.QuadPart)
    * 1000
    / (double) w->performance_frequency.QuadPart;
  w->time = time;


  return false;
}

WINDOW_DEF bool window_get_mouse_position(Window *w, int *width, int *height) {
  if(GetCursorPos(&w->point) && ScreenToClient(w->hwnd, &w->point)) {
    *width = w->point.x;
    *height = w->point.y;
    return true;
  }

  return false;
}
  
WINDOW_DEF void window_swap_buffers(Window *w) {   
  SwapBuffers(w->dc);
}

WINDOW_DEF bool window_toggle_fullscreen(Window *w) {

  DWORD style = (DWORD) GetWindowLongPtr(w->hwnd, GWL_STYLE);

  if(w->running & WINDOW_FULLSCREEN) {

    style &= ~WS_POPUP;
    style |= WINDOW_STYLE; // fix this
    if(!(w->running & WINDOW_NOT_RESIZABLE)) {
      style |= WINDOW_RESIZABLE_STYLE;
    }
    w->running &= ~WINDOW_FULLSCREEN;
    
    SetWindowPos(w->hwnd, NULL,
		 w->rect.left,
		 w->rect.top,
		 w->rect.right - w->rect.left,
		 w->rect.bottom - w->rect.top,
		 SWP_FRAMECHANGED);    
  } else {    
    if(!GetWindowRect(w->hwnd, &w->rect)) {
      return false;
    }

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    style &= ~WINDOW_STYLE;
    if(!(w->running & WINDOW_NOT_RESIZABLE)) {
      style &= ~WINDOW_RESIZABLE_STYLE;
    }
    style |= WS_POPUP;
    w->running |= WINDOW_FULLSCREEN;
    w->width = width;
    w->height = height;

    SetWindowPos(w->hwnd, HWND_TOP, 0, 0, width, height, SWP_FRAMECHANGED);
  }

  SetWindowLongPtr(w->hwnd, GWL_STYLE, style);

  return true;
}

WINDOW_DEF void window_free(Window *w) {	
  ReleaseDC(w->hwnd, w->dc);
  DestroyWindow(w->hwnd);
}

WINDOW_DEF bool window_show_cursor(Window *w, bool show) {
  (void) w;

  CURSORINFO info;
  info.cbSize = sizeof(CURSORINFO);
  if(!GetCursorInfo(&info)) {
    return false;
  }

  bool visible = info.flags;
  if(show && !visible) {
    ShowCursor(true);
  } else if(!show && visible){
    ShowCursor(false);
  }

  return true;
}

WINDOW_DEF bool window_dragged_files_init(Window_Dragged_Files *files, Window_Event *event) {
  files->h_drop = (HDROP) event->as.value;
  files->count = DragQueryFile(files->h_drop, 0xffffffff, files->path, MAX_PATH);
  if(files->count <= 0) {
    return false;
  }
  files->index = 0;

  return true;
}

WINDOW_DEF bool window_dragged_files_next(Window_Dragged_Files *files, char **path) {
  if(files->index >= files->count) {
    return false;
  }
  DragQueryFile(files->h_drop, files->index++, files->path, MAX_PATH);
  *path = files->path;

  return true;
}

WINDOW_DEF void window_dragged_files_free(Window_Dragged_Files *files) {
  DragFinish(files->h_drop);
}

WINDOW_DEF const char *window_shader_type_name(GLenum shader) {
  switch (shader) {
  case GL_VERTEX_SHADER:
    return "GL_VERTEX_SHADER";
  case GL_FRAGMENT_SHADER:
    return "GL_FRAGMENT_SHADER";
  default:
    return "(Unknown)";
  }
}

WINDOW_DEF bool window_compile_shader(GLuint *shader, GLenum shader_type, const char *shader_source) {
  *shader = glCreateShader(shader_type);
  glShaderSource(*shader, 1, &shader_source, NULL);
  glCompileShader(*shader);

  GLint compiled = 0;
  glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);

  if (!compiled) {
    GLchar message[1024];
    GLsizei message_size = 0;
    glGetShaderInfoLog(*shader, sizeof(message), &message_size, message);
    WINDOW_LOG("ERROR: could not compile %s\n", window_shader_type_name(shader_type));
    WINDOW_LOG("%.*s\n", message_size, message);
    return false;
  }

  return true;
}

WINDOW_DEF bool window_link_program(GLuint *program, GLuint vertex_shader, GLuint fragment_shader) {
  *program = glCreateProgram();
  glAttachShader(*program, vertex_shader);
  glAttachShader(*program, fragment_shader);

  glLinkProgram(*program);
  
  GLint linked = 0;
  glGetProgramiv(*program, GL_LINK_STATUS, &linked);
  if(!linked) {
    GLsizei message_size = 0;
    GLchar message[1024];

    glGetProgramInfoLog(*program, sizeof(message), &message_size, message);
    WINDOW_LOG("ERROR: Program Linking: %.*s\n", message_size, message);
    return false;
  }
  
  return true;
    
}

////////////////////////////////////////////////////////////////////////
// renderer - definitions
////////////////////////////////////////////////////////////////////////

#ifndef WINDOW_NO_RENDERER

static const char* window_renderer_vertex_shader_source =
  "#version 330 core\n"
  "\n"
  "layout(location = 0) in vec2 position;\n"
  "layout(location = 1) in vec4 color;\n"
  "layout(location = 2) in vec2 uv;\n"
  "\n"
  "uniform float resolution_x;\n"
  "uniform float resolution_y;\n"
  "\n"
  "out vec4 out_color;\n"
  "out vec2 out_uv;\n"
  "\n"
  "vec2 resolution_project(vec2 point) {\n"
  "    return 2 * point / vec2(resolution_x, resolution_y) - 1;\n"
  "}\n"
  "\n"
  "void main() {\n"
  "  out_color = color;\n"
  "  out_uv = uv;\n"
  "  gl_Position = vec4(resolution_project(position), 0, 1);\n"
  "}";

static const char *window_renderer_fragment_shader_source=
  "#version 330 core\n"
  "\n"
  "uniform sampler2D font_tex;\n"
  "uniform sampler2D tex;\n"
  "\n"
  "in vec4 out_color;\n"
  "in vec2 out_uv;\n"
  "\n"
  "out vec4 fragColor;\n"
  "\n"
  "void main() {\n"
  "    if(out_uv.x < 0 && out_uv.y < 0) {\n"
  "        fragColor = out_color;\n"
  "    } else if(out_color.w < 0) {\n"
  "        vec4 color = texture(font_tex, vec2(out_uv.x, 1-out_uv.y));\n"
  "        float a = color.w * -out_color.w;\n"
  "        color = (color + vec4(1, 1, 1, 0)) * out_color;\n"
  "        color.w = a;\n"
  "        fragColor = color;\n"
  "    } else {\n"
  "        vec4 color = texture(tex, vec2(out_uv.x, 1-out_uv.y));\n"
  "        color = color * out_color;\n"
  "        fragColor = color;\n"
  "    }\n"
  "}\n";

WINDOW_DEF Window_Renderer_Vec2f window_renderer_vec2f(float x, float y) {
  return (Window_Renderer_Vec2f) { x, y};
}

WINDOW_DEF Window_Renderer_Vec4f window_renderer_vec4f(float x, float y, float z, float w) {
  return (Window_Renderer_Vec4f) { x, y, z, w};
}

WINDOW_DEF bool window_renderer_init(Window_Renderer *r) {
  (void) WHITE;
  (void) RED;
  (void) BLUE;
  (void) GREEN;
  (void) BLACK;
    
  // introduce 'verticies' to opengl
  glGenVertexArrays(1, &r->vao);
  glBindVertexArray(r->vao);

  glGenBuffers(1, &r->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(r->verticies), r->verticies, GL_DYNAMIC_DRAW);

  // introduce 'Vertex' to opengl
  glEnableVertexAttribArray(WINDOW_RENDERER_VERTEX_ATTR_POSITION);
  glVertexAttribPointer(WINDOW_RENDERER_VERTEX_ATTR_POSITION,
			sizeof(Window_Renderer_Vec2f)/sizeof(float),
			GL_FLOAT,
			GL_FALSE,
			sizeof(Window_Renderer_Vertex),
			(GLvoid *) offsetof(Window_Renderer_Vertex, position));

  glEnableVertexAttribArray(WINDOW_RENDERER_VERTEX_ATTR_COLOR);
  glVertexAttribPointer(WINDOW_RENDERER_VERTEX_ATTR_COLOR,
			sizeof(Window_Renderer_Vec4f)/sizeof(float),
			GL_FLOAT,
			GL_FALSE,
			sizeof(Window_Renderer_Vertex),
			(GLvoid *) offsetof(Window_Renderer_Vertex, color));

  glEnableVertexAttribArray(WINDOW_RENDERER_VERTEX_ATTR_UV);
  glVertexAttribPointer(WINDOW_RENDERER_VERTEX_ATTR_UV,
			sizeof(Window_Renderer_Vec2f)/sizeof(float),
			GL_FLOAT,
			GL_FALSE,
			sizeof(Window_Renderer_Vertex),
			(GLvoid *) offsetof(Window_Renderer_Vertex, uv));
  
  // compile shaders
  if(!window_compile_shader(&r->vertex_shader, GL_VERTEX_SHADER, window_renderer_vertex_shader_source)) {
    return false;
  }
    
  if(!window_compile_shader(&r->fragment_shader, GL_FRAGMENT_SHADER, window_renderer_fragment_shader_source)) {
    return false;
  }

  // link program
  if(!window_link_program(&r->program, r->vertex_shader, r->fragment_shader)) {
    return false;
  }
  glUseProgram(r->program);


  r->images_count = 0;
  r->verticies_count = 0;
  r->font_index = -1;
  
  return true;
}

WINDOW_DEF void window_renderer_begin(int width, int height) {

  Window_Renderer *r = &window_renderer;

  if(width > 0 && height > 0) {

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    Window_Renderer_Vec4f *c = &r->background;
    glClearColor(c->x, c->y, c->z, c->w);

    // tell vertex shader what is the resolution is
    glUniform1fv(glGetUniformLocation(r->program, "resolution_x"), 1, &r->width);
    glUniform1fv(glGetUniformLocation(r->program, "resolution_y"), 1, &r->height);

    r->width = (float) width;
    r->height = (float) height;
  }


  if(r->font_index > 0) {
    GLint uniformLocation1 = glGetUniformLocation(r->program, "font_tex");
    glUniform1i(uniformLocation1, r->font_index);	
  }

  r->tex_index = -1;
}

WINDOW_DEF void window_renderer_set_color(Window_Renderer_Vec4f color) {
  Window_Renderer *r = &window_renderer;
  r->background = color;
}

WINDOW_DEF void window_renderer_vertex(Window_Renderer_Vec2f p, Window_Renderer_Vec4f c, Window_Renderer_Vec2f uv) {

  Window_Renderer *r = &window_renderer;

  if(r->verticies_count < WINDOW_RENDERER_CAP) {
    Window_Renderer_Vertex *last = &r->verticies[r->verticies_count];
    last->position = p;
    last->color = c;
    last->uv = uv;
    r->verticies_count++;	
  } 
}

WINDOW_DEF void window_renderer_triangle(Window_Renderer_Vec2f p1, Window_Renderer_Vec2f p2, Window_Renderer_Vec2f p3, Window_Renderer_Vec4f c1, Window_Renderer_Vec4f c2, Window_Renderer_Vec4f c3, Window_Renderer_Vec2f uv1, Window_Renderer_Vec2f uv2, Window_Renderer_Vec2f uv3) {

  Window_Renderer *r = &window_renderer;

  if(r->verticies_count + 3 >= WINDOW_RENDERER_CAP) {
    window_renderer_end();
  }
    
  window_renderer_vertex(p1, c1, uv1);
  window_renderer_vertex(p2, c2, uv2);
  window_renderer_vertex(p3, c3, uv3);	
}

WINDOW_DEF void window_renderer_solid_triangle(Window_Renderer_Vec2f p1, Window_Renderer_Vec2f p2, Window_Renderer_Vec2f p3, Window_Renderer_Vec4f c) {

  Window_Renderer *r = &window_renderer;

  if(r->verticies_count + 3 >= WINDOW_RENDERER_CAP) {
    window_renderer_end();
  }
	
  Window_Renderer_Vec2f uv = window_renderer_vec2f(-1, -1);
  window_renderer_vertex(p1, c, uv);
  window_renderer_vertex(p2, c, uv);
  window_renderer_vertex(p3, c, uv);
}

WINDOW_DEF void window_renderer_quad(Window_Renderer_Vec2f p1, Window_Renderer_Vec2f p2, Window_Renderer_Vec2f p3, Window_Renderer_Vec2f p4, Window_Renderer_Vec4f c1, Window_Renderer_Vec4f c2, Window_Renderer_Vec4f c3, Window_Renderer_Vec4f c4, Window_Renderer_Vec2f uv1, Window_Renderer_Vec2f uv2, Window_Renderer_Vec2f uv3, Window_Renderer_Vec2f uv4) {
  window_renderer_triangle(p1, p2, p4, c1, c2, c4, uv1, uv2, uv4);
  window_renderer_triangle(p1, p3, p4, c1, c3, c4, uv1, uv3, uv4);
}

WINDOW_DEF void window_renderer_solid_rect(Window_Renderer_Vec2f pos, Window_Renderer_Vec2f size, Window_Renderer_Vec4f color) {
  Vec2f uv = vec2f(-1, -1);
  window_renderer_quad(pos,
		       window_renderer_vec2f(pos.x + size.x, pos.y),
		       window_renderer_vec2f(pos.x, pos.y + size.y),
		       window_renderer_vec2f(pos.x + size.x, pos.y + size.y),
		       color, color, color, color, uv, uv, uv, uv);
}

WINDOW_DEF void window_renderer_solid_rounded_rect(Vec2f pos, Vec2f size, float radius, int parts, Vec4f color) {
  if(size.y < 4 * radius) radius = size.y / 4; 

  window_renderer_solid_rect(vec2f(pos.x, pos.y + radius),
		  vec2f(size.x, size.y - 2 * radius),
		  color);

  window_renderer_solid_rect(vec2f(pos.x + radius, pos.y),
		  vec2f(size.x - 2 * radius, size.y),
		  color);

  pos = vec2f(pos.x + radius, pos.y + radius);
  size = vec2f(size.x - 2 * radius, size.y - 2 * radius);

  //bottom left
  window_renderer_solid_circle(pos,
		    PI, PI * 3 / 2,
		    radius,
		    parts,
		    color);

  //bottom right
  window_renderer_solid_circle(vec2f(pos.x + size.x, pos.y),
		    PI * 3 / 2, 2 * PI,
		    radius,
		    parts,
		    color);

  //top left
  window_renderer_solid_circle(vec2f(pos.x, pos.y + size.y),
		    PI / 2, PI,
		    radius,
		    parts,
		    color);

  //top right
  window_renderer_solid_circle(vec2f(pos.x + size.x, pos.y + size.y),
		    0, PI /2,
		    radius,
		    parts,
		    color);  
}

WINDOW_DEF void window_renderer_solid_rounded_shaded_rect(Window_Renderer_Vec2f pos,
							  Window_Renderer_Vec2f size,
							  float radius,
							  int parts,
							  float shade_px,
							  Window_Renderer_Vec4f color) {
  window_renderer_solid_rounded_rect(vec2f(pos.x - shade_px, pos.y - shade_px),
				     vec2f(size.x + 2 *shade_px, size.y + 2 *shade_px),
				     radius, parts, vec4f(0, 0, 0, color.w * .5));
  window_renderer_solid_rounded_rect(pos, size, radius, parts, color);
}

WINDOW_DEF void window_renderer_solid_rect_angle(Window_Renderer_Vec2f pos, Window_Renderer_Vec2f size,
						 float angle,
						 Window_Renderer_Vec4f color) {
  Vec2f uv = vec2f(-1, -1);

  float s = sinf(angle);
  float c = cosf(angle);

  float dy1 = s * size.x;
  float dx1 = c * size.x;
  
  float dy2 = c * size.y;
  float dx2 = s * size.y;

  window_renderer_triangle(pos,
			   window_renderer_vec2f(pos.x + dx1, pos.y + dy1),
			   window_renderer_vec2f(pos.x - dx2, pos.y + dy2),
			   color, color, color, uv, uv, uv);

  window_renderer_triangle(window_renderer_vec2f(pos.x + dx1, pos.y + dy1),
			   window_renderer_vec2f(pos.x - dx2, pos.y + dy2),
			   window_renderer_vec2f(pos.x + dx1 - dx2, pos.y + dy1 + dy2),
			   color, color, color, uv, uv, uv);
}

WINDOW_DEF void window_renderer_texture(unsigned int texture,
					Window_Renderer_Vec2f p, Window_Renderer_Vec2f s,
					Window_Renderer_Vec2f uvp, Window_Renderer_Vec2f uvs) {

  Window_Renderer *r = &window_renderer;

  if(r->tex_index != -1) {
    window_renderer_end();
  }

  r->tex_index = (int) texture;
  GLint uniformLocation1 = glGetUniformLocation(r->program, "tex");
  glUniform1i(uniformLocation1, r->tex_index);
    
  Vec4f c = vec4f(1, 1, 1, 1);
  window_renderer_quad(p,
		       window_renderer_vec2f(p.x + s.x, p.y),
		       window_renderer_vec2f(p.x, p.y + s.y),
		       window_renderer_vec2f(p.x + s.x, p.y + s.y),
		       c, c, c, c,
		       uvp,
		       window_renderer_vec2f(uvp.x + uvs.x, uvp.y),
		       window_renderer_vec2f(uvp.x, uvp.y + uvs.y),
		       window_renderer_vec2f(uvp.x + uvs.x, uvp.y + uvs.y));
}

WINDOW_DEF void window_renderer_texture_colored(unsigned int texture,
						Window_Renderer_Vec2f p, Window_Renderer_Vec2f s,
					        Window_Renderer_Vec2f uvp, Window_Renderer_Vec2f uvs,
						Window_Renderer_Vec4f c) {
  Window_Renderer *r = &window_renderer;
  
  if(r->tex_index != -1) {
    window_renderer_end();
  }

  r->tex_index = (int) texture;
  GLint uniformLocation1 = glGetUniformLocation(r->program, "tex");
  glUniform1i(uniformLocation1, r->tex_index);
  
  window_renderer_quad(
		       p,
		       window_renderer_vec2f(p.x + s.x, p.y),
		       window_renderer_vec2f(p.x, p.y + s.y),
		       window_renderer_vec2f(p.x + s.x, p.y + s.y),
		       c, c, c, c,
		       uvp,
		       window_renderer_vec2f(uvp.x + uvs.x, uvp.y),
		       window_renderer_vec2f(uvp.x, uvp.y + uvs.y),
		       window_renderer_vec2f(uvp.x + uvs.x, uvp.y + uvs.y));
}


WINDOW_DEF void window_renderer_solid_circle(Window_Renderer_Vec2f pos,
					     float start_angle, float end_angle,
					     float radius,
					     int parts,
					     Window_Renderer_Vec4f color) {
  float P = end_angle - start_angle;
  float A = start_angle;

  Window_Renderer_Vec2f old = {radius * cosf(A),
			       radius * sinf(A)};

  float t = 0.0f;
  float dt = 1.f / (float) parts;

  for(int j=1;j<=parts;j++) {
    t += dt;
    Window_Renderer_Vec2f new = {radius * cosf(A + P * t),
				 radius * sinf(A + P * t)};
    window_renderer_solid_triangle(pos,
				   window_renderer_vec2f(pos.x + new.x, pos.y + new.y),
				   window_renderer_vec2f(pos.x + old.x, pos.y + old.y),
				   color);
    old = new;
  }

  
}

WINDOW_DEF bool window_renderer_push_texture(int width, int height, const void *data, bool grey, unsigned int *index) {

  Window_Renderer *r = &window_renderer;

  GLenum current_texture;
  switch(r->images_count) {
  case 0:
    current_texture = GL_TEXTURE0;
    break;
  case 1:
    current_texture = GL_TEXTURE1;
    break;
  case 2:
    current_texture = GL_TEXTURE2;
    break;
  case 3:
    current_texture = GL_TEXTURE3;
    break;
  default:
    return false;
  }
  
  glActiveTexture(current_texture);
  
  glGenTextures(1, &r->textures);
  glBindTexture(GL_TEXTURE_2D, r->textures);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  if(grey) {
    glTexImage2D(GL_TEXTURE_2D,
		 0,
		 GL_ALPHA,
		 width,
		 height,
		 0,
		 GL_ALPHA,
		 GL_UNSIGNED_BYTE,
		 data);	
  } else {
    glTexImage2D(GL_TEXTURE_2D,
		 0,
		 GL_RGBA,
		 width,
		 height,
		 0,
		 GL_RGBA,
		 GL_UNSIGNED_INT_8_8_8_8_REV,
		 data);
  }

  *index = r->images_count++;

  return true;
}

WINDOW_DEF void window_renderer_end() {

  Window_Renderer *r = &window_renderer;
  
  glBufferSubData(GL_ARRAY_BUFFER, 0, r->verticies_count * sizeof(Window_Renderer_Vertex), r->verticies);
  glDrawArrays(GL_TRIANGLES, 0, r->verticies_count);
  r->verticies_count = 0;
}

WINDOW_DEF void window_renderer_free(Window_Renderer *r) {
  (void) r;
}

#ifdef __STB_INCLUDE_STB_TRUETYPE_H__
#include <stdio.h>

#define WINDOW_RENDERER_STB_TEMP_BITMAP_SIZE 1024

WINDOW_DEF bool window_renderer_push_font(const char *filepath, float pixel_height) {


  FILE *f = fopen(filepath, "rb");
  if(!f) {
    WINDOW_LOG("ERROR: Can not open file: %s\n", filepath);
    return false;
  }

  if(fseek(f, 0, SEEK_END) < 0) {
    WINDOW_LOG("ERROR: Can not seek in file: %s\n", filepath);
    fclose(f);
    return false;
  }

  long m = ftell(f);
  if(m < 0) {
    WINDOW_LOG("ERROR: Can not read file(ftell): %s\n", filepath);
    fclose(f);
    return false;
  }  

  if(fseek(f, 0, SEEK_SET) < 0) {
    WINDOW_LOG("ERROR: Can not seek in file: %s\n", filepath);
    fclose(f);
    return false;
  }

  unsigned char *buffer = (unsigned char *) malloc((size_t) m);
  if(!buffer) {
    WINDOW_LOG("ERROR: Can not allocate enough memory\n");
    fclose(f);
    return false;
  }

  size_t _m = (size_t) m;
  size_t n = fread(buffer, 1, _m, f);
  if(n != _m) {
    WINDOW_LOG("ERROR: Failed to read file(fread): %s\n", filepath);
    free(buffer);
    fclose(f);
    return false;
  }
    
  fclose(f);

  unsigned char *temp_bitmap = malloc(WINDOW_RENDERER_STB_TEMP_BITMAP_SIZE *
				      WINDOW_RENDERER_STB_TEMP_BITMAP_SIZE);
  
  Window_Renderer *r = &window_renderer;
  stbtt_BakeFontBitmap(buffer,0, pixel_height,
		       temp_bitmap,
		       WINDOW_RENDERER_STB_TEMP_BITMAP_SIZE,
		       WINDOW_RENDERER_STB_TEMP_BITMAP_SIZE,
		       32, 96, r->font_cdata);

  unsigned int tex;
  bool result = push_texture(1024, 1024, temp_bitmap, true, &tex);

  r->font_index = (int) tex;

  free(buffer);
  free(temp_bitmap);
    
  return result;
}

WINDOW_DEF void window_renderer_text(const char *cstr, size_t cstr_len, Window_Renderer_Vec2f pos, float factor, Window_Renderer_Vec4f color) {

  Window_Renderer *r = &window_renderer;

  float x = 0;
  float y = 0;
  color.w *= -1;
    
  for(size_t i=0;i<cstr_len;i++) {
    unsigned char c = cstr[i];
    if (c < 32 && c >= 128) {
      continue;
    }

    float _y = y;

    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(r->font_cdata,
		       WINDOW_RENDERER_STB_TEMP_BITMAP_SIZE,
		       WINDOW_RENDERER_STB_TEMP_BITMAP_SIZE, c-32, &x, &y, &q,1);
    //1=opengl & d3d10+,0=d3d9

    Window_Renderer_Vec2f p = vec2f(pos.x + q.x0 * factor,pos.y + y + _y + factor * (y - q.y1) );
    Window_Renderer_Vec2f s = vec2f((q.x1 - q.x0) * factor, (q.y1 - q.y0) * factor);
    Window_Renderer_Vec2f uvp = vec2f(q.s0, 1 - q.t1);
    Window_Renderer_Vec2f uvs = vec2f(q.s1 - q.s0, q.t1 - q.t0);	

	
    window_renderer_quad(
			 p,
			 window_renderer_vec2f(p.x + s.x, p.y),
			 window_renderer_vec2f(p.x, p.y + s.y),
			 window_renderer_vec2f(p.x + s.x, p.y + s.y),
			 color, color, color, color,
			 uvp,
			 window_renderer_vec2f(uvp.x + uvs.x, uvp.y),
			 window_renderer_vec2f(uvp.x, uvp.y + uvs.y),
			 window_renderer_vec2f(uvp.x + uvs.x, uvp.y + uvs.y));       
  }

}

WINDOW_DEF void window_renderer_measure_text(const char *cstr, size_t cstr_len, float factor, Vec2f *size) {
  Window_Renderer *r = &window_renderer;

  size->y = 0;
  size->x = 0;

  float x = 0;
  float y = 0;
    
  for(size_t i=0;i<cstr_len;i++) {
    unsigned char c = cstr[i];
    if (c < 32 && c >= 128) {
      continue;
    }

    float _y = y;

    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(r->font_cdata,
		       WINDOW_RENDERER_STB_TEMP_BITMAP_SIZE,
		       WINDOW_RENDERER_STB_TEMP_BITMAP_SIZE,
		       c-32, &x, &y, &q, 1);

    size->x = q.y1 - q.y0;
    float height = q.x1 - q.x0;
    if(height > size->y) size->y = height;
  }
  size->x = x * factor;
  size->y *= factor;
}

#endif //__STB_INCLUDE_STB_TRUETYPE_H__

#endif //WINDOW_NO_RENDERER

////////////////////////////////////////////////////////////////////////
// opengl - definitions
////////////////////////////////////////////////////////////////////////

PROC _glActiveTexture = NULL;
void glActiveTexture(GLenum texture) { _glActiveTexture(texture); }

PROC _glGenVertexArrays = NULL;
void glGenVertexArrays(GLsizei n, GLuint *arrays) { _glGenVertexArrays(n, arrays); }

PROC _glBindVertexArray = NULL;
void glBindVertexArray(GLuint array) { _glBindVertexArray(array); }

PROC _glGenBuffers = NULL;
void glGenBuffers(GLsizei n, GLuint *buffers) { _glGenBuffers(n, buffers); }

PROC _glBindBuffer = NULL;
void glBindBuffer(GLenum target, GLuint buffer) { _glBindBuffer(target, buffer); }

PROC _glBufferData = NULL;
void glBufferData(GLenum target, GLsizeiptr size, const void * data, GLenum usage) { _glBufferData(target, size, data, usage); }

PROC _glEnableVertexAttribArray = NULL;
void glEnableVertexAttribArray(GLuint index) { _glEnableVertexAttribArray(index); }

PROC _glVertexAttribPointer = NULL;
void glVertexAttribPointer(GLuint index,
			   GLint size,
			   GLenum type,
			   GLboolean normalized,
			   GLsizei stride,
			   const void * pointer) {
  _glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

PROC _glCreateShader = NULL;
GLuint glCreateShader(GLenum shaderType) { return (GLuint) _glCreateShader(shaderType); }

PROC _glShaderSource = NULL;
void glShaderSource(GLuint shader,
		    GLsizei count,
		    const GLchar **_string,
		    const GLint *length) {
  _glShaderSource(shader, count, _string, length);
}

PROC _glCompileShader = NULL;
void glCompileShader(GLuint shader) { _glCompileShader(shader); }

PROC _glGetShaderiv = NULL;
void glGetShaderiv(GLuint shader, GLenum pname, GLint *params) {
  _glGetShaderiv(shader, pname, params);
}

PROC _glGetShaderInfoLog = NULL;
void glGetShaderInfoLog(GLuint shader,
			GLsizei maxLength,
			GLsizei *length,
			GLchar *infoLog) {
  _glGetShaderInfoLog(shader, maxLength, length, infoLog);
}

PROC _glCreateProgram = NULL;
GLuint glCreateProgram(void) { return (GLuint) _glCreateProgram(); }

PROC _glAttachShader = NULL;
void glAttachShader(GLuint program, GLuint shader) { _glAttachShader(program, shader); }

PROC _glLinkProgram = NULL;
void glLinkProgram(GLuint program) { _glLinkProgram(program); }

PROC _glGetProgramInfoLog = NULL;
void glGetProgramInfoLog(GLuint program,
			 GLsizei maxLength,
			 GLsizei *length,
			 GLchar *infoLog) {
  _glGetProgramInfoLog(program, maxLength, length, infoLog);
}

PROC _glGetProgramiv = NULL;
void glGetProgramiv(GLuint program,
		    GLenum pname,
		    GLint *params) {
  _glGetProgramiv(program, pname, params);
}

PROC _glUseProgram = NULL;
void glUseProgram(GLuint program) { _glUseProgram(program); }

PROC _glBufferSubData = NULL;
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void * data) {
  _glBufferSubData(target, offset, size, data);
}

PROC _glUniform2f = NULL;
void glUniform2f(GLint location, GLfloat v0, GLfloat v1) {
  _glUniform2f(location, v0, v1);
}

PROC _glUniform1f = NULL;
void glUniform1f(GLint location, GLfloat v0) {
  _glUniform1f(location, v0);
}

PROC _glUniform1fv = NULL;
void glUniform1fv(GLint location, GLsizei count, const GLfloat *value) {
  _glUniform1fv(location, count, value);
}

PROC _glUniform2fv = NULL;
void glUniform2fv(GLint location, GLsizei count, const GLfloat *value) {
  _glUniform2fv(location, count, value);
}

PROC _glUniform1i = NULL;
void glUniform1i(GLint location, GLint v0) {
  _glUniform1i(location, v0);
}

PROC _glGetUniformLocation = NULL;
GLint glGetUniformLocation(GLuint program, const GLchar *name) { return (GLint) _glGetUniformLocation(program, name); }

PROC _glGetActiveUniform = NULL;
void glGetActiveUniform(GLuint program,
			GLuint index,
			GLsizei bufSize,
			GLsizei *length,
			GLint *size,
			GLenum *type,
			GLchar *name) {
  _glGetActiveUniform(program, index, bufSize, length, size, type, name);
}

PROC _glGetUniformfv = NULL;
void glGetUniformfv(GLuint program, GLint location, GLfloat *params) {
  _glGetUniformfv(program, location, params);
}

PROC _glGetUniformiv = NULL;
void glGetUniformiv(GLuint program, GLint location, GLsizei bufSize, GLint *params) {
  _glGetUniformiv(program, location, bufSize, params);
}

PROC _wglSwapIntervalEXT = NULL;
int wglSwapIntervalEXT(GLint interval) {
  return (int) _wglSwapIntervalEXT(interval);
}

PROC _glSampleCoverage = NULL;
void glSampleCoverage(GLfloat value, GLboolean invert) {
  _glSampleCoverage(value, invert);
}


WINDOW_DEF void window_win32_opengl_init() {
  if(_glActiveTexture != NULL) {
    return;
  }
  
  _glActiveTexture = wglGetProcAddress("glActiveTexture");
  _glGenVertexArrays = wglGetProcAddress("glGenVertexArrays");
  _glBindVertexArray= wglGetProcAddress("glBindVertexArray");
  _glGenBuffers= wglGetProcAddress("glGenBuffers");
  _glBindBuffer= wglGetProcAddress("glBindBuffer");
  _glBufferData= wglGetProcAddress("glBufferData");
  _glEnableVertexAttribArray= wglGetProcAddress("glEnableVertexAttribArray");
  _glVertexAttribPointer= wglGetProcAddress("glVertexAttribPointer");
  _glCreateShader= wglGetProcAddress("glCreateShader");
  _glShaderSource= wglGetProcAddress("glShaderSource");
  _glCompileShader= wglGetProcAddress("glCompileShader");
  _glGetShaderiv= wglGetProcAddress("glGetShaderiv");
  _glGetShaderInfoLog= wglGetProcAddress("glGetShaderInfoLog");
  _glCreateProgram= wglGetProcAddress("glCreateProgram");
  _glAttachShader= wglGetProcAddress("glAttachShader");
  _glLinkProgram= wglGetProcAddress("glLinkProgram");
  _glGetProgramInfoLog= wglGetProcAddress("glGetProgramInfoLog");
  _glGetProgramiv= wglGetProcAddress("glGetProgramiv");
  _glUseProgram= wglGetProcAddress("glUseProgram");
  _glBufferSubData= wglGetProcAddress("glBufferSubData");
  _glUniform2f= wglGetProcAddress("glUniform2f");
  _glUniform1f= wglGetProcAddress("glUniform1f");
  _glUniform1i= wglGetProcAddress("glUniform1i");
  _glGetUniformLocation= wglGetProcAddress("glGetUniformLocation");
  _glGetActiveUniform= wglGetProcAddress("glGetActiveUniform");
  _glGetUniformfv= wglGetProcAddress("glGetUniformfv");
  _glUniform1fv= wglGetProcAddress("glUniform1fv");
  _glUniform2fv= wglGetProcAddress("glUniform2fv");
  _glGetUniformiv= wglGetProcAddress("glGetUniformiv");
  _wglSwapIntervalEXT = wglGetProcAddress("wglSwapIntervalEXT");
}

#endif //WINDOW_IMPLEMENTATION

#endif //WINDOW_H
