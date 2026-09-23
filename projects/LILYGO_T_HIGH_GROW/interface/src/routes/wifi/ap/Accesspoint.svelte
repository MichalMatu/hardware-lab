<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import Spinner from '$lib/components/Spinner.svelte';
	import AP from '~icons/tabler/access-point';
	import MAC from '~icons/tabler/dna-2';
	import Home from '~icons/tabler/home';
	import Devices from '~icons/tabler/devices';
	import Save from '~icons/tabler/device-floppy';
	import type { ApSettings, ApStatus } from '$lib/types/models';

	let apSettings: ApSettings = $state()!;
	let apStatus: ApStatus = $state()!;

	let formField: any = $state();

	const detailIconClass = 'h-6 w-6 flex-none text-base-content/70';

	async function getAPStatus() {
		try {
			const response = await fetch('/rest/apStatus', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			apStatus = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return apStatus;
	}

	async function getAPSettings() {
		try {
			const response = await fetch('/rest/apSettings', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			apSettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return apSettings;
	}

	const interval = setInterval(async () => {
		getAPStatus();
	}, 5000);

	onDestroy(() => clearInterval(interval));

	onMount(() => {
		if (!page.data.features.security || $user.admin) {
			getAPSettings();
		}
	});

	let apStatusDescription = [
		{ icon_color: 'text-success', description: 'Active' },
		{ icon_color: 'text-error', description: 'Inactive' }
	];

	let formErrors = $state({
		ssid: false,
		password: false,
		channel: false,
		max_clients: false,
		local_ip: false,
		gateway_ip: false,
		subnet_mask: false
	});

	async function postAPSettings(data: ApSettings) {
		try {
			const response = await fetch('/rest/apSettings', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});
			if (response.status == 200) {
				notifications.success('Access Point settings updated.', 3000);
				apSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	function handleSubmitAP() {
		let valid = true;

		// Validate SSID
		if (apSettings.ssid.length < 3 || apSettings.ssid.length > 32) {
			valid = false;
			formErrors.ssid = true;
		} else {
			formErrors.ssid = false;
		}

		// Validate Password (empty = open AP, otherwise WPA2-PSK 8-63 chars)
		if (
			apSettings.password.length > 0 &&
			(apSettings.password.length < 8 || apSettings.password.length > 63)
		) {
			valid = false;
			formErrors.password = true;
		} else {
			formErrors.password = false;
		}

		// Validate Channel
		let channel = Number(apSettings.channel);
		if (1 > channel || channel > 13) {
			valid = false;
			formErrors.channel = true;
		} else {
			formErrors.channel = false;
		}

		// Validate max_clients
		let maxClients = Number(apSettings.max_clients);
		if (1 > maxClients || maxClients > 8) {
			valid = false;
			formErrors.max_clients = true;
		} else {
			formErrors.max_clients = false;
		}

		// RegEx for IPv4
		const regexExp =
			/\b(?:(?:2(?:[0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9])\.){3}(?:(?:2([0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9]))\b/;

		// Validate gateway IP
		if (!regexExp.test(apSettings.gateway_ip)) {
			valid = false;
			formErrors.gateway_ip = true;
		} else {
			formErrors.gateway_ip = false;
		}

		// Validate Subnet Mask
		if (!regexExp.test(apSettings.subnet_mask)) {
			valid = false;
			formErrors.subnet_mask = true;
		} else {
			formErrors.subnet_mask = false;
		}

		// Validate local IP
		if (!regexExp.test(apSettings.local_ip)) {
			valid = false;
			formErrors.local_ip = true;
		} else {
			formErrors.local_ip = false;
		}

		// Submit JSON to REST API
		if (valid) {
			postAPSettings(apSettings);
		}
	}

	function preventDefault(fn: () => void) {
		return function (event: Event) {
			event.preventDefault();
			fn();
		};
	}
</script>

{#await getAPStatus()}
	<div class="flex justify-center items-center p-8">
		<Spinner />
	</div>
{:then _status}
	<div class="grid grid-cols-1 md:grid-cols-2 gap-4">
		<!-- Left Column: AP Status -->
		<div class="card bg-base-200 shadow-primary/50 shadow-lg">
			<div class="card-body p-4">
				<h2 class="card-title text-lg mb-2">
					<AP class="w-6 h-6" /> Access Point Status
				</h2>
				<div
					class="flex w-full flex-col space-y-1"
					transition:slide|local={{ duration: 300, easing: cubicOut }}
				>
					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<AP class={`h-6 w-6 flex-none ${apStatusDescription[apStatus.status].icon_color}`} />
						</div>
						<div>
							<div class="font-bold">Status</div>
							<div class="text-sm opacity-75">
								{apStatusDescription[apStatus.status].description}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<Home class={detailIconClass} />
						</div>
						<div>
							<div class="font-bold">IP Address</div>
							<div class="text-sm opacity-75">
								{apStatus.ip_address}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<MAC class={detailIconClass} />
						</div>
						<div>
							<div class="font-bold">MAC Address</div>
							<div class="text-sm opacity-75">
								{apStatus.mac_address}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="flex h-10 w-10 items-center justify-center">
							<Devices class={detailIconClass} />
						</div>
						<div>
							<div class="font-bold">AP Clients</div>
							<div class="text-sm opacity-75">
								{apStatus.station_num}
							</div>
						</div>
					</div>
				</div>
			</div>
		</div>

		{#if !page.data.features.security || $user.admin}
			<!-- Right Column: AP Settings -->
			<div class="card bg-base-200 shadow-primary/50 shadow-lg">
				<div class="card-body p-4">
					<h2 class="card-title text-lg mb-2">
						<AP class="w-6 h-6" /> Access Point Settings
					</h2>
					{#await getAPSettings()}
						<div class="flex justify-center items-center p-4">
							<Spinner />
						</div>
					{:then _settings}
						<form
							class="fieldset grid w-full grid-cols-1 content-center gap-x-4 gap-y-2 sm:grid-cols-2"
							onsubmit={preventDefault(handleSubmitAP)}
							novalidate
							bind:this={formField}
						>
							<div>
								<label class="label" for="ssid">SSID</label>
								<input
									type="text"
									class="input w-full invalid:border-error invalid:border-2 {formErrors.ssid
										? 'border-error border-2'
										: ''}"
									bind:value={apSettings.ssid}
									id="ssid"
									autocomplete="off"
									min="2"
									max="32"
									required
								/>
								<label class="label" for="ssid">
									<span class="text-error {formErrors.ssid ? '' : 'hidden'}"
										>SSID must be between 2 and 32 characters long</span
									>
								</label>
							</div>

							<div>
								<label class="label" for="pwd">Password</label>
								<InputPassword
									bind:value={apSettings.password}
									id="pwd"
									class={formErrors.password ? 'border-error border-2' : ''}
									maxlength={63}
								/>
								<label class="label" for="pwd">
									<span class="text-error {formErrors.password ? '' : 'hidden'}">
										Password must be empty (open AP) or 8-63 characters long
									</span>
								</label>
							</div>
							<div>
								<label class="label" for="channel">Preferred Channel</label>
								<input
									type="number"
									min="1"
									max="13"
									class="input w-full invalid:border-error invalid:border-2 {formErrors.channel
										? 'border-error border-2'
										: ''}"
									bind:value={apSettings.channel}
									id="channel"
									required
								/>
								<label class="label" for="channel">
									<span class="text-error {formErrors.channel ? '' : 'hidden'}"
										>Must be channel 1 to 13</span
									>
								</label>
							</div>

							<div>
								<label class="label" for="clients">Max Clients</label>
								<input
									type="number"
									min="1"
									max="8"
									class="input w-full invalid:border-error invalid:border-2 {formErrors.max_clients
										? 'border-error border-2'
										: ''}"
									bind:value={apSettings.max_clients}
									id="clients"
									required
								/>
								<label class="label" for="clients">
									<span class="text-error {formErrors.max_clients ? '' : 'hidden'}"
										>Maximum 8 clients allowed</span
									>
								</label>
							</div>

							<div>
								<label class="label" for="localIP">Local IP</label>
								<input
									type="text"
									class="input w-full {formErrors.local_ip ? 'border-error border-2' : ''}"
									minlength="7"
									maxlength="15"
									size="15"
									bind:value={apSettings.local_ip}
									id="localIP"
									required
								/>
								<label class="label" for="localIP">
									<span class="text-error {formErrors.local_ip ? '' : 'hidden'}"
										>Must be a valid IPv4 address</span
									>
								</label>
							</div>

							<div>
								<label class="label" for="gateway">Gateway IP</label>
								<input
									type="text"
									class="input w-full {formErrors.gateway_ip ? 'border-error border-2' : ''}"
									minlength="7"
									maxlength="15"
									size="15"
									bind:value={apSettings.gateway_ip}
									id="gateway"
									required
								/>
								<label class="label" for="gateway">
									<span class="text-error {formErrors.gateway_ip ? '' : 'hidden'}"
										>Must be a valid IPv4 address</span
									>
								</label>
							</div>
							<div>
								<label class="label" for="subnet">Subnet Mask</label>
								<input
									type="text"
									class="input w-full {formErrors.subnet_mask ? 'border-error border-2' : ''}"
									minlength="7"
									maxlength="15"
									size="15"
									bind:value={apSettings.subnet_mask}
									id="subnet"
									required
								/>
								<label class="label" for="subnet">
									<span class="text-error {formErrors.subnet_mask ? '' : 'hidden'}"
										>Must be a valid IPv4 address</span
									>
								</label>
							</div>

							<label class="label my-auto cursor-pointer justify-start gap-4 col-span-2">
								<input
									type="checkbox"
									bind:checked={apSettings.ssid_hidden}
									class="checkbox checkbox-primary"
								/>
								<span class="">Hide SSID</span>
							</label>
						</form>

						<div class="mt-4 flex justify-end">
							<button class="btn btn-primary btn-sm" onclick={handleSubmitAP}>
								<Save class="w-4 h-4" /> Apply Settings
							</button>
						</div>
					{/await}
				</div>
			</div>
		{/if}
	</div>
{/await}
