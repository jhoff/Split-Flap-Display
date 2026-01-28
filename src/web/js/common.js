export default () => ({
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

    get processing() {
        return (
            this.saving || this.loading.settings || this.loading.timezones
        );
    },

    showDialog(message, type = "success", persistent = false) {
        this.dialog.message = message;
        this.dialog.type = type;
        this.dialog.show = true;

        if (!persistent) {
            setTimeout(() => (this.dialog.show = false), 3000);
        }
    },
});
