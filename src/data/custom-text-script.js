document.addEventListener("DOMContentLoaded", function () {
    const inputField = document.getElementById("custom-text");
    const button = document.getElementById("sendBtn");
    const modeToggle = document.getElementById("word-toggle");
    const centreToggle = document.getElementById("centre-toggle");
    const delayField = document.getElementById("delay")
    modeToggle.checked=false;
    centreToggle.checked=true;

    function sendData() {
        var originalContent = button.textContent;

        if ((delayField.value.trim() === "") && (modeToggle.checked)) {
            alert("Delay cannot be empty!");
            button.textContent = originalContent;
            ssidField.value = "";
            passwordField.value = "";
            button.disabled = false;
            return; // Prevent further execution
        }

        button.disabled = true;
        button.textContent = "Updating...";
        var centreChecked = centreToggle.checked? 1 : 0;

        if (modeToggle.checked) {
            // If the modeToggle is checked, send multiple words data
            body = "inputType=multiple&words=" + encodeURIComponent(words.join(",")) + "&delay=" + encodeURIComponent(delayField.value)+"&centering="+centreChecked;
        } else {
            // If the modeToggle is unchecked, send single word data
            body = "inputType=single&word=" + encodeURIComponent(inputField.value)+"&centering="+centreChecked;
        }

        fetch("/submit", {
            method: "POST",
            headers: { "Content-Type": "application/x-www-form-urlencoded" },
            body: body,
        })
        .then(response => {
            if (!response.ok) {
                throw new Error("Failed to update text");
            }
            return response.json();
        })
        .then(data => {
            inputField.value = "";
            // alert("Text updated: " + inputText);
        })
        .catch(error => {
            alert(error.message);
        })
        .finally(() => {
            button.disabled = false;
            button.textContent = originalContent;
        });
    }

    // Submit when Enter key is pressed
    inputField.addEventListener("keypress", function (event) {
        if (event.key === "Enter") {
            event.preventDefault(); // Prevent form submission (if inside a form)
            sendData();
        }
    });

    // Attach click event to button
    button.addEventListener("click", sendData);
});