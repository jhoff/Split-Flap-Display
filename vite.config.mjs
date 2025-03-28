import { defineConfig } from 'vite'
import { minify } from 'html-minifier-terser';
import tailwindcss from '@tailwindcss/vite'
import jsonminify from 'jsonminify'
import fs from 'fs'
import path from 'path'

export default defineConfig({
  root: 'src/web',
  build: {
    outDir: '../../build/web',
    assetsDir: '.',
    rollupOptions: {
        output: {
            entryFileNames: `[name].js`,
            chunkFileNames: `[name].js`,
            assetFileNames: `[name].[ext]`,
        },
    },
  },
  plugins: [
    tailwindcss(),
    {
      name: 'minify-copy-html-json',
      closeBundle() {
        const srcDir = path.resolve(__dirname, 'src/web');
        const destDir = path.resolve(__dirname, 'build/web');

        fs.readdirSync(srcDir).forEach(file => {
          const transform = async (file, callback) => {
            const filePath = path.join(srcDir, file);
            const destPath = path.join(destDir, file);

            const content = fs.readFileSync(filePath, 'utf8');
            const modifiedContent = await callback(content);

            fs.mkdirSync(path.dirname(destPath), { recursive: true });
            fs.writeFileSync(destPath, modifiedContent, 'utf8');
          }

          if (file.endsWith('.json')) {
            transform(file, jsonminify);
          }

          if (file.endsWith('.html')) {
            transform(file, async content => await minify(content, {
              collapseWhitespace: true,
              removeComments: true,
              minifyCSS: true,
              minifyJS: true,
            }));
          }
        });
      },
    },
  ],
})
