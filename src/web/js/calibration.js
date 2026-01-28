export default () => ({
    testingModule: null,

    async testModule(moduleIndex) {
        this.testingModule = moduleIndex;
        try {
            const response = await fetch(
                `/api/module/${moduleIndex}/test`,
                {
                    method: "POST",
                },
            );
            const data = await response.json();
            if (data.type === "success") {
                this.showDialog(
                    `Module ${moduleIndex} test complete`,
                    "success",
                );
            } else {
                this.showDialog(data.message || "Test failed", "error");
            }
        } catch (error) {
            console.error("Test failed:", error);
            this.showDialog("Test failed", "error");
        } finally {
            this.testingModule = null;
        }
    },

    async updateModuleOffset(moduleIndex, delta) {
        const currentOffset = parseInt(this.offsetArray[moduleIndex]) || 0;
        const newOffset = currentOffset + delta;

        this.setOffset(moduleIndex, newOffset);

        try {
            const response = await fetch(
                `/api/module/${moduleIndex}/offset`,
                {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({ offset: newOffset }),
                },
            );
            const data = await response.json();
            if (data.type !== "success") {
                console.error("Offset update failed:", data.message);
                this.showDialog(
                    data.message || "Offset update failed",
                    "error",
                );
            }
        } catch (error) {
            console.error("Offset update failed:", error);
            this.showDialog("Offset update failed", "error");
        }
    },
});
