import type { Plugin } from 'vite';
import type { OutputOptions } from 'rollup';

function normalizeOutputOptions(output: OutputOptions): OutputOptions {
	const { assetFileNames, chunkFileNames, entryFileNames } = output;
	if (typeof assetFileNames !== 'string') return output;

	const normalized: OutputOptions = {
		...output,
		assetFileNames: assetFileNames.replace('.[hash]', '')
	};

	if (typeof chunkFileNames === 'string' && chunkFileNames.includes('hash')) {
		normalized.chunkFileNames = chunkFileNames.replace('.[hash]', '');
		normalized.entryFileNames =
			typeof entryFileNames === 'string' ? entryFileNames.replace('.[hash]', '') : entryFileNames;
	}

	return normalized;
}

export default function viteLittleFS(): Plugin[] {
	return [
		{
			name: 'vite-plugin-littlefs',
			enforce: 'post',
			apply: 'build',

			async config(config) {
				const buildConfig = config.build?.rollupOptions;
				const output = buildConfig?.output;
				if (!buildConfig || !output) return;

				if (Array.isArray(output)) {
					buildConfig.output = output.map(normalizeOutputOptions);
				} else {
					buildConfig.output = normalizeOutputOptions(output);
				}
			}
		}
	];
}
