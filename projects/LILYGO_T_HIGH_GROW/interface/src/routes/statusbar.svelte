<script lang="ts">
	import { page } from '$app/state';
	import { telemetry } from '$lib/stores/telemetry';
	import { modals } from 'svelte-modals';
	import { user } from '$lib/stores/user';
	import { formatShortDateTimeParts } from '$lib/utils/timeFormat';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import WiFiOff from '~icons/tabler/wifi-off';
	import Hamburger from '~icons/tabler/menu-2';
	import Power from '~icons/tabler/power';
	import Cancel from '~icons/tabler/x';
	import RssiIndicator from '$lib/components/RSSIIndicator.svelte';
	import BatteryIndicator from '$lib/components/BatteryIndicator.svelte';
	import { onMount, onDestroy } from 'svelte';
	import type { ModalComponent } from 'svelte-modals';

	let currentDate = $state('');
	let currentClock = $state('');

	let fetchInterval: ReturnType<typeof setInterval>;
	let powerHeartbeatInterval: ReturnType<typeof setInterval>;

	const toModalComponent = (component: unknown) => component as unknown as ModalComponent<any>;

	onMount(() => {
		fetchESPTime();
		fetchInterval = setInterval(fetchESPTime, 60000); // Fetch from ESP every 1 minute
		heartbeatPower();
		powerHeartbeatInterval = setInterval(heartbeatPower, 15000); // keep ESP awake via power status
	});

	onDestroy(() => {
		if (fetchInterval) clearInterval(fetchInterval);
		if (powerHeartbeatInterval) clearInterval(powerHeartbeatInterval);
	});

	async function fetchESPTime() {
		try {
			const response = await fetch('/rest/ntpStatus', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			if (!response.ok) return;
			const data = await response.json();
			if (data.local_time) {
				const parts = formatShortDateTimeParts(data.local_time);
				currentDate = parts.date;
				currentClock = parts.time;
			}
		} catch (error) {
			console.error('Failed to fetch ESP time:', error);
		}
	}

	async function postSleep() {
		await fetch('/rest/sleep', {
			method: 'POST',
			headers: {
				Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
			}
		});
	}

	async function heartbeatPower() {
		try {
			await fetch('/rest/power/status', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
				}
			});
			// Refresh displayed time alongside the heartbeat to keep navbar clock fresh
			fetchESPTime();
		} catch (error) {
			console.error('Failed to heartbeat power status:', error);
		}
	}

	function confirmSleep() {
		modals.open(toModalComponent(ConfirmDialog), {
			title: 'Confirm Power Down',
			message: 'Are you sure you want to switch off the device?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Switch Off', icon: Power }
			},
			onConfirm: () => {
				modals.close();
				postSleep();
			}
		});
	}
</script>

<div class="navbar bg-base-300 sticky top-0 z-10 h-12 min-h-fit w-full drop-shadow-lg lg:h-16">
	<div class="flex flex-1 min-w-0 items-center justify-left">
		<!-- Page Hamburger Icon here -->
		<label for="main-menu" class="btn btn-ghost btn-circle btn-sm drawer-button lg:hidden"
			><Hamburger class="h-6 w-auto" /></label
		>
		<span class="inline-block min-w-0 truncate px-2 text-xl font-bold lg:text-2xl"
			>{page.data?.title || 'PlantWatch'}</span
		>
	</div>
	<div class="flex-none px-2 text-sm font-mono whitespace-nowrap">
		<span class="max-[360px]:hidden">{currentDate}</span>
		<span class="max-[360px]:hidden">&nbsp;</span>
		<span class="max-[320px]:hidden">{currentClock}</span>
	</div>
	<div class="flex-none">
		{#if $telemetry.rssi.disconnected}
			<WiFiOff class="inline-block h-7 w-7" />
		{:else}
			<RssiIndicator
				showDBm={false}
				rssi_dbm={$telemetry.rssi.rssi}
				ssid={$telemetry.rssi.ssid}
				class="inline-block h-7 w-7"
			/>
		{/if}
	</div>

	{#if page.data.features.battery}
		<div class="flex-none">
			<BatteryIndicator
				charging={$telemetry.battery.charging}
				soc={$telemetry.battery.soc}
				class="inline-block h-7 w-7"
			/>
		</div>
	{/if}

	{#if page.data.features.sleep}
		<div class="flex-none">
			<button class="btn btn-square btn-ghost h-9 w-10" onclick={confirmSleep}>
				<Power class="text-error h-9 w-9" />
			</button>
		</div>
	{/if}
</div>
