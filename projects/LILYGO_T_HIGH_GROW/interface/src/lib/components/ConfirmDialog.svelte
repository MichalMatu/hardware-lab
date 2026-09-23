<script lang="ts">
	import { modals, type ModalProps } from 'svelte-modals';
	import { focusTrap } from 'svelte-focus-trap';
	import { fly } from 'svelte/transition';
	import Cancel from '~icons/tabler/x';
	import Check from '~icons/tabler/check';

	interface Props extends ModalProps {
		title: string;
		message?: string;
		// Use only for trusted markup. Prefer `message` for plain text.
		messageHtml?: string;
		onConfirm: () => void;
		labels?: {
			cancel: { label: string; icon: typeof Cancel };
			confirm: { label: string; icon: typeof Cancel };
		};
	}

	let {
		isOpen,
		title,
		message = '',
		messageHtml,
		onConfirm,
		labels = {
			cancel: { label: 'Cancel', icon: Cancel },
			confirm: { label: 'OK', icon: Check }
		}
	}: Props = $props();

	function handleConfirm() {
		onConfirm?.();
		modals.close();
	}

	function handleCancel() {
		modals.close();
	}
</script>

{#if isOpen}
	{@const SvelteComponent = labels?.confirm.icon}
	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-start justify-center overflow-y-auto p-4 sm:items-center"
		transition:fly={{ y: 50 }}
		use:focusTrap={{}}
	>
		<div
			class="rounded-box bg-base-100 shadow-secondary/30 pointer-events-auto flex max-h-[calc(100dvh-2rem)] w-full max-w-xs min-h-0 flex-col overflow-hidden p-4 shadow-lg sm:max-w-sm md:max-w-md"
		>
			<h2 class="text-base-content text-start text-2xl font-bold break-words">{title}</h2>
			<div class="divider my-2"></div>
			<div class="min-h-0 flex-1 overflow-y-auto">
				{#if messageHtml}
					<p class="text-base-content mb-1 text-start break-words">
						{@html messageHtml}
					</p>
				{:else}
					<p class="text-base-content mb-1 text-start break-words whitespace-pre-line">{message}</p>
				{/if}
			</div>
			<div class="divider my-2"></div>
			<div class="flex justify-end gap-2">
				<button class="btn btn-primary inline-flex items-center" onclick={handleCancel}
					><labels.cancel.icon class="h-5 w-5" /><span>{labels?.cancel.label}</span></button
				>
				<button
					class="btn btn-warning text-warning-content inline-flex items-center"
					onclick={handleConfirm}
					><SvelteComponent class="h-5 w-5" /><span>{labels?.confirm.label}</span></button
				>
			</div>
		</div>
	</div>
{/if}
