<script lang="ts">
	import { onMount } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import Temperature from '~icons/tabler/temperature';
	import Droplet from '~icons/tabler/droplet';
	import Sun from '~icons/tabler/sun';
	import Leaf from '~icons/tabler/leaf';
	import Battery from '~icons/tabler/battery';
	import Adjustments from '~icons/tabler/adjustments';
	import DeviceFloppy from '~icons/tabler/device-floppy';

	const securityEnabled = page.data.features.security;

	let config = {
		tempOffset: 0,
		humidOffset: 0,
		luxOffset: 0,
		soilOffset: 0,
		soilMin: 1391,
		soilMax: 3300,
		batAdcMin: 1860,
		batAdcMax: 2700
	};

	const authHeaders = (): Record<string, string> => {
		if (!securityEnabled) return {};
		return { Authorization: `Bearer ${$user.bearer_token}` };
	};

	onMount(() => {
		fetch('/api/config', { headers: authHeaders() })
			.then((res) => res.json())
			.then((data) => (config = data))
			.catch((err) => console.error('Config fetch error:', err));
	});

	function saveConfig() {
		fetch('/api/config', {
			method: 'POST',
			headers: { 'Content-Type': 'application/json', ...authHeaders() },
			body: JSON.stringify(config)
		})
			.then(() => notifications.success('Configuration saved!', 3000))
			.catch((err) => notifications.error('Error: ' + err, 5000));
	}
</script>

<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-4 max-[320px]:px-2">
	<div class="grid grid-cols-1 md:grid-cols-2 gap-4">
		<div class="card bg-base-200 shadow-primary/50 shadow-lg">
			<div class="card-body p-4">
				<h2 class="card-title text-lg mb-2"><Adjustments class="w-6 h-6" /> Offsets</h2>

				<div class="grid grid-cols-3 gap-2 items-center mb-2">
					<label class="label-text col-span-2 flex items-center" for="tempOffset"
						><Temperature class="w-4 h-4 mr-2" /> Temperature Offset (°C)</label
					>
					<input
						id="tempOffset"
						type="number"
						step="0.1"
						bind:value={config.tempOffset}
						class="input input-bordered input-sm w-full"
					/>
				</div>

				<div class="grid grid-cols-3 gap-2 items-center mb-2">
					<label class="label-text col-span-2 flex items-center" for="humidOffset"
						><Droplet class="w-4 h-4 mr-2" /> Humidity Offset (%)</label
					>
					<input
						id="humidOffset"
						type="number"
						step="0.1"
						bind:value={config.humidOffset}
						class="input input-bordered input-sm w-full"
					/>
				</div>

				<div class="grid grid-cols-3 gap-2 items-center mb-2">
					<label class="label-text col-span-2 flex items-center" for="luxOffset"
						><Sun class="w-4 h-4 mr-2" /> Light Offset (lux)</label
					>
					<input
						id="luxOffset"
						type="number"
						step="1"
						bind:value={config.luxOffset}
						class="input input-bordered input-sm w-full"
					/>
				</div>

				<div class="grid grid-cols-3 gap-2 items-center mb-2">
					<label class="label-text col-span-2 flex items-center" for="soilOffset"
						><Leaf class="w-4 h-4 mr-2" /> Soil Offset (%)</label
					>
					<input
						id="soilOffset"
						type="number"
						step="0.1"
						bind:value={config.soilOffset}
						class="input input-bordered input-sm w-full"
					/>
				</div>
			</div>
		</div>

		<div class="card bg-base-200 shadow-primary/50 shadow-lg">
			<div class="card-body p-4">
				<h2 class="card-title text-lg mb-2"><Adjustments class="w-6 h-6" /> Calibration Ranges</h2>

				<div class="grid grid-cols-3 gap-2 items-center mb-2">
					<label class="label-text col-span-2 flex items-center" for="soilMin"
						><Leaf class="w-4 h-4 mr-2" /> Soil Min (wet ADC)</label
					>
					<input
						id="soilMin"
						type="number"
						bind:value={config.soilMin}
						class="input input-bordered input-sm w-full"
					/>
				</div>

				<div class="grid grid-cols-3 gap-2 items-center mb-2">
					<label class="label-text col-span-2 flex items-center" for="soilMax"
						><Leaf class="w-4 h-4 mr-2" /> Soil Max (dry ADC)</label
					>
					<input
						id="soilMax"
						type="number"
						bind:value={config.soilMax}
						class="input input-bordered input-sm w-full"
					/>
				</div>

				<div class="grid grid-cols-3 gap-2 items-center mb-2">
					<label class="label-text col-span-2 flex items-center" for="batAdcMin"
						><Battery class="w-4 h-4 mr-2" /> Battery ADC Min (empty)</label
					>
					<input
						id="batAdcMin"
						type="number"
						bind:value={config.batAdcMin}
						class="input input-bordered input-sm w-full"
					/>
				</div>

				<div class="grid grid-cols-3 gap-2 items-center mb-2">
					<label class="label-text col-span-2 flex items-center" for="batAdcMax"
						><Battery class="w-4 h-4 mr-2" /> Battery ADC Max (full)</label
					>
					<input
						id="batAdcMax"
						type="number"
						bind:value={config.batAdcMax}
						class="input input-bordered input-sm w-full"
					/>
				</div>
			</div>
		</div>
	</div>

	<div class="mt-4 flex justify-end">
		<button class="btn btn-primary btn-sm" on:click={saveConfig}>
			<DeviceFloppy class="w-4 h-4" /> Save Configuration
		</button>
	</div>
</div>
