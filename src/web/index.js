import Alpine from "alpinejs";
window.Alpine = Alpine;

document.addEventListener("alpine:init", () => {
    Alpine.data("page", (type) => ({
        get header() {
            return (this.settings.name || "Split Flap");
        },

        loading: {
            settings: true,
            timezones: true,
        },
        saving: false,
        dialog: {
            show: false,
            message: "",
            type: null,
        },
        settings: {
            mode: 2,
        },
        errors: {},
        timezones: {},

        // Control page specific
        singleMode: true,
        singleWord: "",
        multiWord: "",
        multiWords: [],
        delay: 1,
        centerText: false,

        get processing() {
            return this.saving || this.loading.settings || this.loading.timezones;
        },

        // 🆕 Helper for per-module input arrays
        get addressArray() {
            return this.settings.moduleAddresses?.split(',').map(s => s.trim()) || [];
        },
        setAddress(index, value) {
            const arr = this.addressArray;
            arr[index] = value;
            this.settings.moduleAddresses = arr.join(',');
        },

        get offsetArray() {
            return this.settings.moduleOffsets?.split(',').map(s => s.trim()) || [];
        },
        setOffset(index, value) {
            const arr = this.offsetArray;
            arr[index] = value;
            this.settings.moduleOffsets = arr.join(',');
        },

        init() {
            this.loadSettings();
            if (type === "Settings") {
                this.loadTimezones();
            }
        },

        loadSettings() {
            fetch("/settings")
                .then((res) => res.json())
                .then((data) => {
                    Object.assign(this.settings, data);
                })
                .catch(() => this.showDialog("Failed to load settings", "error", true))
                .finally(() => {
                    this.loading.settings = false;
                });
        },

        loadTimezones() {
            fetch("/timezones.json")
                .then((res) => res.json())
                .then((data) => {
                    this.timezones = data;
                })
                .catch(() =>
                    this.showDialog("Failed to load timezones. Refresh the page.", "error", true)
                )
                .finally(() => (this.loading.timezones = false));
        },

        updateDisplay() {
            if (this.settings.mode === 6) {
                if (this.delay < 1) {
                    return this.showDialog("Delay must be at least 1 second.", "error");
                }

                if (this.singleMode && this.singleWord.trim() === "") {
                    return this.showDialog("Single word cannot be empty.", "error");
                }

                if (!this.singleMode && this.multiWords.length === 0) {
                    return this.showDialog("Word list cannot be empty.", "error");
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
                        words: this.singleMode ? [this.singleWord] : this.multiWords,
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

        save() {
            this.saving = true;
            this.errors = {};

            fetch("/settings", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(this.settings),
            })
                .then((res) => res.json())
                .then((data) => {
                    this.errors = data.errors || {};
                    this.showDialog(data.message, data.type, data.persistent);
                    if (data.redirect) {
                        setTimeout(() => {
                            window.location.href = data.redirect;
                        }, 10000);
                    }
                })
                .catch(() => this.showDialog("Failed to save settings.", "error"))
                .finally(() => (this.saving = false));
        },

        reset() {
            if (confirm("Are you sure you want to reset settings to defaults?")) {
                fetch("/settings/reset", { method: "POST" })
                    .then((res) => res.json())
                    .then((data) => {
                        this.showDialog(data.message, data.type, data.persistent);
                        this.loadSettings();
                    })
                    .catch(() => {
                        this.showDialog("Failed to reset settings.", "error");
                    });
            }
        },

        showDialog(message, type = "success", persistent = false) {
            this.dialog.message = message;
            this.dialog.type = type;
            this.dialog.show = true;

            if (!persistent) {
                setTimeout(() => (this.dialog.show = false), 3000);
            }
        },
    }));
});

Alpine.start();
