# Split Flap Display Firmware
Firmware for the modular Split Flap Display created by [Morgan Manly](https://github.com/ManlyMorgan/Split-Flap-Display)

- [Instructables](https://www.instructables.com/Split-Flap-Display-3D-Printed-Modular-Compact-Encl/) ( original )
- [Print Files](https://makerworld.com/en/models/1116618#profileId-1114192) ( original )
- [Original firmware](https://github.com/ManlyMorgan/Split-Flap-Display)

## Features
- Fully 3D Printed Modular Split Flap Display with 37 Characters Per Module
- Small Size, 8 Modules are 320mm, 3 Modules are 130mm Wide. 80mm Tall
- Web Interface To Switch Between Operation Modes, modes include custom input, date mode, time mode

## Future features ( help wanted )
- Additional board support
- Full configuration via web interface
- Pre-built firmware releases with web-based firmware update via [esptool-js](https://espressif.github.io/esptool-js/docs/index.html)
- Optional basic json API endpoint to update display remotely
- [ESPHome](https://esphome.io) support

## Supported boards
| Environment          | Processor     | Tested Boards                                                |
| -------------------- | ------------- | ------------------------------------------------------------ |
| `esp32_c3` (default) | ESP32-C3FN4   | Teyleten Robot ESP32-C3-SuperMini<br>Waveshare ESP32-C3-Zero |
| `esp32_s3`           | ESP32-S3FH4R2 | Waveshare ESP32-S3-Zero<sup>*</sup>                          |

<sub>* Requires manually resetting the board into firmware upload mode by holding BOOT, pressing & releasing RESET, then releasing BOOT prior to upload. After uploading is successful, either press & release RESET or power cycle the board to put it in normal operation mode.</sub>

## Setup Instructions

### Using PlatformIO Core
1. Install the [PlatformIO Core CLI](https://platformio.org/install/cli)
1. [Download](https://github.com/jhoff/Split-Flap-Display/archive/refs/heads/main.zip) or clone the [git repository](https://github.com/jhoff/Split-Flap-Display).
1. Open a terminal and `cd` to the project root
1. Edit `src/config.dist.h` to define the number of modules and other hardware configurations for your build
1. Connect the ESP32 to your computer using a USB cable.
1. Run `platformio run -t upload -t uploadfs -e <environment>`.
  * Replace `<environment>` with a value listed in [Supported boards](#supported-boards)
  * This should install all required libraries, compile and upload the firmware, and then upload the filesystem
1. Enjoy!

### Using the PlatformIO IDE ( untested )
1. Install the [PlatformIO IDE](https://platformio.org/install/ide).
1. [Download](https://github.com/jhoss/Split-Flap-Display/archive/refs/heads/main.zip) or clone the [git repository](https://github.com/jhoss/Split-Flap-Display).
1. Open the PlatformIO IDE and open the project by selecting the folder where the repository was downloaded or cloned.
1. Edit `src/config.dist.h` to define the number of modules and other hardware configurations for your build.
1. Connect the ESP32 to your computer using a USB cable.
1. Click the "PlatformIO: Upload" button in the IDE or use the command palette to run the "PlatformIO: Upload" task.
1. Enjoy!

## Contributing

### Setup
1. [Fork](https://github.com/jhoff/Split-Flap-Display/fork) this repository on GitHub.
1. Clone your forked repository to your local machine.
  * `git clone https://github.com/your-username/Split-Flap-Display.git`
1. Copy `src/config.dist.h` to `src/config.h` and customize with your own configuration
  * This prevents your config from being committed, as it will be used during compilation but ignored by git.

### Create your feature
1. Start a new branch with a descriptive name.
1. Compile and upload your changes
  * `platformio run -t upload -e <environment>` to compile core changes and upload
  * `platformio run -t uploadfs -e <environment>` to upload files in `src/data`
  * You can build and upload everything at once using `platformio run -t upload -t uploadfs -e <environment>`, but it takes longer.
1. When ready, commit and push your changes to your forked repository.
1. Open a pull request to this repository.
