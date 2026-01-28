export default () => ({
    singleMode: true,
    singleWord: "",
    multiWord: "",
    multiWords: [],
    delay: 1,
    centerText: false,

    updateDisplay() {
        if (this.settings.mode === 6) {
            if (this.delay < 1) {
                return this.showDialog(
                    "Delay must be at least 1 second.",
                    "error",
                );
            }

            if (this.singleMode && this.singleWord.trim() === "") {
                return this.showDialog(
                    "Single word cannot be empty.",
                    "error",
                );
            }

            if (!this.singleMode && this.multiWords.length === 0) {
                return this.showDialog(
                    "Word list cannot be empty.",
                    "error",
                );
            }
        }

        fetch("/settings", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ mode: this.settings.mode }),
        });

        if (this.settings.mode === 6) {
            fetch("/text", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({
                    mode: this.singleMode ? "single" : "multiple",
                    words: this.singleMode
                        ? [this.singleWord]
                        : this.multiWords,
                    delay: this.delay,
                    center: this.centerText,
                }),
            })
                .then((res) => res.json())
                .then((res) => this.showDialog(res.message, res.type))
                .catch((err) => this.showDialog(err.message, "error"));
        } else {
            this.showDialog("Mode updated successfully.", "success");
        }
    },

    addWord() {
        if (this.multiWord.trim() !== "") {
            this.multiWords.push(this.multiWord.trim());
        }
        this.multiWord = "";
    },

    removeWord(index) {
        this.multiWords.splice(index, 1);
    },
});
