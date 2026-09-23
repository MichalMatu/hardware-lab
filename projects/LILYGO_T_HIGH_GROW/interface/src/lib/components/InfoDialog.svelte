<script lang="ts">
	import { focusTrap } from 'svelte-focus-trap';
	import { fly } from 'svelte/transition';
	import Check from '~icons/tabler/check';

	// provided by <Modals />

	interface Props {
		isOpen: boolean;
		title: string;
		message?: string;
		// Use only for trusted markup. Prefer `message` for plain text.
		messageHtml?: string;
		onDismiss: any;
		dismiss?: any;
	}

	const fallbackDismiss = { label: 'Dismiss', icon: Check };
	const fallbackData = {
		isOpen: false,
		title: '',
		message: '',
		onDismiss: () => {},
		dismiss: fallbackDismiss
	};

	const rawProps = $props();
	const infoProps: Props = { ...fallbackData, ...rawProps };

	const {
		isOpen = false,
		title = '',
		message = '',
		messageHtml,
		onDismiss = () => {},
		dismiss
	} = infoProps;

	// Compatibility:
	// - In this app, `dismiss` is commonly used as button config: { label, icon }.
	// - Some modal systems provide a `dismiss()` function prop to close the modal.
	const modalDismissFn = typeof dismiss === 'function' ? (dismiss as () => void) : undefined;
	const dismissButton =
		dismiss && typeof dismiss === 'object' ? (dismiss as typeof fallbackDismiss) : fallbackDismiss;

	function handleDismiss() {
		try {
			onDismiss?.();
		} finally {
			modalDismissFn?.();
		}
	}
</script>

{#if isOpen}
	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center"
		transition:fly={{ y: 50 }}
		use:focusTrap
	>
		<div
			class="rounded-box bg-base-100 shadow-secondary/30 pointer-events-auto flex min-w-fit max-w-md flex-col justify-between p-4 shadow-lg"
		>
			<h2 class="text-base-content text-start text-2xl font-bold">{title}</h2>
			<div class="divider my-2"></div>
			{#if messageHtml}
				<p class="text-base-content mb-1 text-start">{@html messageHtml}</p>
			{:else}
				<p class="text-base-content mb-1 text-start whitespace-pre-line">{message}</p>
			{/if}
			<div class="divider my-2"></div>
			<div class="flex justify-end gap-2">
				<button
					class="btn btn-warning text-warning-content inline-flex items-center"
					onclick={handleDismiss}
					><dismissButton.icon class="mr-2 h-5 w-5" /><span>{dismissButton.label}</span></button
				>
			</div>
		</div>
	</div>
{/if}
