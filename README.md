# xfce4-dynamic-opacity
A panel plugin that modifies its opacity when a window is near. I wrestled with CMake instead of XFCE's build tools, so if something goes wrong blame CMake. Right now the timings for transitions aren't correct, and a lot of the program is a horrible mess of GObject code. The times of the commits are also ruined because the time on my VM was off by 13 days.

## Installation
Clone, then run this in the project's root:
``$ cmake ./CMakeLists.txt && make && sudo make install`` 

If you need to uninstall, remove the files listed in the newly created ``install_manifest.txt`` file. 
