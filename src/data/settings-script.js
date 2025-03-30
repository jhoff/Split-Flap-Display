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
        const modal = document.getElementById("settingsModal");
        modal.classList.toggle("hidden");

        // Prevent background scroll when modal is open
        if (!modal.classList.contains("hidden")) {
            document.body.classList.add("modal-open");
        } else {
            document.body.classList.remove("modal-open");
        }
    }

    function enableWifiEdit() {
        ssidField.readOnly = false;
        document.getElementById("wifi-password-group").style.display = "block";
        changeWifiLink.style.display = "none";
    }

    function loadConfig() {
        const timezoneOptions = [
            { label: "UTC (GMT±0)", value: "UTC0" },
            { label: "Central European Time (GMT+1)", value: "CET-1CEST,M3.5.0/2,M10.5.0/3" },
            { label: "Eastern European Time (GMT+2)", value: "EET-2EEST,M3.5.0/3,M10.5.0/4" },
            { label: "Moscow / East Africa (GMT+3)", value: "MSK-3" },
            { label: "Gulf Standard Time (GMT+4)", value: "GST-4" },
            { label: "Pakistan Standard Time (GMT+5)", value: "PKT-5" },
            { label: "Bangladesh / Bhutan (GMT+6)", value: "BST-6" },
            { label: "Thailand / Vietnam (GMT+7)", value: "ICT-7" },
            { label: "China / Singapore / Perth (GMT+8)", value: "CST-8" },
            { label: "Japan / Korea (GMT+9)", value: "JST-9" },
            { label: "Sydney / Guam (GMT+10)", value: "AEST-10AEDT,M10.1.0,M4.1.0/3" },
            { label: "Solomon Islands (GMT+11)", value: "SBT-11" },
            { label: "New Zealand / Fiji (GMT+12)", value: "NZST-12NZDT,M9.5.0,M4.1.0/3" },
            { label: "Azores (GMT-1)", value: "AZOT1AZOST,M3.5.0/0,M10.5.0/1" },
            { label: "South Georgia (GMT-2)", value: "GST2" },
            { label: "Brazil / Argentina (GMT-3)", value: "BRT3BRST,M10.3.0/0,M2.3.0/0" },
            { label: "Atlantic / Venezuela (GMT-4)", value: "AST4" },
            { label: "Eastern US / Colombia (GMT-5)", value: "EST5EDT,M3.2.0/2,M11.1.0/2" },
            { label: "Central US / Mexico City (GMT-6)", value: "CST6CDT,M3.2.0/2,M11.1.0/2" },
            { label: "Mountain US / Canada (GMT-7)", value: "MST7MDT,M3.2.0/2,M11.1.0/2" },
            { label: "Pacific US / Canada (GMT-8)", value: "PST8PDT,M3.2.0/2,M11.1.0/2" },
            { label: "Alaska (GMT-9)", value: "AKST9AKDT,M3.2.0/2,M11.1.0/2" },
            { label: "Hawaii / Tahiti (GMT-10)", value: "HST10" },
            { label: "Niue (GMT-11)", value: "NUT11" },
            { label: "Baker Island (GMT-12)", value: "BIT12" },
        ];

        timezoneField.innerHTML = timezoneOptions.map(option =>
            `<option value="${option.value}">${option.label}</option>`
        ).join("");

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

                // Set value after options are populated
                timezoneField.value = config.timezone;

                initialConfig = {
                    ssid: config.ssid,
                    timezone: config.timezone,
                    format24: config.format24,
                    mqtt_server: config.mqtt_server,
                    mqtt_port: config.mqtt_port,
                    mqtt_user: config.mqtt_user
                    // password intentionally excluded
                };

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

        if (timezone !== initialConfig.timezone) {
            data.append("timezone", encodeURIComponent(timezone));
        }

        if (format24 !== initialConfig.format24) {
            data.append("format24", format24);
        }

        if (mqttServer !== initialConfig.mqtt_server) {
            data.append("mqtt_server", encodeURIComponent(mqttServer));
        }
        if (mqttPort !== String(initialConfig.mqtt_port)) {
            data.append("mqtt_port", mqttPort);
        }
        if (mqttUser !== initialConfig.mqtt_user) {
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

    document.addEventListener("keydown", function (e) {
        if (e.key === "Escape") {
            const modal = document.getElementById("settingsModal");
            if (!modal.classList.contains("hidden")) {
                modal.classList.add("hidden");
                document.body.classList.remove("modal-open");
            }
        }
    });


    button.addEventListener("click", saveSettings);

    // Load settings at startup
    loadConfig();
});
