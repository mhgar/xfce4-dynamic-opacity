# xfce4-dynamic-opacity
A panel plugin that modifies the panel's opacity when a window is near. There's a fade effect that forces the panel to refresh, so the entire panel should have a uniform opacity. It works on a per-panel basis, just add the plugin to the desired panels and you're set.

I wrestled with CMake instead of XFCE's build tools, so if something goes wrong blame CMake. Right now the timings for transitions aren't correct, and a lot of the program is a horrible mess of GObject code. The times of the commits are also ruined because the time on my VM was off by 13 days.

![Far](https://raw.githubusercontent.com/mhgar/xfce4-dynamic-opacity/master/1.png) ![Near](https://raw.githubusercontent.com/mhgar/xfce4-dynamic-opacity/master/2.png)

## Installation
You'll first need ``gcc`` and ``make``. Clone, then run this in the project's root:
``$ cmake ./CMakeLists.txt && make && sudo make install``. After adding to the panel you might need to fiddle with the settings to generate the xfconf entries, but it should work fine.

If you need to uninstall, remove the files listed in the newly created ``install_manifest.txt`` file. 
