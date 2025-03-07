# Split Flap Display
Firmware for the modular Split Flap Display created by Morgan Manly

- [Instructables](https://www.instructables.com/Split-Flap-Display-3D-Printed-Modular-Compact-Encl/)
- [Print Files](https://makerworld.com/en/models/1116618#profileId-1114192)

## Features
- Fully 3D Printed Modular Split Flap Display with 37 Characters Per Module
- Small Size, 8 Modules are 320mm, 3 Modules are 130mm Wide. 80mm Tall
- Web Interface To Switch Between Operation Modes, modes include custom input, date mode, time mode

## Setup Instructions

### Using PlatformIO Core
1. Install the [PlatformIO Core CLI](https://platformio.org/install/cli)
1. [Download](https://github.com/ManlyMorgan/Split-Flap-Display/archive/refs/heads/main.zip) or clone the [git repository](https://github.com/ManlyMorgan/Split-Flap-Display).
1. Open a terminal and `cd` to the project root
1. Edit `src/config.dist.h` to define the number of modules and other hardware configurations for your build
1. Connect the ESP32 to your computer using a USB cable.
1. Run `platformio run -t upload -t uploadfs`.
  * This should install all required libraries, compile and upload the firmware, and then upload the filesystem
1. Enjoy!

### Using the PlatformIO IDE ( untested )
1. Install the [PlatformIO IDE](https://platformio.org/install/ide).
1. [Download](https://github.com/ManlyMorgan/Split-Flap-Display/archive/refs/heads/main.zip) or clone the [git repository](https://github.com/ManlyMorgan/Split-Flap-Display).
1. Open the PlatformIO IDE and open the project by selecting the folder where the repository was downloaded or cloned.
1. Edit `src/config.dist.h` to define the number of modules and other hardware configurations for your build.
1. Connect the ESP32 to your computer using a USB cable.
1. Click the "PlatformIO: Upload" button in the IDE or use the command palette to run the "PlatformIO: Upload" task.
1. Enjoy!

## Contributing

### Setup
1. Fork this repository on GitHub.
1. Clone your forked repository to your local machine.
  * `git clone https://github.com/your-username/Split-Flap-Display.git`
1. Copy `src/config.dist.h` to `src/config.h` and customize with your own configuration
  * This prevents your config from being committed, as it will be used during compilation but ignored by git.

### Create your feature
1. Start a new branch with a descriptive name.
1. Compile and upload your changes
  * `platformio run -t upload` to compile core changes and upload
  * `platformio run -t uploadfs` to upload files in `src/data`
  * You can build and upload everything at once using `platformio run -t upload -t uploadfs`, but it takes longer.
1. When ready, commit and push your changes to your forked repository.
1. Open a pull request to the main repository.
