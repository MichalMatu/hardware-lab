<script lang="ts">
	import { onDestroy as _onDestroy, onMount as _onMount } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import Spinner from '$lib/components/Spinner.svelte';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import CPU from '~icons/tabler/cpu';
	import CPP from '~icons/tabler/binary';
	import Reload from '~icons/tabler/reload';
	import Speed from '~icons/tabler/activity';
	import Flash from '~icons/tabler/device-sd-card';
	import Pyramid from '~icons/tabler/pyramid';
	import Sketch from '~icons/tabler/chart-pie';
	import Folder from '~icons/tabler/folder';
	import Heap from '~icons/tabler/box-model';
	import Temperature from '~icons/tabler/temperature';
	import Health from '~icons/tabler/stethoscope';
	import Stopwatch from '~icons/tabler/24-hours';
	import SDK from '~icons/tabler/sdk';
	import type { SystemInformation } from '$lib/types/models';

	let systemInformation: SystemInformation = $state()!;

	const statIconClass = 'h-6 w-6 flex-none text-base-content/70';

	async function getSystemStatus() {
		try {
			const response = await fetch('/rest/systemStatus', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			systemInformation = await response.json();
		} catch (error) {
			console.log('Error:', error);
		}
		return systemInformation;
	}

	function convertSeconds(seconds: number) {
		// Calculate the number of seconds, minutes, hours, and days
		let minutes = Math.floor(seconds / 60);
		let hours = Math.floor(minutes / 60);
		let days = Math.floor(hours / 24);

		// Calculate the remaining hours, minutes, and seconds
		hours = hours % 24;
		minutes = minutes % 60;
		seconds = seconds % 60;

		// Create the formatted string
		let result = '';
		if (days > 0) {
			result += days + ' day' + (days > 1 ? 's' : '') + ' ';
		}
		if (hours > 0) {
			result += hours + ' hour' + (hours > 1 ? 's' : '') + ' ';
		}
		if (minutes > 0) {
			result += minutes + ' minute' + (minutes > 1 ? 's' : '') + ' ';
		}
		result += seconds + ' second' + (seconds > 1 ? 's' : '');

		return result;
	}
</script>

{#await getSystemStatus()}
	<div class="flex justify-center items-center p-8">
		<Spinner />
	</div>
{:then _result}
	<div class="grid grid-cols-1 md:grid-cols-2 gap-4">
		<!-- Left Column: System Information -->
		<div class="card bg-base-200 shadow-primary/50 shadow-lg">
			<div class="card-body p-4">
				<h2 class="card-title text-lg mb-2">
					<Health class="w-6 h-6" /> System / Software
				</h2>
				<div
					class="flex w-full flex-col space-y-1"
					transition:slide|local={{ duration: 300, easing: cubicOut }}
				>
					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<Stopwatch class={statIconClass} />
						</div>
						<div>
							<div class="font-bold">Uptime</div>
							<div class="text-sm opacity-75">
								{convertSeconds(systemInformation.uptime)}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<Heap class={statIconClass} />
						</div>
						<div>
							<div class="font-bold">Memory</div>
							<div class="text-sm opacity-75">
								{(
									((systemInformation.total_heap - systemInformation.free_heap) /
										systemInformation.total_heap) *
									100
								).toFixed(1)} % of {Math.round(systemInformation.total_heap / 1000).toLocaleString(
									'en-US'
								)} KB
								<span
									>({Math.round(systemInformation.free_heap / 1000).toLocaleString('en-US')} KB free,
									Max alloc {Math.round(systemInformation.max_alloc_heap / 1000).toLocaleString(
										'en-US'
									)} KB)</span
								>
							</div>
						</div>
					</div>

					{#if systemInformation.psram_size}
						<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
							<div class="flex h-10 w-10 items-center justify-center">
								<Pyramid class={statIconClass} />
							</div>
							<div>
								<div class="font-bold">PSRAM</div>
								<div class="text-sm opacity-75">
									{((systemInformation.used_psram / systemInformation.psram_size) * 100).toFixed(1)}
									% of {Math.round(systemInformation.psram_size / 1000).toLocaleString('en-US')} KB
									<span
										>({Math.round(systemInformation.free_psram / 1000).toLocaleString('en-US')} KB free)</span
									>
								</div>
							</div>
						</div>
					{/if}

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<Folder class={statIconClass} />
						</div>
						<div>
							<div class="font-bold">File System</div>
							<div class="flex flex-wrap justify-start gap-1 text-sm opacity-75">
								<span
									>{((systemInformation.fs_used / systemInformation.fs_total) * 100).toFixed(1)} % of
									{Math.round(systemInformation.fs_total / 1000).toLocaleString('en-US')} KB</span
								>

								<span
									>({Math.round(
										(systemInformation.fs_total - systemInformation.fs_used) / 1000
									).toLocaleString('en-US')}
									KB free)</span
								>
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<Sketch class={statIconClass} />
						</div>
						<div>
							<div class="font-bold">Sketch</div>
							<div class="text-sm opacity-75">
								{Math.round(systemInformation.sketch_size / 1000).toLocaleString('en-US')} KB
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<CPP class={statIconClass} />
						</div>
						<div>
							<div class="font-bold">Firmware Version</div>
							<div class="text-sm opacity-75">
								{systemInformation.firmware_version}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<SDK class={statIconClass} />
						</div>
						<div>
							<div class="font-bold">SDK Version</div>
							<div class="text-sm opacity-75">
								ESP-IDF {systemInformation.sdk_version} / Arduino {systemInformation.arduino_version}
							</div>
						</div>
					</div>
				</div>
			</div>
		</div>

		<!-- Right Column: Hardware Information -->
		<div class="card bg-base-200 shadow-primary/50 shadow-lg">
			<div class="card-body p-4">
				<h2 class="card-title text-lg mb-2">
					<CPU class="w-6 h-6" /> Hardware Information
				</h2>
				<div
					class="flex w-full flex-col space-y-1"
					transition:slide|local={{ duration: 300, easing: cubicOut }}
				>
					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<Temperature class={statIconClass} />
						</div>
						<div>
							<div class="font-bold">Core Temperature</div>
							<div class="text-sm opacity-75">
								{systemInformation.core_temp == 53.33
									? 'NaN'
									: systemInformation.core_temp.toFixed(2) + ' °C'}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<Reload class={statIconClass} />
						</div>
						<div>
							<div class="font-bold">Reset Reason</div>
							<div class="text-sm opacity-75">
								{systemInformation.cpu_reset_reason}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<CPU class={statIconClass} />
						</div>
						<div>
							<div class="font-bold">Chip</div>
							<div class="text-sm opacity-75">
								{systemInformation.cpu_type} Rev {systemInformation.cpu_rev}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<Speed class={statIconClass} />
						</div>
						<div>
							<div class="font-bold">CPU Frequency</div>
							<div class="text-sm opacity-75">
								{systemInformation.cpu_freq_mhz} MHz {systemInformation.cpu_cores == 2
									? 'Dual Core'
									: 'Single Core'}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<Flash class={statIconClass} />
						</div>
						<div>
							<div class="font-bold">Flash Chip</div>
							<div class="text-sm opacity-75">
								{Math.round(systemInformation.flash_chip_size / 1000).toLocaleString('en-US')} KB / {(
									systemInformation.flash_chip_speed / 1000000
								).toLocaleString('en-US')} MHz
							</div>
						</div>
					</div>
				</div>
			</div>
		</div>
	</div>
{/await}
