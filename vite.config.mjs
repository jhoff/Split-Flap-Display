import { defineConfig } from "vite";
import tailwindcss from "@tailwindcss/vite";
import { viteStaticCopy } from "vite-plugin-static-copy";
import jsonminify from "jsonminify";
import path from "path";

export default defineConfig({
    root: "src/web",
    build: {
        outDir: "../../build/web",
        assetsDir: ".",
        emptyOutDir: true,
        rollupOptions: {
            input: {
                main: path.resolve(__dirname, "src/web/index.html"),
                settings: path.resolve(__dirname, "src/web/settings.html"),
            },
            output: {
                entryFileNames: `[name].js`,
                chunkFileNames: `[name].js`,
                assetFileNames: `[name].[ext]`,
            },
        },
    },
    plugins: [
        tailwindcss(),
        viteStaticCopy({
            targets: [
                {
                    src: "timezones.json",
                    dest: ".",
                    transform: (content) => jsonminify(content.toString()),
                },
            ],
        }),
    ],
});
