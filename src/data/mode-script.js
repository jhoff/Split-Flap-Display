document.addEventListener("DOMContentLoaded", function () {
    const inputField = document.getElementById("modeDropDown");
    const button = document.getElementById("sendBtn");

    function sendData() {
        var originalContent = button.textContent;

        button.disabled = true;
        button.textContent = "Updating...";
        
        const body = new URLSearchParams();
        body.append("mode", encodeURIComponent(inputField.value));

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
            // inputField.value = "";
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

    // // Submit when Enter key is pressed
    // inputField.addEventListener("keypress", function (event) {
    //     if (event.key === "Enter") {
    //         event.preventDefault(); // Prevent form submission (if inside a form)
    //         sendData();
    //     }
    // });

    // Attach click event to button
    button.addEventListener("click", sendData);
});