# xfce4-dynamic-opacity
A panel plugin that modifies the panel's opacity when a window is near. There's a fade effect that forces the panel to refresh, so the entire panel should have a uniform opacity. It works on a per-panel basis, just add the plugin to the desired panels and you're set.

I wrestled with CMake instead of XFCE's build tools, so if something goes wrong blame CMake. Right now the timings for transitions aren't correct, and a lot of the program is a horrible mess of GObject code. The times of the commits are also ruined because the time on my VM was off by 13 days.

![Far](https://raw.githubusercontent.com/mhgar/xfce4-dynamic-opacity/master/1.png) ![Near](https://raw.githubusercontent.com/mhgar/xfce4-dynamic-opacity/master/2.png)

## Installation
You'll first need ``gcc`` and ``make``. Clone, then run this in the project's root:
``$ cmake ./CMakeLists.txt && make && sudo make install``. If you need to uninstall, remove the files listed in the newly created ``install_manifest.txt`` file. You can use ``cat install_manifest.txt | xargs sudo rm`` if you're lazy.

## Troubleshooting
After adding to the panel you might need to fiddle with the settings to generate the xfconf entries, but it should work fine out of the box. You'll also need to enable "Allow forcing panel's background style" if your panel is not already in "Solid color" mode. Your currently selected solid colour will be replaced with black, but I hope to change this in a later version. In some cases the GTK theme can affect how the panel renders, which may case uneven bumps in the panel where other plugins are located. In this case you must change the GTK theme of ``xfce4-panel``, if you search around you can find information on how to set a GTK theme for a specific program.
