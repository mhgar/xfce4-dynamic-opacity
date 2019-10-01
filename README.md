# xfce4-dynamic-opacity
A panel plugin that modifies the panel's opacity when a window is near. There's a fade effect that forces the panel to refresh, so the entire panel should have a uniform opacity. It works on a per-panel basis, just add the plugin to the desired panels and you're set.

I wrestled with CMake instead of XFCE's build tools, so if something goes wrong blame CMake. Right now the timings for transitions aren't correct, and a lot of the program is a horrible mess of GObject code. The times of the commits are also ruined because the time on my VM was off by 13 days.

![Far](https://raw.githubusercontent.com/mhgar/xfce4-dynamic-opacity/master/1.png) ![Near](https://raw.githubusercontent.com/mhgar/xfce4-dynamic-opacity/master/2.png)

## Installation
You'll first need ``gcc`` and ``make``. Clone, then run this in the project's root:
``$ cmake ./CMakeLists.txt && make && sudo make install``. If you need to uninstall, remove the files listed in the newly created ``install_manifest.txt`` file. You can use ``cat install_manifest.txt | xargs sudo rm`` if you're lazy.

## Settings page
![Settings](https://raw.githubusercontent.com/mhgar/xfce4-dynamic-opacity/master/settings.png)

- "Allow forcing panel's background style" allows the background style of the panel to be overwitten by the plugin.
- "Window proximity" dictates how close windows can get before changing the panel's opacity.
- "Normal opacity" is the opacity of the panel when there are no windows within the given proximity.
- "Opacity when a window is near" is exactly what it says. This can be greater or less than the "normal opacity".
- "Transition time" is the time is milliseconds it takes for the panel to change between the two opacities. Smaller differences in opacities will not result in faster transition times.
- NOTE: transitioning is currently poorly implemented with exponential interpolation, so right now just set the "transition time" to whatever feels best.

## Troubleshooting
After adding the plugin to the panel you may need to fiddle with the settings to generate xfconf entries, but for now it seems to work out of the box. However, you'll also need to enable "Allow forcing panel's background style" in the plugin's options if your panel is not already in "Solid color" mode. Your currently selected solid colour will be replaced with black, but I hope to change this in a later version. 

In some cases the GTK theme can affect how the panel renders, which may cause uneven bumps in the panel where other plugins are located. In this case you must change the GTK theme of ``xfce4-panel``. If you search around you can find information on how to set a GTK theme for a specific program. 

On some distributions showing shadows on panels is enabled by default. For the best visual experience please disable them through ``Settings > Window Manager Tweaks > Compositor > Show shadows under dock windows``.
