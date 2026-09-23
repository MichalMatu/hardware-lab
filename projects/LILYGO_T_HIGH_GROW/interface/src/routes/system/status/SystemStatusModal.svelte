<script lang="ts">
	import { fly } from 'svelte/transition';
	import { focusTrap } from 'svelte-focus-trap';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { socket as _socket } from '$lib/stores/socket';
	import Spinner from '$lib/components/Spinner.svelte';
	import Cancel from '~icons/tabler/x';
	import CPU from '~icons/tabler/cpu';
	import CPP from '~icons/tabler/binary';
	import Power from '~icons/tabler/reload';
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

	interface Props {
		isOpen: boolean;
		onClose: () => void;
	}

	let { isOpen = $bindable(false), onClose }: Props = $props();

	let systemInformation = $state<SystemInformation | null>(null);

	const statIconClass = 'w-4 h-4 flex-none opacity-60';

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

	$effect(() => {
		if (isOpen) {
			getSystemStatus();
		}
	});

	function convertSeconds(seconds: number) {
		let minutes = Math.floor(seconds / 60);
		let hours = Math.floor(minutes / 60);
		let days = Math.floor(hours / 24);

		hours = hours % 24;
		minutes = minutes % 60;
		seconds = seconds % 60;

		let result = '';
		if (days > 0) result += days + 'd ';
		if (hours > 0) result += hours + 'h ';
		if (minutes > 0) result += minutes + 'm ';
		result += seconds + 's';

		return result;
	}
</script>

{#if isOpen}
	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center p-4"
		transition:fly={{ y: 50 }}
		use:focusTrap
	>
		<div
			class="pointer-events-auto rounded-box bg-base-100 shadow-secondary/30 flex max-h-[90vh] w-full max-w-4xl flex-col shadow-2xl"
		>
			<!-- Header -->
			<div class="flex items-center justify-between border-b border-base-300 px-4 py-3">
				<div class="flex items-center gap-2">
					<Health class="w-5 h-5" />
					<h2 class="text-base font-bold">System Status</h2>
				</div>
				<button class="btn btn-ghost btn-sm btn-circle" onclick={onClose}>
					<Cancel class="w-4 h-4" />
				</button>
			</div>

			<!-- Content -->
			<div class="overflow-y-auto p-4">
				{#await getSystemStatus()}
					<div class="flex justify-center items-center p-8">
						<Spinner />
					</div>
				{:then _result}
					{#if systemInformation}
						<div class="grid grid-cols-2 gap-x-4 gap-y-1 text-xs">
							<!-- Uptime -->
							<div class="flex items-center gap-2 rounded bg-base-200 px-2 py-1">
								<Stopwatch class={statIconClass} />
								<span class="font-semibold min-w-[80px]">Uptime:</span>
								<span class="opacity-75">{convertSeconds(systemInformation.uptime)}</span>
							</div>

							<!-- Core Temperature -->
							<div class="flex items-center gap-2 rounded bg-base-200 px-2 py-1">
								<Temperature class={statIconClass} />
								<span class="font-semibold min-w-[80px]">Core Temp:</span>
								<span class="opacity-75">
									{systemInformation.core_temp == 53.33
										? 'NaN'
										: systemInformation.core_temp.toFixed(2) + ' °C'}
								</span>
							</div>

							<!-- Memory -->
							<div class="flex items-center gap-2 rounded bg-base-200 px-2 py-1">
								<Heap class={statIconClass} />
								<span class="font-semibold min-w-[80px]">Memory:</span>
								<span class="opacity-75 truncate">
									{(
										((systemInformation.total_heap - systemInformation.free_heap) /
											systemInformation.total_heap) *
										100
									).toFixed(1)}% ({Math.round(systemInformation.free_heap / 1000)}KB free)
								</span>
							</div>

							<!-- Reset Reason -->
							<div class="flex items-center gap-2 rounded bg-base-200 px-2 py-1">
								<Power class={statIconClass} />
								<span class="font-semibold min-w-[80px]">Reset:</span>
								<span class="opacity-75 truncate">{systemInformation.cpu_reset_reason}</span>
							</div>

							<!-- PSRAM (conditional) -->
							{#if systemInformation.psram_size}
								<div class="flex items-center gap-2 rounded bg-base-200 px-2 py-1">
									<Pyramid class={statIconClass} />
									<span class="font-semibold min-w-[80px]">PSRAM:</span>
									<span class="opacity-75 truncate">
										{((systemInformation.used_psram / systemInformation.psram_size) * 100).toFixed(
											1
										)}% ({Math.round(systemInformation.free_psram / 1000)}KB free)
									</span>
								</div>
							{/if}

							<!-- Sketch -->
							<div class="flex items-center gap-2 rounded bg-base-200 px-2 py-1">
								<Sketch class={statIconClass} />
								<span class="font-semibold min-w-[80px]">Sketch:</span>
								<span class="opacity-75">
									{Math.round(systemInformation.sketch_size / 1000)} KB
								</span>
							</div>

							<!-- File System -->
							<div class="flex items-center gap-2 rounded bg-base-200 px-2 py-1">
								<Folder class={statIconClass} />
								<span class="font-semibold min-w-[80px]">FS:</span>
								<span class="opacity-75 truncate">
									{((systemInformation.fs_used / systemInformation.fs_total) * 100).toFixed(1)}% ({Math.round(
										(systemInformation.fs_total - systemInformation.fs_used) / 1000
									)}KB free)
								</span>
							</div>

							<!-- Firmware Version -->
							<div class="flex items-center gap-2 rounded bg-base-200 px-2 py-1">
								<CPP class={statIconClass} />
								<span class="font-semibold min-w-[80px]">Firmware:</span>
								<span class="opacity-75 truncate">{systemInformation.firmware_version}</span>
							</div>

							<!-- Chip -->
							<div class="flex items-center gap-2 rounded bg-base-200 px-2 py-1">
								<CPU class={statIconClass} />
								<span class="font-semibold min-w-[80px]">Chip:</span>
								<span class="opacity-75">
									{systemInformation.cpu_type} Rev {systemInformation.cpu_rev}
								</span>
							</div>

							<!-- SDK Version -->
							<div class="flex items-center gap-2 rounded bg-base-200 px-2 py-1 col-span-2">
								<SDK class={statIconClass} />
								<span class="font-semibold min-w-[80px]">SDK:</span>
								<span class="opacity-75 truncate">
									ESP-IDF {systemInformation.sdk_version} / Arduino {systemInformation.arduino_version}
								</span>
							</div>

							<!-- CPU Frequency -->
							<div class="flex items-center gap-2 rounded bg-base-200 px-2 py-1">
								<Speed class={statIconClass} />
								<span class="font-semibold min-w-[80px]">CPU:</span>
								<span class="opacity-75">
									{systemInformation.cpu_freq_mhz} MHz {systemInformation.cpu_cores == 2
										? 'Dual Core'
										: 'Single Core'}
								</span>
							</div>

							<!-- Flash Chip -->
							<div class="flex items-center gap-2 rounded bg-base-200 px-2 py-1">
								<Flash class={statIconClass} />
								<span class="font-semibold min-w-[80px]">Flash:</span>
								<span class="opacity-75">
									{Math.round(systemInformation.flash_chip_size / 1000)} KB / {(
										systemInformation.flash_chip_speed / 1000000
									).toFixed(0)} MHz
								</span>
							</div>
						</div>
					{/if}
				{/await}
			</div>
		</div>
	</div>

	<!-- Backdrop -->
	<button
		class="fixed inset-0 z-40 cursor-default bg-black/50 backdrop-blur-sm"
		onclick={onClose}
		aria-label="Close modal"
	></button>
{/if}
