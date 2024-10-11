#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <assert.h>
#include <cairo-xlib.h>
#include <cairo.h>
#include <stdio.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <thread>

volatile sig_atomic_t sigint_received = 0;

/**
 * @brief Signal handler for SIGINT.
 *
 * Sets the sigint_received flag to 1 when a SIGINT is received.
 *
 * @param sig The signal number (unused).
 */
void handle_sigint(int /*sig*/) {
    sigint_received = 1;
}

/**
 * @brief Draws a green rectangle on the given Cairo context.
 *
 * @param cr The Cairo context to draw on.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 */
void draw_rectangle(cairo_t *cr, int w, int h) {
    cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 0.5);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_set_line_width(cr, 10.0);
    cairo_stroke(cr);
}

/**
 * @brief Adds a virtual monitor to the display.
 *
 * Creates a virtual monitor with the specified dimensions and position.
 *
 * The created monitor will be 10px smaller than specified to not overlap with the window border.
 *
 * @param d The display connection.
 * @param root The root window.
 * @param w The width of the monitor.
 * @param h The height of the monitor.
 * @param x The x-coordinate of the monitor.
 * @param y The y-coordinate of the monitor.
 */
void add_monitor(Display *d, Window root, int w, int h, int x, int y) {
    RROutput primary_output = XRRGetOutputPrimary(d, root);

    // Create virtual monitor (equivalent to xrandr --setmonitor)
    XRRMonitorInfo monitor;
    monitor.name = XInternAtom(d, "clipscreen", False);
    monitor.x = x + 5;
    monitor.y = y + 5;
    monitor.width = w - 10;
    monitor.height = h - 10;
    monitor.mwidth = w - 10;  // Aspect ratio 1/1
    monitor.mheight = h - 10; // Aspect ratio 1/1
    monitor.noutput = 1;      // Number of outputs used by this monitor
    monitor.outputs = &primary_output;

    XRRSetMonitor(d, root, &monitor);
}

/**
 * @brief Removes the virtual monitor from the display.
 *
 * Deletes the virtual monitor named "clipscreen".
 *
 * @param display The display connection.
 * @param root The root window.
 */
void remove_monitor(Display *display, Window root) {
    Atom monitorAtom = XInternAtom(display, "clipscreen", False);

    if (!monitorAtom) {
        return;
    }

    // Remove the virtual monitor
    XRRDeleteMonitor(display, root, monitorAtom);
}

/**
 * @brief Main function for the clipscreen application.
 *
 * Sets up a virtual monitor and draws a rectangle around it. Waits for SIGINT to terminate.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return int Exit status.
 */
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

    add_monitor(d, root, w, h, x, y);

    // these two lines are really all you need
    XSetWindowAttributes attrs;
    attrs.override_redirect = true;

    XVisualInfo vinfo;
    if (!XMatchVisualInfo(d, DefaultScreen(d), 32, TrueColor, &vinfo)) {
        printf("No visual found supporting 32 bit color, terminating\n");
        exit(EXIT_FAILURE);
    }
    attrs.colormap = XCreateColormap(d, root, vinfo.visual, AllocNone);
    attrs.background_pixel = 0;
    attrs.border_pixel = 0;

    Window overlay = XCreateWindow(d, root, x, y, w, h, 0, vinfo.depth, InputOutput, vinfo.visual,
                                   CWOverrideRedirect | CWColormap | CWBackPixel | CWBorderPixel, &attrs);

    XMapWindow(d, overlay);

    cairo_surface_t *surf = cairo_xlib_surface_create(d, overlay, vinfo.visual, w, h);
    cairo_t *cr = cairo_create(surf);

    draw_rectangle(cr, w, h);
    XFlush(d);

    printf("waiting for sigint to stdout\n");
    while (!sigint_received) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    remove_monitor(d, root);

    XUnmapWindow(d, overlay);
    XCloseDisplay(d);
    return 0;
}
