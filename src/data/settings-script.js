document.addEventListener("DOMContentLoaded", function () {
  const ssidField = document.getElementById("ssid");
  const passwordField = document.getElementById("password");
  const timezoneField = document.getElementById("timezone");
  const format24Toggle = document.getElementById("format-24-toggle");
  const button = document.getElementById("settingsSaveBtn");

  function saveSettings() {
    const ssid = ssidField.value.trim();
    const password = passwordField.value.trim();
    const timezone = timezoneField.value;
    const format24 = format24Toggle.checked ? "1" : "0";

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

    // Only append password (and SSID) if password is provided
    if (password !== "") {
      data.append("ssid", encodeURIComponent(ssid));
      data.append("password", encodeURIComponent(password));
    }

    // Always append timezone and format24
    data.append("timezone", encodeURIComponent(timezone));
    data.append("format24", format24);

    fetch("/submit", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
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

  // Enter key triggers save
  [ssidField, passwordField].forEach(field => {
    field.addEventListener("keypress", function (event) {
      if (event.key === "Enter") {
        event.preventDefault();
        saveSettings();
      }
    });
  });

  // Save on button click
  button.addEventListener("click", saveSettings);
});
