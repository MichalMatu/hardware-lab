<script lang="ts">
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import Spinner from '$lib/components/Spinner.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import { TIME_ZONES } from './timezones';
	import { formatLongDateTime, formatUTCDateTime } from '$lib/utils/timeFormat';
	import NTP from '~icons/tabler/clock-check';
	import Server from '~icons/tabler/server';
	import Clock from '~icons/tabler/clock';
	import UTC from '~icons/tabler/clock-pin';
	import Stopwatch from '~icons/tabler/24-hours';
	import Save from '~icons/tabler/device-floppy';
	import type { NTPSettings, NTPStatus, RTCStatus } from '$lib/types/models';

	let ntpSettings: NTPSettings = $state()!;
	let ntpStatus: NTPStatus = $state()!;
	let rtcStatus: RTCStatus = $state()!;
	let manualTimeInput: string = $state('');

	// Auto-fill manual time with browser time when switching to manual mode
	$effect(() => {
		if (!ntpSettings?.enabled && !manualTimeInput) {
			// Use current browser time
			const date = new Date();
			const year = date.getFullYear();
			const month = String(date.getMonth() + 1).padStart(2, '0');
			const day = String(date.getDate()).padStart(2, '0');
			const hours = String(date.getHours()).padStart(2, '0');
			const minutes = String(date.getMinutes()).padStart(2, '0');
			const seconds = String(date.getSeconds()).padStart(2, '0');
			manualTimeInput = `${year}-${month}-${day}T${hours}:${minutes}:${seconds}`;
		}
	});

	async function getNTPStatus() {
		try {
			const response = await fetch('/rest/ntpStatus', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			ntpStatus = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	async function getNTPSettings() {
		try {
			const response = await fetch('/rest/ntpSettings', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			ntpSettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	async function getRTCStatus() {
		try {
			const response = await fetch('/rest/rtc/status', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			if (response.ok) {
				rtcStatus = await response.json();
			}
		} catch (error) {
			console.error('RTC Status Error:', error);
		}
		return;
	}

	async function refreshStatus() {
		await Promise.all([getNTPStatus(), getRTCStatus()]);
	}

	async function syncRTC() {
		try {
			const response = await fetch('/rest/rtc/sync', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			if (response.ok) {
				notifications.success('RTC synchronized with system time.', 3000);
				await getRTCStatus();
			} else {
				const errorData = await response.json();
				notifications.error(errorData.message || 'Failed to sync RTC.', 3000);
			}
		} catch (error) {
			console.error('RTC Sync Error:', error);
			notifications.error('Error syncing RTC.', 3000);
		}
	}

	async function getNTPData() {
		await getNTPStatus();
		await getRTCStatus();
		if (!page.data.features.security || $user.admin) {
			await getNTPSettings();
		}
	}

	let formField: any = $state();

	const detailIconClass = 'h-6 w-6 flex-none text-base-content/70';
	const statusIconClass = (active: boolean) =>
		`h-6 w-6 flex-none ${active ? 'text-success' : 'text-error'}`;

	let formErrors = $state({
		server: false
	});

	async function postNTPSettings(data: NTPSettings) {
		try {
			const response = await fetch('/rest/ntpSettings', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});

			if (response.status == 200) {
				notifications.success('Time settings updated successfully.', 3000);
				ntpSettings = await response.json();
			} else {
				notifications.error('Failed to update settings. Check authorization.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	function useBrowserTime() {
		// Get current browser time in format YYYY-MM-DDTHH:MM:SS
		const now = new Date();
		// Format to local datetime without timezone suffix
		const year = now.getFullYear();
		const month = String(now.getMonth() + 1).padStart(2, '0');
		const day = String(now.getDate()).padStart(2, '0');
		const hours = String(now.getHours()).padStart(2, '0');
		const minutes = String(now.getMinutes()).padStart(2, '0');
		const seconds = String(now.getSeconds()).padStart(2, '0');
		manualTimeInput = `${year}-${month}-${day}T${hours}:${minutes}:${seconds}`;
	}

	async function setManualTime() {
		if (!manualTimeInput) {
			notifications.error('Please enter a valid time.', 3000);
			return;
		}

		if (ntpSettings.enabled) {
			notifications.error('Please disable NTP before setting manual time.', 3000);
			return;
		}

		try {
			// First, ensure NTP is disabled in backend (save settings if needed)
			if (ntpSettings.enabled === false) {
				notifications.info('Disabling NTP...', 2000);
				await postNTPSettings(ntpSettings);
				// Wait for backend to apply NTP disable
				await new Promise((resolve) => setTimeout(resolve, 500));
			}

			// Log what we're sending
			console.log('Setting manual time:', {
				local_time: manualTimeInput,
				ntp_enabled: ntpSettings.enabled
			});

			// Now set the manual time via TIME_PATH endpoint
			const timeResponse = await fetch('/rest/time', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ local_time: manualTimeInput })
			});

			if (timeResponse.status !== 200) {
				let errorMsg = 'Failed to set time';
				try {
					const errorJson = await timeResponse.json();
					errorMsg = errorJson.message || errorMsg;
				} catch {
					const errorText = await timeResponse.text();
					console.error('Time set failed:', timeResponse.status, errorText);
				}
				notifications.error(errorMsg, 5000);
				return;
			}

			notifications.success('Time set successfully. Syncing to RTC...', 3000);

			// Wait for system time to be applied
			await new Promise((resolve) => setTimeout(resolve, 300));

			// Verify system time was actually set and refresh RTC status immediately
			await Promise.all([getNTPStatus(), getRTCStatus()]);

			// Check if year is valid (2025-2040)
			const systemYear = new Date(ntpStatus.utc_time).getFullYear();
			if (systemYear < 2025 || systemYear > 2040) {
				notifications.error(
					`Invalid system time year: ${systemYear}. Backend validation failed.`,
					5000
				);
				return;
			}

			// Now try RTC sync with retry
			let rtcSynced = false;
			for (let attempt = 0; attempt < 3 && !rtcSynced; attempt++) {
				if (attempt > 0) {
					await new Promise((resolve) => setTimeout(resolve, 500));
				}

				const rtcResponse = await fetch('/rest/rtc/sync', {
					method: 'POST',
					headers: {
						Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
						'Content-Type': 'application/json'
					}
				});

				if (rtcResponse.status === 200) {
					rtcSynced = true;
					notifications.success('Time synchronized to RTC successfully.', 3000);
					// Refresh status to show new time (NTP + RTC)
					await Promise.all([getNTPStatus(), getRTCStatus()]);
				} else if (attempt === 2) {
					const errorText = await rtcResponse.text();
					console.error('RTC sync failed after 3 attempts:', errorText);
					notifications.warning(
						'Time set but RTC sync failed. RTC may not have battery backup.',
						5000
					);
				}
			}
		} catch (error) {
			console.error('Error setting manual time:', error);
			notifications.error('Error setting manual time.', 3000);
		}
	}

	async function handleSubmitNTP() {
		let valid = true;

		// Validate Server
		// RegEx for IPv4
		const regexExpIPv4 =
			/\b(?:(?:2(?:[0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9])\.){3}(?:(?:2([0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9]))\b/;
		const regexExpURL =
			/[-a-zA-Z0-9@:%_.~#?&//=]{2,256}\.[a-z]{2,4}\b(\/[-a-zA-Z0-9@:%_.~#?&//=]*)?/i;

		if (!regexExpURL.test(ntpSettings.server) && !regexExpIPv4.test(ntpSettings.server)) {
			valid = false;
			formErrors.server = true;
		} else {
			formErrors.server = false;
		}

		ntpSettings.tz_format = TIME_ZONES[ntpSettings.tz_label];

		// Submit JSON to REST API
		if (valid) {
			await postNTPSettings(ntpSettings);

			// If NTP is disabled and manual time is set, apply it
			if (!ntpSettings.enabled && manualTimeInput) {
				await setManualTime();
			}
		}
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

	function preventDefault(fn: () => void) {
		return function (event: Event) {
			event.preventDefault();
			fn();
		};
	}
</script>

{#await getNTPData()}
	<div class="flex justify-center items-center p-8">
		<Spinner />
	</div>
{:then _result}
	<div class="grid grid-cols-1 md:grid-cols-2 gap-4">
		<!-- Left Column: Time Status -->
		<div class="card bg-base-200 shadow-primary/50 shadow-lg">
			<div class="card-body p-4">
				<h2 class="card-title text-lg mb-2">
					<Clock class="w-6 h-6" /> Time Status
				</h2>
				<div
					class="flex w-full flex-col space-y-1"
					transition:slide|local={{ duration: 300, easing: cubicOut }}
				>
					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<NTP class={statusIconClass(ntpStatus.status === 1)} />
						</div>
						<div>
							<div class="font-bold">Status</div>
							<div class="text-sm opacity-75">
								{ntpStatus.status === 1 ? 'Active' : 'Inactive'}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<Server class={detailIconClass} />
						</div>
						<div>
							<div class="font-bold">NTP Server</div>
							<div class="text-sm opacity-75">
								{ntpStatus.server}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<Clock class={detailIconClass} />
						</div>
						<div>
							<div class="font-bold">Local Time</div>
							<div class="text-sm opacity-75">
								{formatLongDateTime(ntpStatus.local_time, ntpSettings?.tz_label)}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<UTC class={detailIconClass} />
						</div>
						<div>
							<div class="font-bold">UTC Time</div>
							<div class="text-sm opacity-75">
								{formatUTCDateTime(ntpStatus.utc_time)}
							</div>
						</div>
					</div>
				</div>
			</div>
		</div>

		<!-- RTC Status Card -->
		{#if rtcStatus}
			<div class="card bg-base-200 shadow-primary/50 shadow-lg">
				<div class="card-body p-4">
					<h2 class="card-title text-lg mb-2">
						<Clock class="w-6 h-6" /> RTC Status (DS3231)
					</h2>
					<div
						class="flex w-full flex-col space-y-1"
						transition:slide|local={{ duration: 300, easing: cubicOut }}
					>
						<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
							<div class="flex h-10 w-10 items-center justify-center">
								<NTP class={statusIconClass(rtcStatus.initialized && !rtcStatus.lost_power)} />
							</div>
							<div class="flex-1">
								<div class="font-bold">Battery Status</div>
								<div class="text-sm opacity-75">
									{#if rtcStatus.lost_power}
										<span class="text-warning">⚠️ Lost power - battery may be dead</span>
									{:else}
										<span class="text-success">✓ Battery backup OK</span>
									{/if}
								</div>
							</div>
						</div>

						<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
							<div class="flex h-10 w-10 items-center justify-center">
								<Clock class={detailIconClass} />
							</div>
							<div class="flex-1">
								<div class="font-bold">RTC Time (Local)</div>
								<div class="text-sm opacity-75">
									{formatLongDateTime(rtcStatus.rtc_time, ntpSettings?.tz_label)}
								</div>
							</div>
						</div>

						<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
							<div class="flex h-10 w-10 items-center justify-center">
								<UTC class={detailIconClass} />
							</div>
							<div class="flex-1">
								<div class="font-bold">Last RTC Sync</div>
								<div class="text-sm opacity-75">
									{#if rtcStatus.last_sync_time === 'never'}
										<span class="text-warning">Never synced</span>
									{:else}
										{formatLongDateTime(rtcStatus.last_sync_time, ntpSettings?.tz_label)}
									{/if}
								</div>
							</div>
						</div>

						<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
							<div class="flex h-10 w-10 items-center justify-center">
								<Stopwatch class={detailIconClass} />
							</div>
							<div class="flex-1">
								<div class="font-bold">Uptime</div>
								<div class="text-sm opacity-75">
									{convertSeconds(ntpStatus.uptime)}
								</div>
							</div>
						</div>
					</div>
				</div>
			</div>
		{/if}
	</div>

	<div class="grid grid-cols-1 gap-4 mt-4">
		{#if !page.data.features.security || $user.admin}
			<!-- Right Column: Time Settings -->
			<div class="card bg-base-200 shadow-primary/50 shadow-lg">
				<div class="card-body p-4">
					<h2 class="card-title text-lg mb-2">
						<Clock class="w-6 h-6" /> Time Settings
					</h2>
					<form
						class="space-y-3"
						onsubmit={preventDefault(handleSubmitNTP)}
						novalidate
						bind:this={formField}
					>
						<fieldset class="space-y-2">
							<legend class="label p-0">Time Source</legend>
							<div class="flex gap-6">
								<label class="label cursor-pointer justify-start gap-3" for="time-source-ntp">
									<input
										id="time-source-ntp"
										type="radio"
										name="time-source"
										class="radio radio-primary"
										checked={ntpSettings.enabled}
										onchange={() => (ntpSettings.enabled = true)}
									/>
									<span>Network Time (NTP)</span>
								</label>
								<label class="label cursor-pointer justify-start gap-3" for="time-source-manual">
									<input
										id="time-source-manual"
										type="radio"
										name="time-source"
										class="radio radio-primary"
										checked={!ntpSettings.enabled}
										onchange={() => (ntpSettings.enabled = false)}
									/>
									<span>Manual Time</span>
								</label>
							</div>
						</fieldset>

						<div class="divider my-4">
							{ntpSettings.enabled ? 'Network Time (NTP)' : 'Manual Time'}
						</div>

						{#if ntpSettings.enabled}
							<div>
								<label class="label" for="server">Server</label>
								<input
									type="text"
									min="3"
									max="64"
									class="input w-full invalid:border-error invalid:border-2 {formErrors.server
										? 'border-error border-2'
										: ''}"
									bind:value={ntpSettings.server}
									id="server"
									required
								/>
								{#if formErrors.server}
									<label class="label" for="server">
										<span class="text-error text-sm">Please enter a valid NTP server.</span>
									</label>
								{/if}
							</div>

							<div>
								<label class="label" for="tz">Time Zone</label>
								<select class="select w-full" bind:value={ntpSettings.tz_label} id="tz">
									{#each Object.entries(TIME_ZONES) as [tz_label, _tzFormat]}
										<option value={tz_label}>{tz_label}</option>
									{/each}
								</select>
							</div>
						{:else}
							<div>
								<label class="label" for="manual-time">Set Time</label>
								<div class="flex gap-2 items-center">
									<input
										type="datetime-local"
										bind:value={manualTimeInput}
										class="input w-full"
										id="manual-time"
										step="1"
									/>
									<button
										type="button"
										class="btn btn-square btn-sm"
										onclick={useBrowserTime}
										title="Use current browser time"
									>
										<Clock class="w-4 h-4" />
									</button>
								</div>
								<div class="text-xs opacity-75 mt-1">
									Time will be applied when you click "Apply Settings" below.
								</div>
							</div>
						{/if}
					</form>

					<div class="mt-4 flex justify-end gap-2">
						<button class="btn btn-secondary btn-sm" onclick={preventDefault(refreshStatus)}>
							Refresh Status
						</button>
						<button class="btn btn-accent btn-sm" onclick={preventDefault(syncRTC)}>
							Sync RTC Now
						</button>
						<button class="btn btn-primary btn-sm" onclick={handleSubmitNTP}>
							<Save class="w-4 h-4" /> Apply Settings
						</button>
					</div>
				</div>
			</div>
		{/if}
	</div>
{/await}
