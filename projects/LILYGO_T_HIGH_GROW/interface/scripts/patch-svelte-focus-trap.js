#!/usr/bin/env node
// Adds an exports condition to svelte-focus-trap so Vite/Svelte stops warning about missing exports.

import fs from 'fs';
import path from 'path';

const pkgPath = path.join(process.cwd(), 'node_modules', 'svelte-focus-trap', 'package.json');

if (!fs.existsSync(pkgPath)) {
	console.warn('svelte-focus-trap not installed; skip patch');
	process.exit(0);
}

const pkgRaw = fs.readFileSync(pkgPath, 'utf8');
let pkg;
try {
	pkg = JSON.parse(pkgRaw);
} catch (err) {
	console.error('Failed to parse svelte-focus-trap package.json', err);
	process.exit(1);
}

const current = pkg.exports?.['.'];
if (current && current.svelte) {
	console.log('svelte-focus-trap already has exports.svelte; nothing to do');
	process.exit(0);
}

const hasDist = fs.existsSync(path.join(path.dirname(pkgPath), 'dist', 'index.mjs'));

pkg.exports = {
	'.': {
		svelte: './src/index.js',
		import: hasDist ? './dist/index.mjs' : './src/index.js',
		require: hasDist ? './dist/index.js' : './src/index.js',
		default: hasDist ? './dist/index.mjs' : './src/index.js'
	},
	'./src/index.js': './src/index.js'
};

fs.writeFileSync(pkgPath, JSON.stringify(pkg, null, 2));
console.log('Patched svelte-focus-trap exports for Vite/Svelte');
