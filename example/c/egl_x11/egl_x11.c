// gcc example/c/egl_x11.c -Ibuild/include build/src/*.c -ldl -lX11

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <glad/egl.h>
//#include <glad/gles2.h>
#include <glad/gl.h>
#include "3dmath.h"

const int window_width = 800, window_height = 480;


int main(void) {
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        printf("cannot connect to X server\n");
        return 1;
    }

    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);
    Visual *visual = DefaultVisual(display, screen);

    Colormap colormap = XCreateColormap(display, root, visual, AllocNone);

    XSetWindowAttributes attributes;
    attributes.colormap = colormap;
    attributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask;

    Window window =
        XCreateWindow(display, root, 0, 0, window_width, window_height, 0,
                      DefaultDepth(display, screen), InputOutput, visual,
                      CWColormap | CWEventMask, &attributes);

    XFreeColormap(display, colormap);

    XMapWindow(display, window);
    XStoreName(display, window, "[glad] EGL with X11");

    if (!window) {
        printf("Unable to create window.\n");
        return 1;
    }

    int egl_version = gladLoaderLoadEGL(NULL);
    if (!egl_version) {
        printf("Unable to load EGL.\n");
        return 1;
    }
    printf("Loaded EGL %d.%d on first load.\n",
           GLAD_VERSION_MAJOR(egl_version), GLAD_VERSION_MINOR(egl_version));

    EGLDisplay egl_display = eglGetDisplay((EGLNativeDisplayType) display);
    if (egl_display == EGL_NO_DISPLAY) {
        printf("Got no EGL display.\n");
        return 1;
    }

    if (!eglInitialize(egl_display, NULL, NULL)) {
        printf("Unable to initialize EGL\n");
        return 1;
    }

    egl_version = gladLoaderLoadEGL(egl_display);
    if (!egl_version) {
        printf("Unable to reload EGL.\n");
        return 1;
    }
    printf("Loaded EGL %d.%d after reload.\n",
           GLAD_VERSION_MAJOR(egl_version), GLAD_VERSION_MINOR(egl_version));

    EGLint attr[] = {
        EGL_BUFFER_SIZE, 16,
        EGL_RENDERABLE_TYPE,
        // EGL_OPENGL_ES2_BIT,
        EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLConfig egl_config;
    EGLint num_config;
    if (!eglChooseConfig(egl_display, attr, &egl_config, 1, &num_config)) {
        printf("Failed to choose config (eglError: %d)\n", eglGetError());
        return 1;
    }

    if (num_config != 1) {
        printf("Didn't get exactly one config, but %d\n", num_config);
        return 1;
    }

    EGLSurface egl_surface =
        eglCreateWindowSurface(egl_display, egl_config, window, NULL);
    if (egl_surface == EGL_NO_SURFACE) {
        printf("Unable to create EGL surface (eglError: %d)\n",
               eglGetError());
        return 1;
    }

	EGLint ok;
    ok = eglBindAPI(EGL_OPENGL_API); 
    if (!ok) {
		printf("eglBindAPI: bind EGL Api failed (eglError: %d)\n",
               eglGetError());
        return 1;
	}

    EGLint ctxattr[] = {
        // EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_CONTEXT_MAJOR_VERSION_KHR, 2,
        EGL_CONTEXT_MINOR_VERSION_KHR, 1,
        EGL_NONE
    };
    EGLContext egl_context =
        eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, ctxattr);
    if (egl_context == EGL_NO_CONTEXT) {
        printf("Unable to create EGL context (eglError: %d)\n",
               eglGetError());
        return 1;
    }

    // activate context before loading GL functions using glad
    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

    int gl_version = gladLoaderLoadGL();
    if (!gl_version) {
        printf("Unable to load GL.\n");
        return 1;
    }
    printf("Loaded GL %d.%d\n", GLAD_VERSION_MAJOR(gl_version), GLAD_VERSION_MINOR(gl_version));

    // int gles_version = gladLoaderLoadGLES2();
    // if (!gles_version) {
    //     printf("Unable to load GLES.\n");
    //     return 1;
    // }
    // printf("Loaded GLES %d.%d.\n",
    //        GLAD_VERSION_MAJOR(gles_version), GLAD_VERSION_MINOR(gles_version));

    XWindowAttributes gwa;
    XGetWindowAttributes(display, window, &gwa);
    glViewport(0, 0, gwa.width, gwa.height);

    double time_passed = 0.0;
    bool quit = false;
    while (!quit) {
        while (XPending(display)) {
            XEvent xev;
            XNextEvent(display, &xev);

            if (xev.type == KeyPress) {
                quit = true;
            }
        }
        time_passed += 0.0166666;
        //glClearColor(0.8f, 0.6, 0.7, 1.0);
        glClearColor(0.0f, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        mat4 m = perspective(65.0f, (GLfloat) gwa.width / (GLfloat) gwa.height, 1.0f,
                            100.0f);
        glLoadMatrixf(m.d);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(-90, 1, 0, 0);
        glTranslatef(0, 0, -1.0f);

        // Draw a rotating colorful triangle
        glTranslatef(0.0f, 14.0f, 0.0f);
        glRotatef((GLfloat) time_passed * 100.0f, 0.0f, 0.0f,
              1.0f);
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.01f, 0.01f, 0.0f);
        glVertex3f(-5.0f, 0.0f, -4.0f);
        glColor4f(0.01f, 1.0f, 0.01f, 0.0f);
        glVertex3f(5.0f, 0.0f, -4.0f);
        glColor4f(0.01f, 0.01f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 6.0f);
        glEnd();

        eglSwapBuffers(egl_display, egl_surface);

        usleep(1000 * 10);
    }
    gladLoaderUnloadGL();
    //gladLoaderUnloadGLES2();

    eglDestroyContext(egl_display, egl_context);
    eglDestroySurface(egl_display, egl_surface);
    eglTerminate(egl_display);

    gladLoaderUnloadEGL();

    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
