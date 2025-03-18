document.addEventListener("DOMContentLoaded", function () {
    const ssidField = document.getElementById("ssid");
    const passwordField = document.getElementById("password");
    const button = document.getElementById("saveBtn");

    function sendData() {
        var originalContent = button.textContent;
        // Get values from input fields
        const ssid = ssidField.value;
        const password = passwordField.value;
        
        if (ssid.trim() === "") {
            alert("SSID cannot be empty!");
            button.textContent = originalContent;
            ssidField.value = "";
            passwordField.value = "";
            button.disabled = false;
            return; // Prevent further execution
        }
        
        button.disabled = true;
        button.textContent = "Connecting...";

        // Prepare the data to be sent
        const data = new URLSearchParams();
        data.append("ssid", encodeURIComponent(ssid));
        data.append("password", encodeURIComponent(password));

        // Send the data via POST request
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
        .then(data => {
            // ssidField.value = "";
            passwordField.value = "";
            // alert("Settings saved successfully!");

        })
        .catch(error => {
            alert(error.message);
        })
        .finally(() => {
            passwordField.value = "";
            // Change button text to "Settings Saved"
            button.textContent = "Saving..";
            // After 2 seconds, change button text back to "Save"
            setTimeout(() => {
                button.disabled = false;
                button.textContent = originalContent;
            }, 500); //ms
            // alert("Settings Saved, Connect Back To Your Network and Reload Site");
        });
    }

    // Submit when Enter key is pressed
    ssidField.addEventListener("keypress", function (event) {
        if (event.key === "Enter") {
            event.preventDefault(); // Prevent form submission (if inside a form)
            sendData();
        }
    });

    passwordField.addEventListener("keypress", function (event) {
        if (event.key === "Enter") {
            event.preventDefault(); // Prevent form submission (if inside a form)
            sendData();
        }
    });

    // Attach click event to button
    button.addEventListener("click", sendData);
});