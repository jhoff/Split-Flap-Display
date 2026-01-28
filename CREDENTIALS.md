# Credentials Management

This project uses a local credentials file to keep sensitive information out of version control.

## Setup for Local Development

1. **Copy the example credentials file:**
   ```bash
   cp src/credentials.h.example src/credentials.h
   ```

2. **Edit `src/credentials.h` with your actual credentials:**
   ```cpp
   #define WIFI_SSID "your_wifi_ssid"
   #define WIFI_PASS "your_wifi_password"

   #define MQTT_SERVER "your-mqtt-broker.com"
   #define MQTT_PORT 1883
   #define MQTT_USER "your_username"
   #define MQTT_PASS "your_password"
   ```

3. **Build and upload:**
   ```bash
   npm run build
   ```

The `credentials.h` file is in `.gitignore` and will never be committed to git.

## How It Works

- **`src/credentials.h.example`** - Template file (committed to git)
- **`src/credentials.h`** - Your local credentials (NOT in git, in .gitignore)
- **`src/SplitFlapDisplay.ino`** - Uses credentials if available, otherwise defaults to empty

The code uses `__has_include` to check if `credentials.h` exists:
- If it exists: Uses your local credentials as defaults
- If it doesn't exist: Uses empty defaults
- Either way: Settings can be configured via the web interface

## Alternative: Configure via Web Interface

You don't need to create `credentials.h` if you prefer to configure everything through the web interface:

1. Flash the firmware with default (empty) settings
2. Connect to the device's WiFi network
3. Open the settings page
4. Enter your WiFi and MQTT credentials
5. Save settings

Settings are persisted to the ESP32's flash storage.

## What's Gitignored

The following files are excluded from version control:
- `src/credentials.h` - Local MQTT and WiFi credentials
- `.env` - Environment variables
- `.claude/settings.local.json` - Claude Code local settings
- `secrets.h` - Alternative secrets file
- `credentials.json` - Alternative JSON credentials
- `platformio_override.ini` - PlatformIO overrides (if needed for other settings)

## What's Committed (Safe)

The following files ARE committed:
- `src/credentials.h.example` - Template with placeholder credentials
- `platformio.ini` - Main config WITHOUT credentials (references credentials.h)

## Security Best Practices

- Never commit credentials to git
- Use strong passwords for MQTT and OTA
- Change default passwords after initial setup
- Restrict MQTT broker access to authorized devices
- Use TLS/SSL for MQTT if possible (requires code modifications)
