<script lang="ts">
	import { modals, type ModalProps } from 'svelte-modals';
	import { focusTrap } from 'svelte-focus-trap';
	import { fly } from 'svelte/transition';
	import { notifications } from '$lib/components/toasts/notifications';
	import Copy from '~icons/tabler/copy';
	import Check from '~icons/tabler/check';

	type ChatEntry = {
		id: string;
		type: string;
		name?: string;
	};

	interface Props extends ModalProps {
		title: string;
		chats: ChatEntry[];
	}

	let { isOpen, title, chats }: Props = $props();

	let lastCopiedId: string | null = $state(null);

	async function copyText(value: string) {
		// Clipboard API requires a secure context on many platforms.
		// Fallback to execCommand for http/insecure contexts (common on embedded devices).
		if (navigator.clipboard?.writeText && window.isSecureContext) {
			await navigator.clipboard.writeText(value);
			return;
		}

		const textarea = document.createElement('textarea');
		textarea.value = value;
		textarea.setAttribute('readonly', '');
		textarea.style.position = 'fixed';
		textarea.style.left = '-9999px';
		textarea.style.top = '0';
		document.body.appendChild(textarea);
		textarea.focus();
		textarea.select();
		const ok = document.execCommand('copy');
		document.body.removeChild(textarea);
		if (!ok) throw new Error('execCommand(copy) failed');
	}

	async function handleCopy(id: string) {
		try {
			await copyText(id);
			lastCopiedId = id;
			notifications.success('Chat ID copied to clipboard.', 2000);
			setTimeout(() => {
				if (lastCopiedId === id) lastCopiedId = null;
			}, 2000);
		} catch (error) {
			console.error('Failed to copy Chat ID:', error);
			notifications.error('Failed to copy Chat ID.', 2500);
		}
	}

	function handleClose() {
		modals.close();
	}
</script>

{#if isOpen}
	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-start justify-center overflow-y-auto p-4 sm:items-center"
		transition:fly={{ y: 50 }}
		use:focusTrap={{}}
	>
		<div
			class="rounded-box bg-base-100 shadow-secondary/30 pointer-events-auto flex max-h-[calc(100dvh-2rem)] w-full max-w-xs min-h-0 flex-col overflow-hidden p-4 shadow-lg sm:max-w-sm md:max-w-md"
			aria-labelledby="chat-ids-dialog-title"
		>
			<h2
				id="chat-ids-dialog-title"
				class="text-base-content text-start text-2xl font-bold break-words"
			>
				{title}
			</h2>
			<div class="divider my-2"></div>
			<div class="min-h-0 flex-1 overflow-y-auto space-y-2">
				<p class="text-sm opacity-70">Copy a Chat ID, then paste it into the Chat ID field.</p>

				{#if !chats || chats.length === 0}
					<div class="alert alert-warning">
						<span>No chats found.</span>
					</div>
				{:else}
					<ul class="space-y-2">
						{#each chats as chat (chat.id)}
							<li class="rounded-box bg-base-200 p-3">
								<div class="flex items-start justify-between gap-3">
									<div class="min-w-0">
										<div class="font-semibold break-all">{chat.id}</div>
										<div class="text-xs opacity-70 break-words">
											Type: {chat.type}{#if chat.name}
												· Name: {chat.name}{/if}
										</div>
									</div>

									<button
										type="button"
										class="btn btn-ghost btn-square btn-sm"
										onclick={() => handleCopy(chat.id)}
										title="Copy Chat ID"
										aria-label="Copy Chat ID"
									>
										{#if lastCopiedId === chat.id}
											<Check class="h-4 w-4" />
										{:else}
											<Copy class="h-4 w-4" />
										{/if}
									</button>
								</div>
							</li>
						{/each}
					</ul>
				{/if}
			</div>
			<div class="divider my-2"></div>
			<div class="flex justify-end gap-2">
				<button class="btn btn-primary" type="button" onclick={handleClose}>Close</button>
			</div>
		</div>
	</div>
{/if}
