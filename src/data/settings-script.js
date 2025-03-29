document.addEventListener("DOMContentLoaded", function () {
    const ssidField = document.getElementById("ssid");
    const passwordField = document.getElementById("password");
    const timezoneField = document.getElementById("timezone");
    const format24Toggle = document.getElementById("format-24-toggle");
    const button = document.getElementById("settingsSaveBtn");
    const changeWifiLink = document.getElementById("change-wifi-link");
    const mqttServerField = document.getElementById("mqtt-server");
    const mqttPortField = document.getElementById("mqtt-port");
    const mqttUserField = document.getElementById("mqtt-user");
    const mqttPasswordField = document.getElementById("mqtt-password");

    function toggleSettings() {
        document.getElementById("settingsModal").classList.toggle("hidden");
    }

    function enableWifiEdit() {
        ssidField.readOnly = false;
        document.getElementById("wifi-password-group").style.display = "block";
        changeWifiLink.style.display = "none";
    }

    function loadConfig() {
        fetch("/config")
            .then(res => res.json())
            .then(config => {
                format24Toggle.checked = config.format24;
                timezoneField.value = config.timezone;
                ssidField.value = config.ssid;
                mqttServerField.value = config.mqtt_server;
                mqttPortField.value = config.mqtt_port;
                mqttUserField.value = config.mqtt_user;
                mqttPasswordField.value = "";

                if (config.ssid && config.ssid !== "") {
                    ssidField.readOnly = true;
                    document.getElementById("wifi-password-group").style.display = "none";
                    changeWifiLink.style.display = "flex";
                } else {
                    ssidField.readOnly = false;
                    document.getElementById("wifi-password-group").style.display = "block";
                    changeWifiLink.style.display = "none";
                }

                const modeMap = {
                    0: "Custom",
                    1: "Custom",
                    2: "Date",
                    3: "Time",
                    4: "Count",
                    5: "Random"
                };
                const dropdown = document.getElementById("modeDropDown");
                if (dropdown) {
                    dropdown.value = modeMap[config.mode] || "Date";

                    // Trigger change manually so the UI updates
                    const event = new Event("change");
                    dropdown.dispatchEvent(event);
                }

            })
            .catch(err => console.error("Failed to load config:", err));
    }

    function saveSettings() {
        const ssid = ssidField.value.trim();
        const password = passwordField.value.trim();
        const timezone = timezoneField.value;
        const format24 = format24Toggle.checked ? "1" : "0";
        const mqttServer = mqttServerField.value.trim();
        const mqttPort = mqttPortField.value.trim();
        const mqttUser = mqttUserField.value.trim();
        const mqttPassword = mqttPasswordField.value.trim();

        const originalText = button.textContent;

        if (ssid === "") {
            alert("SSID cannot be empty!");
            ssidField.value = "";
            passwordField.value = "";
            return;
        }

        button.disabled = true;
        button.textContent = "Connecting...";

        const data = new URLSearchParams();

        // Only append SSID and password if password is filled
        if (password !== "") {
            data.append("ssid", encodeURIComponent(ssid));
            data.append("password", encodeURIComponent(password));
        }

        data.append("timezone", encodeURIComponent(timezone));
        data.append("format24", format24);

        if (mqttServer !== "") {
            data.append("mqtt_server", encodeURIComponent(mqttServer));
        }
        if (mqttPort !== "") {
            data.append("mqtt_port", mqttPort);  // no need to encode a number
        }

        if (mqttUser !== "") {
            data.append("mqtt_user", encodeURIComponent(mqttUser));
        }
        if (mqttPassword !== "") {
            data.append("mqtt_pass", encodeURIComponent(mqttPassword));
        }

        fetch("/submit", {
            method: "POST",
            headers: {"Content-Type": "application/x-www-form-urlencoded"},
            body: data.toString(),
        })
            .then(response => {
                if (!response.ok) {
                    throw new Error("Failed to save settings");
                }
                return response.json();
            })
            .then(() => {
                passwordField.value = "";
            })
            .catch(error => {
                alert(error.message);
            })
            .finally(() => {
                button.textContent = "Saving..";
                setTimeout(() => {
                    button.disabled = false;
                    button.textContent = originalText;
                }, 500);
            });
    }

    // Bind interactions
    document.getElementById("settingsBtn").addEventListener("click", toggleSettings);
    document.getElementById("settingsCloseBtn").addEventListener("click", toggleSettings);
    changeWifiLink.addEventListener("click", function (e) {
        e.preventDefault();
        enableWifiEdit();
    });

    // Enter key triggers save
    [ssidField, passwordField].forEach(field => {
        field.addEventListener("keypress", function (event) {
            if (event.key === "Enter") {
                event.preventDefault();
                saveSettings();
            }
        });
    });

    button.addEventListener("click", saveSettings);

    // Load settings at startup
    loadConfig();
});
