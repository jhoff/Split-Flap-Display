document.addEventListener("DOMContentLoaded", function () {
    const modeDropDown = document.getElementById("modeDropDown");
    const sendButton = document.getElementById("sendBtn");
    const inputField = document.getElementById("custom-text");
    const modeToggle = document.getElementById("word-toggle");
    const centreToggle = document.getElementById("centre-toggle");
    const delayField = document.getElementById("delay");
    let words = [];

    function sendData() {
        const selectedMode = modeDropDown.value;
        let body = new URLSearchParams();

        sendButton.disabled = true;
        sendButton.textContent = "Updating...";

        if (selectedMode === "Custom") {
            const centreChecked = centreToggle.checked ? 1 : 0;

            if (modeToggle.checked) {
                if (delayField.value.trim() === "") {
                    alert("Delay cannot be empty!");
                    resetButton();
                    return;
                }
                body.append("inputType", "multiple");
                body.append("words", encodeURIComponent(words.join(",")));
                body.append("delay", encodeURIComponent(delayField.value));
                body.append("centering", centreChecked);
            } else {
                body.append("inputType", "single");
                body.append("word", encodeURIComponent(inputField.value));
                body.append("centering", centreChecked);
            }
        } else {
            body.append("mode", encodeURIComponent(selectedMode));
        }

        fetch("/submit", {
            method: "POST",
            headers: {"Content-Type": "application/x-www-form-urlencoded"},
            body: body.toString(),
        })
            .then((response) => {
                if (!response.ok) throw new Error("Failed to update");
                return response.json();
            })
            .then(() => {
                if (selectedMode === "Custom") inputField.value = "";
            })
            .catch((error) => alert(error.message))
            .finally(resetButton);
    }

    function resetButton() {
        setTimeout(() => {
            sendButton.disabled = false;
            sendButton.textContent = "Update Display";
        }, 500);
    }

    function toggleWordMode() {
        const singleContainer = document.getElementById("single-word-container");
        const multiContainer = document.getElementById("multi-word-container");

        if (modeToggle.checked) {
            singleContainer.classList.add("hidden");
            multiContainer.classList.remove("hidden");
        } else {
            singleContainer.classList.remove("hidden");
            multiContainer.classList.add("hidden");
        }
    }

    function addWord() {
        const wordInput = document.getElementById("word-input");
        if (wordInput.value.trim() !== "") {
            words.push(wordInput.value.trim());
            wordInput.value = "";
            updateTags();
        }
    }

    function removeWord(index) {
        words.splice(index, 1);
        updateTags();
    }

    function updateTags() {
        const tagContainer = document.getElementById("tag-container");
        tagContainer.innerHTML = "";

        words.forEach((word, index) => {
            let tag = document.createElement("span");
            tag.classList.add("tag");
            tag.innerHTML = `${word} <button data-index="${index}">×</button>`;
            tagContainer.appendChild(tag);
        });

        tagContainer.querySelectorAll("button").forEach((btn) => {
            btn.addEventListener("click", () => {
                removeWord(parseInt(btn.getAttribute("data-index")));
            });
        });
    }

    function handleModeChange() {
        const customOptions = document.querySelector(".options-custom");
        customOptions.style.display = modeDropDown.value === "Custom" ? "flex" : "none";
    }

    // Bind events
    modeDropDown.addEventListener("change", handleModeChange);
    document.getElementById("sendBtn").addEventListener("click", sendData);
    modeToggle.addEventListener("change", toggleWordMode);
    document.getElementById("add-word-btn")?.addEventListener("click", addWord);

    // Init
    handleModeChange();
});
