<script lang="ts">
	import { modals } from 'svelte-modals';
	import type { ModalComponent } from 'svelte-modals';
	import { page } from '$app/state';
	import { user } from '$lib/stores/user';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import Cancel from '~icons/tabler/x';
	import Power from '~icons/tabler/reload';
	import Sleep from '~icons/tabler/zzz';
	import FactoryReset from '~icons/tabler/refresh-dot';

	const toModalComponent = (component: unknown) => component as unknown as ModalComponent<any>;

	async function postRestart() {
		await fetch('/rest/restart', {
			method: 'POST',
			headers: {
				Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
			}
		});
	}

	function confirmRestart() {
		modals.open(toModalComponent(ConfirmDialog), {
			title: 'Confirm Restart',
			message: 'Are you sure you want to restart the device?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Restart', icon: Power }
			},
			onConfirm: () => {
				modals.close();
				postRestart();
			}
		});
	}

	async function postFactoryReset() {
		await fetch('/rest/factoryReset', {
			method: 'POST',
			headers: {
				Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
			}
		});
	}

	function confirmReset() {
		modals.open(toModalComponent(ConfirmDialog), {
			title: 'Confirm Factory Reset',
			message:
				'This will be erased:\n' +
				'• WiFi configuration (saved STA networks)\n' +
				'• LittleFS data (logs/data logger and configuration files)\n' +
				'• Calibration data\n\n' +
				'Restored to defaults after reboot:\n' +
				'• AP defaults:\n' +
				'- SSID: T-HIGROW-<unique_id>\n' +
				'- Password: plantstatus\n' +
				'- IP: 192.168.4.1\n' +
				'• Hostname/mDNS: T-HIGROW-<unique_id>.local\n' +
				'• Login: admin/admin',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Factory Reset', icon: FactoryReset }
			},
			onConfirm: () => {
				modals.close();
				postFactoryReset();
			}
		});
	}

	async function postSleep() {
		await fetch('/rest/power/sleepCycle', {
			method: 'POST',
			headers: {
				Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
			}
		});
	}

	function confirmSleep() {
		modals.open(toModalComponent(ConfirmDialog), {
			title: 'Confirm Going to Sleep',
			message: 'Are you sure you want to put the device into sleep?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Sleep', icon: Sleep }
			},
			onConfirm: () => {
				modals.close();
				postSleep();
			}
		});
	}
</script>

<div class="card bg-base-200 shadow-primary/50 shadow-lg">
	<div class="card-body p-4">
		<h2 class="card-title text-lg mb-2">Power Settings</h2>

		<div class="text-sm opacity-80 leading-relaxed">
			<p>
				Planned: this section will expose power/runtime behavior settings (in addition to the action
				buttons below).
			</p>
			<ul class="list-disc ml-5 mt-2">
				<li>
					<b>Wake/keep-alive interval</b> (currently fixed): how often the device wakes up and performs
					a short keep-alive cycle (e.g. every 5 minutes).
				</li>
				<li>
					<b>Wake window (active time)</b>: how long the device stays awake after each wake-up
					before returning to sleep (e.g. 5–20 seconds).
				</li>
				<li>
					<b>Sleep interval</b>: the technical wake/sleep cadence used for low-power or power-bank
					mode. This is separate from the keep-alive interval and from scheduled automation actions.
				</li>
				<li>
					<b>Always-on mode</b>: prevent entering sleep to keep the device continuously online.
				</li>
				<li>
					<b>Power bank mode</b>: keep the ESP32 alive when powered from “smart” power banks that
					cut power at very low load. The device will periodically wake up for a short time window
					(doing minimal work) and then return to sleep, while still executing scheduled actions
					(e.g. every 5 minutes).
				</li>
				<li>
					<b>Safety limits</b>: minimum awake time after boot / after applying settings, to avoid
					locking yourself out with too aggressive sleep.
				</li>
				<li>
					<b>External power policy</b>: optionally switch to Always-on when USB/external power is
					present (and return to low-power mode on battery).
				</li>
				<li>
					<b>Low battery behavior</b> (when battery is present): adjust sleep and keep-alive intervals,
					disable heavy workloads, or limit actuators to protect the battery.
				</li>
				<li>
					<b>Keep-alive method</b> (power bank mode): select how the device prevents a power bank from
					shutting down (implementation detail; UI may keep this as a single toggle).
				</li>
			</ul>
		</div>

		<div class="flex flex-wrap justify-end gap-2">
			{#if page.data.features.sleep}
				<button class="btn btn-primary btn-sm" onclick={confirmSleep}>
					<Sleep class="w-4 h-4" /> Sleep
				</button>
			{/if}
			{#if !page.data.features.security || $user.admin}
				<button class="btn btn-primary btn-sm" onclick={confirmRestart}>
					<Power class="w-4 h-4" /> Restart
				</button>
				<button class="btn btn-secondary btn-sm" onclick={confirmReset}>
					<FactoryReset class="w-4 h-4" /> Factory Reset
				</button>
			{/if}
		</div>
	</div>
</div>
