# KeyZ-ro

Made for Linux. Show keybindings for various programs. To remind yourself or any
unfortunate normal computer user who has to interact with your
mouseless ridiculousness.

## Usage

To make this easy to get too, I bind mine to `$mod + ESC` in my


1. Scroll down to the binding you forgot.
2. Remember.
3. Press `ESC` to exit or close it with your WM's controls.

## Installation

```bash
git clone github.com/projectz-ro/keyz-ro
cd keyz-ro
bash install.sh
```

### Dependencies

`raylib` and `cjson`.

Install script will download them if you dont have them.

## Config

JSON config to edit to fill in the programs and keybindings to
display.

Fully manual. I don't want to try to keep up with all
the different ways different programs map keys. Especially when
it won't even necessarily stay consistent per program.

That said, just remember to update your config after you add a
new binding. Not too hard.

I also add a window rule in my config to make it
transparent. Maybe in the future I'll add color config, but this
isn't really a tool you'll keep open for long.

> Note:
> I'm using arch with sway, but there should be no reason this
> doesn't work on most other Linux distros and WM's. If it doesn't,
> let me know.
