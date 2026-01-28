import Alpine from "alpinejs";
import common from "./common.js";
import settings from "./settings.js";
import controls from "./controls.js";
// import calibration from "./calibration.js";

window.Alpine = Alpine;

document.addEventListener("alpine:init", () => {
    Alpine.data("page", (type) => ({
        ...common(),
        ...settings(),
        ...controls(),
        // ...calibration(),

        init() {
            this.loadSettings();
            if (type === "Settings") {
                this.loadTimezones();
            }
        },
    }));

    Alpine.data("helpModal", () => ({
        visible: false,
        title: "",
        content: "",

        open({ title, content }) {
            this.title = title;
            this.content = content;
            this.visible = true;
        },

        close() {
            this.visible = false;
            this.title = "";
            this.content = "";
        },
    }));
});

Alpine.start();
