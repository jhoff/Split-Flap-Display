import Alpine from 'alpinejs'
window.Alpine = Alpine

document.addEventListener('alpine:init', () => {
    Alpine.data('page', (type) => ({
        get header() {
            return (this.settings.name || 'Split Flap') + ' ' + type;
        },

        loading: {
            settings: true,
        },

        dialog: {
            show: false,
            message: "",
            type: null,
        },

        settings: {},
        errors: {},

        init() {
            this.loadSettings();
        },

        loadSettings() {
            fetch('/settings')
                .then(response => response.json())
                .then(data => {
                    Object.keys(data).forEach(key => {
                        this.settings[key] = data[key];
                    });
                })
                .catch(error => this.showDialog('Failed to load settings. Refresh the page.', 'error', true))
                .finally(() => this.loading.settings = false);
        },

        showDialog(message, type, persistent = false) {
            this.dialog.message = message;
            this.dialog.type = type;
            this.dialog.show = true;
            if (! persistent) {
                setTimeout(() => this.dialog.show = false, 3000);
            }
        },
    }));
});

Alpine.start()
