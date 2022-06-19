import { defineConfig } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'
import { resolve } from 'path';
import sveltePreprocess from 'svelte-preprocess'

export default defineConfig({
    plugins: [
        svelte({
            configFile: false,
            preprocess: sveltePreprocess(),
            compilerOptions: {
                dev: true,
                enableSourcemap: true
            }
        })
    ],
    build: {
        lib: {
            entry: resolve(__dirname, 'src/main.ts'),
            name: 'MckSampler',
            fileName: (format) => `bundle.${format}.js`
        },
        outDir: "../www/dist",
        emptyOutDir: true
    }
})