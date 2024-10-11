#include <assert.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

#include <cairo.h>
#include <cairo-xlib.h>

#include <cstdlib>
#include <csignal>
#include <csignal>
#include <unistd.h>
#include <chrono>
#include <thread>

void draw(cairo_t *cr, int w, int h) {
    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.5);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_set_line_width(cr, 10.0);
    cairo_stroke(cr);
}

void set_monitor(Display *d, Window root, int w, int h, int x, int y) {
    RROutput primary_output = XRRGetOutputPrimary(d, root);

    // Create virtual monitor (equivalent to xrandr --setmonitor)
    XRRMonitorInfo monitor;
    monitor.name = XInternAtom(d, "clipscreen", False);
    monitor.x = x+5;
    monitor.y = y+5;
    monitor.width = w-10;
    monitor.height = h-10;
    monitor.mwidth = w-10; // Aspect ratio 1/1
    monitor.mheight = h-10; // Aspect ratio 1/1
    monitor.noutput = 1; // Number of outputs used by this monitor
    monitor.outputs = &primary_output;

    XRRSetMonitor(d, root, &monitor);
}

void removeMonitor(Display* display, Window root) {
    Atom monitorAtom = XInternAtom(display, "clipscreen", False);

    if (!monitorAtom) {
        return;
    }

    // Remove the virtual monitor
    XRRDeleteMonitor(display, root, monitorAtom);
}


volatile sig_atomic_t sigint_received = 0;

void handle_sigint(int /*sig*/) {
    sigint_received = 1;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <width> <height> <x> <y>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int w = atoi(argv[1]);
    int h = atoi(argv[2]);
    int x = atoi(argv[3]);
    int y = atoi(argv[4]);
    Display *d = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(d);

    set_monitor(d, root, w, h, x, y);

    // these two lines are really all you need
    XSetWindowAttributes attrs;
    attrs.override_redirect = true;

    XVisualInfo vinfo;
    if (!XMatchVisualInfo(d, DefaultScreen(d), 32, TrueColor, &vinfo)) {
        printf("No visual found supporting 32 bit color, terminating\n");
        exit(EXIT_FAILURE);
    }
    // these next three lines add 32 bit depth, remove if you dont need and change the flags below
    attrs.colormap = XCreateColormap(d, root, vinfo.visual, AllocNone);
    attrs.background_pixel = 0;
    attrs.border_pixel = 0;

    Window overlay = XCreateWindow(
        d, root,
        x, y, w, h, 0,
        vinfo.depth, InputOutput,
        vinfo.visual,
        CWOverrideRedirect | CWColormap | CWBackPixel | CWBorderPixel, &attrs
    );

    XMapWindow(d, overlay);

    cairo_surface_t* surf = cairo_xlib_surface_create(d, overlay, vinfo.visual, w, h);
    cairo_t* cr = cairo_create(surf);

    draw(cr, w, h);
    XFlush(d);

    printf("waiting for sigint to stdout\n");
    while (!sigint_received) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    removeMonitor(d, root);

    XUnmapWindow(d, overlay);
    XCloseDisplay(d);
    return 0;
}
