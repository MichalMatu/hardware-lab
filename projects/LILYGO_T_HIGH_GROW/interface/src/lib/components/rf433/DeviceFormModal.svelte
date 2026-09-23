<script lang="ts">
	import { focusTrap } from 'svelte-focus-trap';
	import { fly } from 'svelte/transition';
	import type { Rf433Device } from '$lib/types/rf433';
	import X from '~icons/tabler/x';
	import DeviceFloppy from '~icons/tabler/device-floppy';

	interface Props {
		isOpen: boolean;
		device?: Rf433Device | null;
		onClose: () => void;
		onSave: (_device: Rf433Device) => Promise<void>;
	}

	let { isOpen = $bindable(false), device: _device = null, onClose, onSave }: Props = $props();

	// Internal form state with hex codes as strings
	let formData = $state({
		id: '',
		label: '',
		code_on: '',
		code_off: '',
		bit_length: 24,
		protocol: 1,
		pulse_length: 0,
		repeat: 4
	});

	let errors = $state<Record<string, string>>({});
	let isSaving = $state(false);

	const isEditMode = $derived(_device !== null && _device !== undefined);
	const modalTitle = $derived(isEditMode ? 'Edit Device' : 'Add Device');

	// Auto-generate ID from label (slugify)
	function slugify(text: string): string {
		return text
			.toLowerCase()
			.trim()
			.replace(/[^\w\s-]/g, '') // Remove non-word chars except spaces and dashes
			.replace(/[\s_]+/g, '_') // Replace spaces and underscores with single underscore
			.replace(/-+/g, '_') // Replace dashes with underscores
			.replace(/^_+|_+$/g, ''); // Remove leading/trailing underscores
	}

	// Auto-update ID when label changes (only in add mode)
	$effect(() => {
		if (!isEditMode && formData.label) {
			formData.id = slugify(formData.label);
		}
	});

	// Reset form when modal opens/closes or device changes
	$effect(() => {
		if (isOpen && _device) {
			// Convert numeric codes to hex strings for display
			formData = {
				..._device,
				code_on: '0x' + _device.code_on.toString(16).toUpperCase(),
				code_off: '0x' + _device.code_off.toString(16).toUpperCase()
			};
		} else if (isOpen && !_device) {
			formData = {
				id: '',
				label: '',
				code_on: '',
				code_off: '',
				bit_length: 24,
				protocol: 1,
				pulse_length: 0,
				repeat: 4
			};
		}
		errors = {};
	});

	function validateForm(): boolean {
		const newErrors: Record<string, string> = {};

		// Label validation (will auto-generate ID)
		if (!formData.label.trim()) {
			newErrors.label = 'Label is required';
		}

		// Validate auto-generated ID
		if (!formData.id || !formData.id.trim()) {
			newErrors.label = 'Label must contain valid characters';
		} else if (!/^[a-zA-Z0-9_-]+$/.test(formData.id)) {
			newErrors.label = 'Label contains invalid characters';
		}

		// Hex code validation
		const hexPattern = /^(0x)?[0-9A-Fa-f]+$/;
		if (!formData.code_on.trim()) {
			newErrors.code_on = 'ON code is required';
		} else if (!hexPattern.test(formData.code_on)) {
			newErrors.code_on = 'Invalid format (hex: e.g. 0x1234 or 1234)';
		}

		if (!formData.code_off.trim()) {
			newErrors.code_off = 'OFF code is required';
		} else if (!hexPattern.test(formData.code_off)) {
			newErrors.code_off = 'Invalid format (hex: e.g. 0x1234 or 1234)';
		}

		// Numeric validations
		if (formData.bit_length < 1 || formData.bit_length > 32) {
			newErrors.bit_length = 'Bit length must be between 1 and 32';
		}

		if (formData.protocol < 1 || formData.protocol > 12) {
			newErrors.protocol = 'Protocol must be between 1 and 12';
		}

		if (formData.pulse_length < 0) {
			newErrors.pulse_length = 'Pulse length cannot be negative';
		}

		if (formData.repeat < 1 || formData.repeat > 10) {
			newErrors.repeat = 'Repeat count must be between 1 and 10';
		}

		errors = newErrors;
		return Object.keys(newErrors).length === 0;
	}

	async function handleSubmit() {
		if (!validateForm()) {
			return;
		}

		isSaving = true;
		try {
			// Normalize hex codes (ensure 0x prefix) and convert to numbers
			const hexOn = formData.code_on.startsWith('0x') ? formData.code_on : `0x${formData.code_on}`;
			const hexOff = formData.code_off.startsWith('0x')
				? formData.code_off
				: `0x${formData.code_off}`;

			const normalizedDevice: Rf433Device = {
				id: formData.id,
				label: formData.label,
				code_on: parseInt(hexOn, 16),
				code_off: parseInt(hexOff, 16),
				bit_length: formData.bit_length,
				protocol: formData.protocol,
				pulse_length: formData.pulse_length,
				repeat: formData.repeat
			};

			await onSave(normalizedDevice);
			isOpen = false;
		} catch (err) {
			errors.submit = err instanceof Error ? err.message : 'Save error';
		} finally {
			isSaving = false;
		}
	}

	function handleClose() {
		if (!isSaving) {
			isOpen = false;
			onClose();
		}
	}
</script>

{#if isOpen}
	<!-- Backdrop with blur -->
	<div class="fixed inset-0 z-50 bg-black/50 backdrop-blur-sm"></div>

	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center"
		transition:fly={{ y: 50 }}
		use:focusTrap
	>
		<div
			class="bg-base-100 shadow-secondary/30 rounded-box pointer-events-auto flex max-h-[90vh] w-full max-w-2xl flex-col p-4 shadow-lg"
		>
			<!-- Header -->
			<div class="flex items-center justify-between mb-2">
				<h2 class="text-base-content text-start text-2xl font-bold">{modalTitle}</h2>
				<button
					class="btn btn-ghost btn-sm btn-circle"
					onclick={handleClose}
					disabled={isSaving}
					aria-label="Close"
				>
					<X class="h-5 w-5" />
				</button>
			</div>

			<div class="divider my-2"></div>

			<form
				onsubmit={(e) => {
					e.preventDefault();
					handleSubmit();
				}}
				class="flex flex-col flex-1 overflow-hidden"
			>
				<div class="overflow-y-auto flex-1 px-2">
					<!-- Label -->
					<div class="form-control w-full mb-4">
						<label class="label" for="device-label">
							<span class="label-text">Device Name <span class="text-error">*</span></span>
						</label>
						<input
							id="device-label"
							type="text"
							bind:value={formData.label}
							disabled={isSaving}
							placeholder="e.g. Living Room Lamp"
							autocomplete="off"
							class="input input-bordered w-full"
							class:input-error={errors.label}
						/>
						{#if errors.label}
							<div class="label">
								<span class="label-text-alt text-error">{errors.label}</span>
							</div>
						{:else if formData.id && !isEditMode}
							<div class="label">
								<span class="label-text-alt opacity-60">Auto ID: {formData.id}</span>
							</div>
						{:else if isEditMode}
							<div class="label">
								<span class="label-text-alt opacity-60">ID: {formData.id}</span>
							</div>
						{/if}
					</div>

					<!-- Codes -->
					<div class="grid grid-cols-1 md:grid-cols-2 gap-4 mb-4 min-w-0">
						<div class="form-control w-full min-w-0">
							<label class="label" for="code-on">
								<span class="label-text">ON Code (hex) <span class="text-error">*</span></span>
							</label>
							<input
								id="code-on"
								type="text"
								bind:value={formData.code_on}
								disabled={isSaving}
								placeholder="0x123456"
								class="input input-bordered w-full font-mono overflow-x-auto"
								class:input-error={errors.code_on}
							/>
							{#if errors.code_on}
								<div class="label">
									<span class="label-text-alt text-error">{errors.code_on}</span>
								</div>
							{/if}
						</div>

						<div class="form-control w-full min-w-0">
							<label class="label" for="code-off">
								<span class="label-text">OFF Code (hex) <span class="text-error">*</span></span>
							</label>
							<input
								id="code-off"
								type="text"
								bind:value={formData.code_off}
								disabled={isSaving}
								placeholder="0xABCDEF"
								class="input input-bordered w-full font-mono overflow-x-auto"
								class:input-error={errors.code_off}
							/>
							{#if errors.code_off}
								<div class="label">
									<span class="label-text-alt text-error">{errors.code_off}</span>
								</div>
							{/if}
						</div>
					</div>

					<!-- Technical params -->
					<div class="grid grid-cols-1 md:grid-cols-2 gap-4 mb-4 min-w-0">
						<div class="form-control w-full min-w-0">
							<label class="label" for="bit-length">
								<span class="label-text">Bit Length</span>
							</label>
							<input
								id="bit-length"
								type="number"
								bind:value={formData.bit_length}
								disabled={isSaving}
								min="1"
								max="32"
								class="input input-bordered w-full"
								class:input-error={errors.bit_length}
							/>
							{#if errors.bit_length}
								<div class="label">
									<span class="label-text-alt text-error">{errors.bit_length}</span>
								</div>
							{/if}
						</div>

						<div class="form-control w-full min-w-0">
							<label class="label" for="protocol">
								<span class="label-text">Protocol (1-12)</span>
							</label>
							<select
								id="protocol"
								bind:value={formData.protocol}
								disabled={isSaving}
								class="select select-bordered w-full"
								class:select-error={errors.protocol}
							>
								{#each Array.from({ length: 12 }, (_, i) => i + 1) as protocolNum}
									<option value={protocolNum}>Protocol {protocolNum}</option>
								{/each}
							</select>
							{#if errors.protocol}
								<div class="label">
									<span class="label-text-alt text-error">{errors.protocol}</span>
								</div>
							{/if}
						</div>
					</div>

					<div class="grid grid-cols-1 md:grid-cols-2 gap-4 mb-4">
						<div class="form-control w-full">
							<label class="label" for="pulse-length">
								<span class="label-text">Pulse Length (µs)</span>
								<span class="label-text-alt opacity-60">(0 = auto)</span>
							</label>
							<input
								id="pulse-length"
								type="number"
								bind:value={formData.pulse_length}
								disabled={isSaving}
								min="0"
								class="input input-bordered w-full"
								class:input-error={errors.pulse_length}
							/>
							{#if errors.pulse_length}
								<div class="label">
									<span class="label-text-alt text-error">{errors.pulse_length}</span>
								</div>
							{/if}
						</div>

						<div class="form-control w-full">
							<label class="label" for="repeat">
								<span class="label-text">Repeats</span>
							</label>
							<input
								id="repeat"
								type="number"
								bind:value={formData.repeat}
								disabled={isSaving}
								min="1"
								max="10"
								class="input input-bordered w-full"
								class:input-error={errors.repeat}
							/>
							{#if errors.repeat}
								<div class="label">
									<span class="label-text-alt text-error">{errors.repeat}</span>
								</div>
							{/if}
						</div>
					</div>

					{#if errors.submit}
						<div class="alert alert-error mb-4">
							<span>{errors.submit}</span>
						</div>
					{/if}
				</div>

				<div class="divider my-2"></div>

				<div class="flex flex-wrap justify-end gap-2">
					<button
						type="button"
						class="btn btn-ghost inline-flex flex-none items-center"
						onclick={handleClose}
						disabled={isSaving}
					>
						<X class="mr-2 h-5 w-5" />
						<span>Cancel</span>
					</button>
					<button
						type="submit"
						class="btn btn-primary text-primary-content inline-flex flex-none items-center"
						disabled={isSaving}
					>
						<DeviceFloppy class="mr-2 h-5 w-5" />
						<span>{isSaving ? 'Saving...' : 'Save'}</span>
					</button>
				</div>
			</form>
		</div>
	</div>
{/if}
