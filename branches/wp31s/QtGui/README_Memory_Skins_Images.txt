The emulator stores non-volatile memory and flash regions in .dat files.
Non-volatile memory is in wp34s.dat, flash regions in wp34s-R.dat and wp34s-1.dat to wp34s-9.dat


These files are searched in different directories:

- first in the user's application data directory:
	- on Linux, $HOME/.config/WP-34s
	- on MacOSX, $HOME/Library/Preferences/WP-34s
	- on Windows, %APPDATA%\WP-34S 

- then on MacOSX only, in the package ressources directory:
<ExecutableDirectory>/../resources/memory

- finally, in the "memory" subdirectory of the executable directory

The .dat files near the excutable are read-only. Memory and flash regions are always written in the user's ones.


If "Use a custom Directory for memory files" is checked in Preferences, the directory exists and is readable, it will be the only used
for .dat files.
This option is useful for development. 

If the "-dev" option is present on the command line, the memory subdirectory of the application "current directory", i.e. the one it is launched 
from is also searched. But not if the "Use a custom Directory for memory files" option is checked.

Skins and images files are searched in the same directories/subdirectories as the memory files except that they have subdirectory names of the own: 
"skins" and "images" obviously.
On Linux, they are also searched in <execdir>/../lib/WP34-s/{skins,images} and on MacOS X in <execdir>/../resources/{skins,images}

The "Use a custom Directory for memory files" and "-dev" behave differently for skins and images: they add the corresponding directories 
to the search path.


The system also uses the DejaVu and Luxi free fonts. See here for more informations:
https://sourceforge.net/projects/dejavu/
http://en.wikipedia.org/wiki/Luxi_(fonts)

