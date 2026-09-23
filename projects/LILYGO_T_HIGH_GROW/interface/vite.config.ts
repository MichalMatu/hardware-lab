import { sveltekit } from '@sveltejs/kit/vite';
import type { UserConfig } from 'vite';
import Icons from 'unplugin-icons/vite';
import viteLittleFS from './vite-plugin-littlefs';
import tailwindcss from '@tailwindcss/vite';
import { visualizer } from 'rollup-plugin-visualizer';

const config: UserConfig = {
	plugins: [
		sveltekit(),
		Icons({
			compiler: 'svelte'
		}),
		tailwindcss(),
		visualizer({ filename: 'stats.html', template: 'treemap', gzipSize: true, brotliSize: true }),
		// Shorten file names for LittleFS 32 char limit
		viteLittleFS()
	],
	resolve: {
		alias: {
			'svelte-focus-trap': 'svelte-focus-trap/src/index.js'
		}
	},
	server: {
		proxy: {
			'/api': {
				target: process.env.VITE_PROXY_TARGET || 'http://192.168.0.49',
				changeOrigin: true
			},
			'/rest': {
				target: process.env.VITE_PROXY_TARGET || 'http://192.168.0.49',
				changeOrigin: true
			},
			'/ws': {
				target: (process.env.VITE_PROXY_TARGET || 'http://192.168.0.49').replace('http', 'ws'),
				changeOrigin: true,
				ws: true
			}
		}
	},
	build: {
		minify: 'terser',
		sourcemap: false,
		rollupOptions: {
			// Note: manualChunks is not compatible with bundleStrategy: 'single' in svelte.config.js
			// The single-bundle strategy is needed for LittleFS embedding
			// output: {
			// 	manualChunks(id) {
			// 		if (!id.includes('node_modules')) return;
			// 		if (id.includes('chart.js') || id.includes('chartjs-adapter-luxon') || id.includes('luxon')) {
			// 			return 'chart';
			// 		}
			// 		if (id.includes('msgpack-lite')) {
			// 			return 'msgpack';
			// 		}
			// 		if (id.includes('@iconify') || id.includes('unplugin-icons')) {
			// 			return 'icons';
			// 		}
			// 		if (id.includes('svelte-dnd-action') || id.includes('svelte-modals')) {
			// 			return 'ui-extras';
			// 		}
			// 		return 'vendor';
			// 	}
			// }
		}
	}
};

export default config;
