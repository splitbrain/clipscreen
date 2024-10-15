#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/shape.h>
#include <assert.h>
#include <cairo-xlib.h>
#include <cairo.h>
#include <stdio.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <thread>

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
 * @brief Removes the virtual monitor from the display.
 *
 * Deletes the virtual monitor named "clipscreen" if it exists.
 *
 * @param display The display connection.
 * @param root The root window.
 */
void remove_monitor(Display *display, Window root) {
    int numMonitors;
    XRRMonitorInfo *monitors = XRRGetMonitors(display, root, True, &numMonitors);

    for (int i = 0; i < numMonitors; ++i) {
        if (strcmp(XGetAtomName(display, monitors[i].name), "clipscreen") == 0) {
            XRRDeleteMonitor(display, root, monitors[i].name);
            printf("Removed virtual monitor\n");
            break;
        }
    }

    XFree(monitors);
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
    // remove any leftover virtual monitor
    remove_monitor(d, root);

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
    printf("Added virtual monitor\n");
}

/**
 * @brief Creates an overlay window on the display.
 *
 * Sets up an overlay window with the specified dimensions and position.
 *
 * @param d The display connection.
 * @param root The root window.
 * @param vinfo The X visuals
 * @param w The width of the overlay.
 * @param h The height of the overlay.
 * @param x The x-coordinate of the overlay.
 * @param y The y-coordinate of the overlay.
 * @return Window The created overlay window.
 */
Window create_overlay_window(Display *d, Window root, XVisualInfo vinfo, int w, int h, int x, int y) {
    XSetWindowAttributes attrs;
    attrs.override_redirect = true;

    attrs.colormap = XCreateColormap(d, root, vinfo.visual, AllocNone);
    attrs.background_pixel = 0;
    attrs.border_pixel = 0;

    Window overlay = XCreateWindow(d, root, x, y, w, h, 0, vinfo.depth, InputOutput, vinfo.visual,
                                   CWOverrideRedirect | CWColormap | CWBackPixel | CWBorderPixel, &attrs);

    XRectangle rect;
    XserverRegion region = XFixesCreateRegion(d, &rect, 0);
    XFixesSetWindowShapeRegion(d, overlay, ShapeInput, 0, 0, region);
    XFixesDestroyRegion(d, region);

    XMapWindow(d, overlay);
    return overlay;
}

/**
 * @brief Parses the geometry from the command-line arguments.
 *
 * Sets the width, height, x, and y accordingly and ensures a minimum size of 100x100.
 *
 * Exits on errors.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param w The width of the window.
 * @param h The height of the window.
 * @param x The x-coordinate of the window.
 * @param y The y-coordinate of the window.
 */
void initGeometry(int argc, char *argv[], unsigned int &w, unsigned int &h, int &x, int &y) {
    if (argc > 2 || argc <= 1) {
        fprintf(stderr, "Usage: %s <width>x<height>+<x>+<y> (e.g. 800x600+100+100)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int ret = XParseGeometry(argv[1], &x, &y, &w, &h);
    if (ret == 0) {
        fprintf(stderr, "invalid geometry: %s (e.g. 800x600+100+100)\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // ensure minimum size so that we can draw a border around something
    if (w < 100) {
        fprintf(stderr, "Auto adjusted width\n");
        w = 100;
    }
    if (h < 100) {
        fprintf(stderr, "Auto adjusted height\n");
        h = 100;
    }
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
    unsigned int w = 0;
    unsigned int h = 0;
    int x = 0;
    int y = 0;

    // parse geometry from arguments
    initGeometry(argc, argv, w, h, x, y);

    // set up X11
    Display *d = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(d);

    // add virtual monitor
    add_monitor(d, root, w, h, x, y);

    // Initialize Visuals and check for 32 bit color support
    XVisualInfo vinfo;
    if (!XMatchVisualInfo(d, DefaultScreen(d), 32, TrueColor, &vinfo)) {
        printf("No visual found supporting 32 bit color, terminating\n");
        exit(EXIT_FAILURE);
    }

    // create overlay border
    Window overlay = create_overlay_window(d, root, vinfo, w, h, x, y);
    cairo_surface_t *surf = cairo_xlib_surface_create(d, overlay, vinfo.visual, w, h);
    cairo_t *cr = cairo_create(surf);
    draw_rectangle(cr, w, h);
    XFlush(d);

    // wait for SIGINT or SIGTERM
    printf("Press Ctrl-C to exit\n");
    int sig;
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);
    sigprocmask(SIG_BLOCK, &sigset, nullptr);
    sigwait(&sigset, &sig);

    // clean up
    remove_monitor(d, root);
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
    XUnmapWindow(d, overlay);
    XCloseDisplay(d);
    return 0;
}
