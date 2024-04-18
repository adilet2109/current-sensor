# Software for ESP32 laser pulser project

## Building and flashing the firmware 
Ensure either [VSCode](https://code.visualstudio.com/download) and the [platformio extension](https://platformio.org/platformio-ide) are installed, or the [PlatformIO Core (CLI)](https://docs.platformio.org/en/stable/core/index.html#platformio-core-cli) is installed and configured according to the build platform's OS (Windows/MacOS/Linux).

Clone or download the repo onto the build system into a dedicated folder.

### VSCode and PlatformIO extension
Open VSCode, navigate to `File -> Open Folder...` and open the `/software/` folder of the repo.

The PlatformIO extension should autmatically detect that a PlatformIO project has been opened, and will do some automatic configuration and downloading of libraries.

To build the project, click the Build Project button at the bottom of the VSCode window, or open the Command Palette (Ctrl + Shift + P) and run the "PlatformIO: Build" command.

To upload to a target device, first connect the device to the build computer using a USB cable, and either click the Upload button at the bottom of the screen, or open the Command Palette and run "PlatformIO: Upload" command.

To view the serial monitor output from the target device, either click the Serial Monitor button at the bottom of the screen, or run "PlatformIO: Serial Monitor" from the Command Palette.

### PlatformIO Core (CLI)
Open a system terminal from the `/software/` folder. To build, flash, and then open a serial monitor, run the command 
```
pio run --list-targets monitor
```

See the [PlatformIO Core CLI Guide](https://docs.platformio.org/en/latest/core/userguide/index.html) for more information on further commands available from the terminal.

## Usage 

*coming soon*