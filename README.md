# clipscreen

clipscreen is a simple application that creates a virtual monitor that mirrors a portion of your screen. A green rectangle highlights the specified area. 

Why's this useful? You can use any screen sharing tool (Google Meet, Microsoft Teams, Jitsi Meet, etc.) to share the virtual monitor instead of your entire screen. No need to share individual windows and having to switch between them, just move any window you want to share into the green border.

## Compile

Ensure you have the following installed on your system:

- X11 development libraries
- Cairo graphics library
- A C compiler (e.g., gcc)

For example, on Ubuntu 24.04,
running `apt-get install libx11-dev xserver-xorg-dev xorg-dev libcairo2-dev gcc`
will install the required libraries.

Then simply run the following command to compile the application:

```bash
make
```

Note: The application has only been tested on Linux and xorg. I doubt it will work on any other system.

## Usage

Run the compiled executable with the following command:

```bash
./clipscreen <width>x<height>+<x>+<y>
```

- `<width>`: The width of the overlay and virtual monitor.
- `<height>`: The height of the overlay and virtual monitor.
- `<x>`: The x-coordinate of the top-left corner of the overlay and virtual monitor.
- `<y>`: The y-coordinate of the top-left corner of the overlay and virtual monitor.

For example:

```bash
./clipscreen 800x600+100+100
```

This command will create an 800x600 overlay window starting at position (100, 100) on your screen.

To select an area interactively:

```bash
./clipscreen $(hacksaw)
```

or

```bash
./clipscreen $(slop)
```

With this command, you will be able to select a portion of the screen interactively.

## Termination

To terminate the application, press `Ctrl+C` in the terminal where the application is running.

## Links

  * [hacksaw](https://github.com/neXromancers/hacksaw)
  * [slop](https://github.com/naelstrof/slop)

## License

Copyright 2024 Andreas Gohr <andi@splitbrain.org>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
