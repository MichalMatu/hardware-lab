<script lang="ts">
	import { onMount } from 'svelte';
	import { page } from '$app/stores';
	import { user } from '$lib/stores/user';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import DeviceFormModal from '$lib/components/rf433/DeviceFormModal.svelte';
	import Radio from '~icons/tabler/antenna-bars-5';
	import Plus from '~icons/tabler/plus';
	import Trash from '~icons/tabler/trash';
	import Edit from '~icons/tabler/edit';
	import Power from '~icons/tabler/power';
	import CircleOff from '~icons/tabler/circle-off';
	import Refresh from '~icons/tabler/refresh';
	import MoreVertical from '~icons/tabler/dots-vertical';
	import type {
		Rf433Device,
		Rf433DevicesResponse,
		Rf433StateResponse,
		Rf433SendCommandResponse
	} from '$lib/types/rf433';

	let devices: Rf433Device[] = $state([]);
	let deviceStates = $state<Map<string, boolean>>(new Map());
	let loading = $state(true);
	let sending = $state<string | null>(null);
	let error = $state<string | null>(null);
	let controllerReady = $state(false);

	// Modal state
	let isModalOpen = $state(false);
	let editingDevice = $state<Rf433Device | null>(null);

	async function loadDevices() {
		loading = true;
		error = null;
		try {
			const response = await fetch('/api/rf433/devices', {
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
				}
			});
			if (!response.ok) throw new Error('Failed to load devices');
			const data: Rf433DevicesResponse = await response.json();
			devices = data.devices;
		} catch (e) {
			error = e instanceof Error ? e.message : 'Failed to load devices';
			console.error('Failed to load devices:', e);
		} finally {
			loading = false;
		}
	}

	async function loadState() {
		try {
			const response = await fetch('/api/rf433/state', {
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
				}
			});
			if (!response.ok) return;
			const data: Rf433StateResponse = await response.json();
			controllerReady = data.controller_ready;

			const newStates = new Map<string, boolean>();
			data.states.forEach((s) => {
				newStates.set(s.device_id, s.last_command_on);
			});
			deviceStates = newStates;
		} catch (e) {
			console.error('Failed to load state:', e);
		}
	}

	async function sendCommand(deviceId: string, command: 'on' | 'off') {
		sending = deviceId + '_' + command;
		error = null;
		try {
			const response = await fetch('/api/rf433/send', {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json',
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
				},
				body: JSON.stringify({ device_id: deviceId, command, sync: false })
			});

			if (!response.ok) {
				const data: Rf433SendCommandResponse = await response.json();
				throw new Error(data.error || 'Failed to send command');
			}

			// Update local state optimistically
			deviceStates.set(deviceId, command === 'on');
			deviceStates = new Map(deviceStates);

			// Reload state after a short delay
			setTimeout(loadState, 500);
		} catch (e) {
			error = e instanceof Error ? e.message : 'Failed to send command';
			console.error('Failed to send command:', e);
		} finally {
			sending = null;
		}
	}

	async function deleteDevice(deviceId: string) {
		if (!confirm(`Delete device "${deviceId}"?`)) return;

		try {
			const response = await fetch(`/api/rf433/devices/${deviceId}`, {
				method: 'DELETE',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
				}
			});

			if (!response.ok) throw new Error('Failed to delete device');

			await loadDevices();
		} catch (e) {
			error = e instanceof Error ? e.message : 'Failed to delete device';
			console.error('Failed to delete device:', e);
		}
	}

	function openAddModal() {
		editingDevice = null;
		isModalOpen = true;
	}

	function openEditModal(device: Rf433Device) {
		editingDevice = device;
		isModalOpen = true;
	}

	function closeModal() {
		isModalOpen = false;
		editingDevice = null;
	}

	async function handleSaveDevice(device: Rf433Device) {
		error = null;

		try {
			// Determine if this is add or update based on editingDevice
			const isUpdate = editingDevice !== null;
			const method = isUpdate ? 'PUT' : 'POST';
			const url = isUpdate ? `/api/rf433/devices/${device.id}` : '/api/rf433/devices';

			// Device already has numeric codes from modal conversion
			const response = await fetch(url, {
				method,
				headers: {
					'Content-Type': 'application/json',
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
				},
				body: JSON.stringify(device)
			});

			if (!response.ok) {
				const data = await response.json();
				throw new Error(data.error || 'Failed to save device');
			}

			await loadDevices();
			closeModal();
		} catch (e) {
			error = e instanceof Error ? e.message : 'Failed to save device';
			throw e; // Re-throw so modal can show error
		}
	}

	onMount(() => {
		loadDevices();
		loadState();
	});

	function formatCode(code: number): string {
		return '0x' + code.toString(16).toUpperCase().padStart(6, '0');
	}

	function getDeviceState(deviceId: string): boolean | null {
		return deviceStates.has(deviceId) ? deviceStates.get(deviceId)! : null;
	}
</script>

<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-4 max-[320px]:px-2 pb-12">
	<SettingsCard collapsible={false} maxwidth="max-w-7xl" marginTop="mt-0">
		{#snippet icon()}<Radio class="mr-3 h-8 w-8" />{/snippet}
		{#snippet title()}433MHz RF Devices{/snippet}
		{#snippet children()}
			<div class="flex flex-col gap-4">
				<!-- Status bar -->
				<div class="flex flex-col gap-3 sm:flex-row sm:items-center sm:justify-between">
					<div class="text-sm opacity-70 flex-1 min-w-0">
						{#if !controllerReady}
							<span class="text-error">⚠ Controller not ready</span>
						{:else}
							<span class="text-success">✓ Controller ready</span>
						{/if}
						· {devices.length} device{devices.length !== 1 ? 's' : ''}
					</div>
					<div class="flex flex-wrap gap-2 sm:shrink-0 sm:justify-end">
						<button class="btn btn-sm" onclick={loadDevices} disabled={loading}>
							<Refresh class="h-4 w-4" />
							<span class="hidden sm:inline">Refresh</span>
						</button>
						<button class="btn btn-sm btn-primary" onclick={openAddModal}>
							<Plus class="h-4 w-4" />
							<span class="hidden sm:inline">Add Device</span>
						</button>
					</div>
				</div>

				<!-- Error message -->
				{#if error}
					<div class="alert alert-error">
						<span>{error}</span>
					</div>
				{/if}

				<!-- Loading state -->
				{#if loading}
					<div class="flex justify-center p-8">
						<span class="loading loading-spinner loading-lg"></span>
					</div>
				{/if}

				<!-- Devices list -->
				{#if !loading && devices.length === 0}
					<div class="text-center p-8 opacity-60">
						<p>No devices configured yet.</p>
						<p class="text-sm mt-2">Click "Add Device" to create your first RF433 device.</p>
					</div>
				{/if}

				{#if !loading && devices.length > 0}
					<div class="w-full max-w-full overflow-x-auto">
						<table class="table table-zebra">
							<thead>
								<tr>
									<th>Device Name</th>
									<th class="hidden md:table-cell">ON Code</th>
									<th class="hidden md:table-cell">OFF Code</th>
									<th class="hidden md:table-cell">Protocol</th>
									<th class="text-center">Status</th>
									<th class="text-right">Actions</th>
								</tr>
							</thead>
							<tbody>
								{#each devices as device}
									{@const state = getDeviceState(device.id)}
									{@const isSendingOn = sending === device.id + '_on'}
									{@const isSendingOff = sending === device.id + '_off'}
									<tr>
										<td class="font-medium break-words">{device.label}</td>
										<td class="hidden md:table-cell font-mono text-xs whitespace-nowrap">
											{formatCode(device.code_on)}
										</td>
										<td class="hidden md:table-cell font-mono text-xs whitespace-nowrap">
											{formatCode(device.code_off)}
										</td>
										<td class="hidden md:table-cell text-center">
											<span class="badge badge-sm">P{device.protocol}</span>
										</td>
										<td class="text-center">
											{#if state === true}
												<span class="badge badge-success badge-sm">ON</span>
											{:else if state === false}
												<span class="badge badge-sm">OFF</span>
											{:else}
												<span class="opacity-40">—</span>
											{/if}
										</td>
										<td class="text-right">
											<!-- Desktop: All buttons visible -->
											<div class="hidden sm:flex flex-wrap justify-end gap-1">
												<button
													class="btn btn-xs btn-success"
													onclick={() => sendCommand(device.id, 'on')}
													disabled={isSendingOn || !controllerReady}
													aria-label="Turn ON"
												>
													{#if isSendingOn}
														<span class="loading loading-spinner loading-xs"></span>
													{:else}
														<Power class="h-3 w-3" />
													{/if}
													ON
												</button>
												<button
													class="btn btn-xs"
													onclick={() => sendCommand(device.id, 'off')}
													disabled={isSendingOff || !controllerReady}
													aria-label="Turn OFF"
												>
													{#if isSendingOff}
														<span class="loading loading-spinner loading-xs"></span>
													{:else}
														<CircleOff class="h-3 w-3" />
													{/if}
													OFF
												</button>
												<button class="btn btn-xs btn-ghost" onclick={() => openEditModal(device)}>
													<Edit class="h-4 w-4" />
												</button>
												<button
													class="btn btn-xs btn-ghost text-error"
													onclick={() => deleteDevice(device.id)}
												>
													<Trash class="h-4 w-4" />
												</button>
											</div>
											<!-- Mobile: Dropdown menu -->
											<div class="dropdown dropdown-end sm:hidden">
												<button tabindex="0" class="btn btn-xs btn-ghost" aria-label="Actions">
													<MoreVertical class="h-4 w-4" />
												</button>
												<ul
													tabindex="-1"
													class="dropdown-content menu bg-base-100 rounded-box z-[1] w-44 p-2 shadow-lg border border-base-300"
												>
													<li>
														<button
															onclick={() => sendCommand(device.id, 'on')}
															disabled={isSendingOn || !controllerReady}
															class="flex items-center gap-2"
														>
															{#if isSendingOn}
																<span class="loading loading-spinner loading-xs"></span>
															{:else}
																<Power class="h-4 w-4 text-success" />
															{/if}
															Turn ON
														</button>
													</li>
													<li>
														<button
															onclick={() => sendCommand(device.id, 'off')}
															disabled={isSendingOff || !controllerReady}
															class="flex items-center gap-2"
														>
															{#if isSendingOff}
																<span class="loading loading-spinner loading-xs"></span>
															{:else}
																<CircleOff class="h-4 w-4" />
															{/if}
															Turn OFF
														</button>
													</li>
													<li>
														<button
															onclick={() => openEditModal(device)}
															class="flex items-center gap-2"
														>
															<Edit class="h-4 w-4" />
															Edit
														</button>
													</li>
													<li>
														<button
															onclick={() => deleteDevice(device.id)}
															class="flex items-center gap-2 text-error"
														>
															<Trash class="h-4 w-4" />
															Delete
														</button>
													</li>
												</ul>
											</div>
										</td>
									</tr>
								{/each}
							</tbody>
						</table>
					</div>
				{/if}

				<!-- Info note -->
				<div class="text-xs opacity-60 mt-4">
					<p>
						<b>Note:</b> 433MHz devices use simple ON/OFF codes. Use the test buttons above to verify
						your devices respond correctly before using them in automation.
					</p>
				</div>
			</div>
		{/snippet}
	</SettingsCard>
</div>

<!-- Add/Edit Modal -->
<DeviceFormModal
	bind:isOpen={isModalOpen}
	device={editingDevice}
	onClose={closeModal}
	onSave={handleSaveDevice}
/>
