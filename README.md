# Split Flap Display Firmware

Firmware for the modular Split Flap Display created by [Morgan Manly](https://github.com/ManlyMorgan/Split-Flap-Display)

- [Instructables](https://www.instructables.com/Split-Flap-Display-3D-Printed-Modular-Compact-Encl/) ( original )
- [Print Files](https://makerworld.com/en/models/1116618#profileId-1114192) ( original )
- [Original firmware](https://github.com/ManlyMorgan/Split-Flap-Display)

## Features

- Fully 3D Printed Modular Split Flap Display with 37 Characters Per Module
- Small Size, 8 Modules are 320mm, 3 Modules are 130mm Wide. 80mm Tall
- OTA updates
- Fully configurable and controllable via Web Interface
    - Switch Between Operation Modes, modes include custom input, date mode, and time mode
    - Configure WiFi, Timezone, and hardware settings
- MQTT Support
- OTA Firmware / Filesystem updating

## Supported boards

| Environment          | Processor     | Tested Boards                                                            |
| -------------------- | ------------- | ------------------------------------------------------------------------ |
| `esp32_c3` (default) | ESP32-C3FN4   | Teyleten Robot ESP32-C3-SuperMini<br>Waveshare ESP32-C3-Zero             |
| `esp32_s3`           | ESP32-S3FH4R2 | Waveshare ESP32-S3-Zero<sup>\*</sup><br>ESP32-S3 Super Mini<sup>\*</sup> |

<sub>\* Requires manually resetting the board into firmware upload mode by holding BOOT, pressing & releasing RESET, then releasing BOOT prior to upload. After uploading is successful, either press & release RESET or power cycle the board to put it in normal operation mode.</sub>

## Setup Instructions

1. Install dependencies
    - [PlatformIO Core CLI](https://platformio.org/install/cli)
    - [Node Version Mananger](https://github.com/nvm-sh/nvm)
    - [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) (Only required if contributing code)
1. [Download](https://github.com/jhoff/Split-Flap-Display/archive/refs/heads/main.zip) or clone this [git repository](https://github.com/jhoff/Split-Flap-Display).
1. Open a terminal and `cd` to the project root
1. Install required version of npm for the project - `nvm install`
1. Install build dependencies - `npm install`
1. Connect the ESP32 to your computer using a USB cable.
1. Build everything and upload to the board - `npm run build`

- Automatically formats all source code ( `npm run format` - if needed )
- Compiles and minifies all frontend assets ( `npm run assets` )
- Downloads all required arduino / esp32 libraries
- Compiles and uploads the esp32 firmware ( `npm run pio:firmware` or `pio run -t upload -e <environment>` )
- Compiles and uploads the littlefs filesystem ( `npm run pio:filesystem` or `pio run -t uploadfs -e <environment>` )

1. Enjoy!

### Using OTA to update the firmware

On the settings page set an OTA password to enable OTA updatable firmware. Use this same password for your `auth` flag in `platformio.ini`, and then use a device environment with `*_ota` appended (ie `esp32_s3_ota`) to upload a new firmware and/or filesystem

## Contributing

### Setup

1. Create a GitHub account if necessary and login.
1. [Fork](https://github.com/jhoff/Split-Flap-Display/fork) this repository.
1. Clone your forked repository to your local machine.

- `git clone https://github.com/your-username/Split-Flap-Display.git`

1. Install the dependencies listed in [Setup Instructions](#setup-instructions)
1. Skip step 2, complete the setup and use `npm run build` to test and upload your changes.

### Create your feature

1. Start a new branch with a descriptive name.
1. Compile and upload your changes

- `npm run pio:firmware` or `pio run -t upload -e <environment>` to compile firmware and upload
- `npm run pio:filesystem` or `pio run -t uploadfs -e <environment>` to compile the filesystem from `src/web` and upload
- You can do both together using `npm run pio` or `pio run -t upload -t uploadfs -e <environment>`

1. When ready, commit and push your changes to your forked repository.
1. Open a pull request to this repository.
