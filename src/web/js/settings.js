export default () => ({
    init() {
        console.log("Settings JS v1.1 Loaded - Defensive Patch Active");
    },
    settings: {
        mode: 2,
        transitionType: 0,
        dateFormat: "{ddd} {dd}/{MM}",
        timeFormat: "{HH}:{mm}",
        moduleCount: 8,
        moduleAddresses: "",
        moduleOffsets: "",
    },
    errors: {},
    timezones: {},

    addressList: [],
    offsetList: [],

    get header() {
        return this.settings.name || "Split Flap";
    },

    setAddress(index, value) {
        this.addressList[index] = value;
        // Sync back to settings string
        this.settings.moduleAddresses = this.addressList
            .filter((x) => x !== "" && x !== undefined && x !== null)
            .join(",");
    },

    setOffset(index, value) {
        this.offsetList[index] = value;
        // Sync back to settings string
        this.settings.moduleOffsets = this.offsetList
            .filter((x) => x !== "" && x !== undefined && x !== null)
            .join(",");
    },

    loadSettings() {
        this.loading.settings = true;
        fetch("/settings")
            .then((res) => res.json())
            .then((data) => {
                Object.assign(this.settings, data);

                // Manually parse strings to arrays for UI
                const parseList = (str) => {
                    if (Array.isArray(str)) return str;
                    if (!str) return [];
                    return String(str).split(",").map(s => s.trim());
                };

                this.addressList = parseList(this.settings.moduleAddresses);
                this.offsetList = parseList(this.settings.moduleOffsets);
            })
            .catch(() =>
                this.showDialog("Failed to load settings", "error", true),
            )
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
                this.showDialog(
                    "Failed to load timezones. Refresh the page.",
                    "error",
                    true,
                ),
            )
            .finally(() => (this.loading.timezones = false));
    },

    save() {
        this.processing = true;
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
            .catch(() =>
                this.showDialog("Failed to save settings.", "error"),
            )
            .finally(() => (this.processing = false));
    },

    reset() {
        if (
            confirm("Are you sure you want to reset settings to defaults?")
        ) {
            fetch("/settings/reset", { method: "POST" })
                .then((res) => res.json())
                .then((data) => {
                    this.showDialog(
                        data.message,
                        data.type,
                        data.persistent,
                    );
                    this.loadSettings();
                })
                .catch(() => {
                    this.showDialog("Failed to reset settings.", "error");
                });
        }
    },
    testingModule: null,
    processing: false, // Matches settings.html binding

    updateModuleOffset(index, change) {
        if (this.testingModule !== null) return;

        console.log("updateModuleOffset called", index, change);

        // Ensure list exists
        if (!this.offsetList || !Array.isArray(this.offsetList)) {
            console.log("offsetList missing or invalid, resetting");
            this.offsetList = [];
        }

        // Update local state temporarily for UI feedback
        let val = this.offsetList[index];
        let currentOffset = parseInt((val === undefined || val === null || val === "") ? 0 : val);
        console.log("currentOffset", currentOffset);

        let newOffset = currentOffset + change;
        this.offsetList[index] = newOffset;

        // Sync to settings object to ensure save captures it
        this.setOffset(index, newOffset);

        // Send to backend
        fetch(`/api/module/${index}/offset`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ offset: newOffset }),
        })
            .then((res) => res.json())
            .then((data) => {
                if (data.type === 'success') {
                    // Success feedback
                } else {
                    this.showDialog(data.message, "error");
                    // Revert on failure
                    this.offsetList[index] = currentOffset;
                    this.setOffset(index, currentOffset);
                }
            })
            .catch(() => {
                this.showDialog("Failed to update offset", "error");
                // Revert on failure
                this.offsetList[index] = currentOffset;
                this.setOffset(index, currentOffset);
            });
    },

    testModule(index) {
        if (this.testingModule !== null) return;

        this.testingModule = index;

        fetch(`/api/module/${index}/test`, {
            method: "POST",
        })
            .then((res) => res.json())
            .then((data) => {
                this.showDialog(data.message, data.type);
            })
            .catch(() => {
                this.showDialog("Failed to test module", "error");
            })
            .finally(() => {
                this.testingModule = null;
            });
    },
});
