sdl_cflags := $(shell pkg-config --cflags sdl2)
sdl_libs := $(shell pkg-config --libs sdl2)

opengl_libs := -lGL	-lGLEW

image_libs := -lSOIL

CFLAGS := -Wall	-g	$(sdl_cflags)
LDLIBS := $(sdl_libs)	$(opengl_libs)	$(image_libs)	-lm
