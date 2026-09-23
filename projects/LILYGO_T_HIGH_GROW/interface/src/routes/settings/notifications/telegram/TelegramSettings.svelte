<script lang="ts">
	import { onMount } from 'svelte';
	import { page } from '$app/state';
	import { modals, type ModalComponent } from 'svelte-modals';
	import { user } from '$lib/stores/user';
	import { notifications } from '$lib/components/toasts/notifications';
	import { TelegramApiService } from '$lib/services/TelegramApiService';
	import Spinner from '$lib/components/Spinner.svelte';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import ChatIdsDialog from './ChatIdsDialog.svelte';
	import Telegram from '~icons/tabler/brand-telegram';
	import Save from '~icons/tabler/device-floppy';
	import Download from '~icons/tabler/download';
	import type { TelegramSettings } from '$lib/types/models';

	let telegramSettings: TelegramSettings = $state({
		enabled: false,
		bot_token: '',
		chat_id: ''
	});
	let formField: any = $state();
	let loading: boolean = $state(true);
	let fetchingChatId: boolean = $state(false);

	let formErrors = $state({
		bot_token: false,
		chat_id: false
	});

	const toModalComponent = (component: unknown) => component as unknown as ModalComponent<any>;

	async function getTelegramSettings() {
		try {
			const response = await fetch('/rest/telegramSettings', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			if (response.ok) {
				telegramSettings = await response.json();
			} else {
				notifications.error('Failed to load Telegram settings.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
			notifications.error('Error loading Telegram settings.', 3000);
		} finally {
			loading = false;
		}
	}

	async function postTelegramSettings(data: TelegramSettings) {
		try {
			const response = await fetch('/rest/telegramSettings', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});
			if (response.status === 200) {
				notifications.success('Telegram settings updated.', 3000);
				telegramSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
			notifications.error('Failed to save settings.', 3000);
		}
	}

	function handleSubmit() {
		let valid = true;

		// Validate Bot Token (should not be empty when enabled)
		if (telegramSettings.enabled && telegramSettings.bot_token.trim().length < 10) {
			valid = false;
			formErrors.bot_token = true;
		} else {
			formErrors.bot_token = false;
		}

		// Validate Chat ID (should not be empty when enabled, typically numeric or starts with -)
		if (telegramSettings.enabled && telegramSettings.chat_id.trim().length === 0) {
			valid = false;
			formErrors.chat_id = true;
		} else {
			formErrors.chat_id = false;
		}

		// Submit if valid
		if (valid) {
			postTelegramSettings(telegramSettings);
		} else {
			notifications.warning('Please fix validation errors.', 3000);
		}
	}

	function preventDefault(fn: () => void) {
		return function (event: Event) {
			event.preventDefault();
			fn();
		};
	}

	function formatError(result: { error?: string; httpCode?: number }) {
		if (result.httpCode === 401 || result.httpCode === 403) {
			return 'Invalid bot token (401/403).';
		}
		if (result.httpCode === 429 || (result.error && result.error.includes('429'))) {
			return 'Telegram rate limit — try again in a moment.';
		}
		if (result.error === 'telegram/response_too_large') {
			return 'Telegram response was too large — try again after sending a single message to the bot.';
		}
		return result.error || 'Failed to fetch Chat ID';
	}

	async function fetchChatId() {
		const token = telegramSettings.bot_token.trim();
		if (token.length < 10 || token.indexOf(':') === -1) {
			notifications.warning('Please enter a valid bot token first.', 3000);
			return;
		}

		fetchingChatId = true;
		try {
			const result = await TelegramApiService.fetchChatId(
				page.data.features.security,
				$user.bearer_token,
				token
			);

			if (result.ok && result.chats && result.chats.length > 0) {
				modals.open(toModalComponent(ChatIdsDialog), {
					title: 'Available Chat IDs',
					chats: result.chats
				});
			} else if (result.ok && result.chats && result.chats.length === 0) {
				notifications.warning(
					'No chats found. Send a message to the bot (e.g. /start) and try again.',
					5000
				);
			} else {
				notifications.error(formatError(result), 4000);
			}
		} catch (error) {
			console.error('Error fetching chat ID:', error);
			notifications.error('Error fetching Chat ID.', 3000);
		} finally {
			fetchingChatId = false;
		}
	}

	onMount(() => {
		if (!page.data.features.security || $user.admin) {
			getTelegramSettings();
		} else {
			loading = false;
		}
	});
</script>

{#if loading}
	<div class="card bg-base-200 shadow-primary/50 shadow-lg h-full">
		<div class="card-body p-3 md:p-4 space-y-4">
			<h2 class="card-title text-lg">
				<Telegram class="h-6 w-6" />
				Telegram Settings
			</h2>
			<div class="flex justify-center items-center py-8">
				<Spinner />
			</div>
		</div>
	</div>
{:else if !page.data.features.security || $user.admin}
	<div class="card bg-base-200 shadow-primary/50 shadow-lg h-full">
		<div class="card-body p-3 md:p-4 flex h-full flex-col gap-4">
			<h2 class="card-title text-lg">
				<Telegram class="h-6 w-6" />
				Telegram Settings
			</h2>
			<form
				onsubmit={preventDefault(handleSubmit)}
				novalidate
				bind:this={formField}
				class="flex flex-1 flex-col gap-3"
			>
				<!-- Hidden username field for accessibility -->
				<input type="text" name="username" autocomplete="username" class="hidden" tabindex="-1" />

				<!-- Enable -->
				<label class="label inline-flex cursor-pointer content-end justify-start gap-4 text-base">
					<input
						type="checkbox"
						bind:checked={telegramSettings.enabled}
						class="checkbox checkbox-primary"
					/>
					Enable Telegram Notifications
				</label>

				<!-- Bot Token -->
				<div>
					<label class="label" for="bot_token">Bot Token</label>
					<InputPassword
						id="bot_token"
						bind:value={telegramSettings.bot_token}
						placeholder="123456789:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgh"
						classes="input w-full {formErrors.bot_token ? 'border-error border-2' : ''}"
						required={telegramSettings.enabled}
					/>
					<label class="label" for="bot_token">
						<span class="text-xs opacity-70">Obtain from @BotFather on Telegram</span>
						<span class="text-error {formErrors.bot_token ? '' : 'hidden'}"
							>Bot token is required</span
						>
					</label>
				</div>

				<!-- Chat ID -->
				<div>
					<label class="label" for="chat_id">Chat ID</label>
					<div class="flex flex-col sm:flex-row gap-2 sm:items-center">
						<input
							type="text"
							class="input w-full {formErrors.chat_id ? 'border-error border-2' : ''}"
							bind:value={telegramSettings.chat_id}
							id="chat_id"
							placeholder="123456789 or -123456789"
							required={telegramSettings.enabled}
						/>
						<button
							type="button"
							class="btn btn-primary btn-sm whitespace-nowrap self-center"
							disabled={fetchingChatId || telegramSettings.bot_token.trim().length < 10}
							aria-busy={fetchingChatId}
							onclick={fetchChatId}
						>
							{#if fetchingChatId}
								<span class="loading loading-spinner loading-sm"></span>
								Fetching...
							{:else}
								<Download class="h-5 w-5" />
								Fetch Chat ID
							{/if}
						</button>
					</div>
					<label class="label" for="chat_id">
						<span class="text-xs opacity-70"
							>Send /start to your bot, then click "Fetch Chat ID"</span
						>
						<span class="text-error {formErrors.chat_id ? '' : 'hidden'}">Chat ID is required</span>
					</label>
				</div>

				<div class="text-sm opacity-70 space-y-2 leading-relaxed">
					<p><strong>How to get your Bot Token:</strong></p>
					<ol class="list-decimal ml-5 space-y-1">
						<li>
							Open Telegram and search for <code class="px-1 bg-base-300 rounded">@BotFather</code>
						</li>
						<li>
							Send <code class="px-1 bg-base-300 rounded">/newbot</code> and follow instructions
						</li>
						<li>
							Copy the token that looks like <code class="px-1 bg-base-300 rounded"
								>123456789:ABC...</code
							>
						</li>
					</ol>

					<details class="pt-2">
						<summary class="cursor-pointer font-bold"
							>Alternatively: Manual Chat ID retrieval</summary
						>
						<ol class="list-decimal ml-5 space-y-1 mt-2">
							<li>Send a message to your bot</li>
							<li>
								Visit
								<code class="px-1 bg-base-300 rounded break-all">
									https://api.telegram.org/bot&lt;YOUR_BOT_TOKEN&gt;/getUpdates
								</code>
							</li>
							<li>
								Look for
								<code class="px-1 bg-base-300 rounded break-all"
									>"chat":&#123;"id":123456789&#125;</code
								>
								in the response
							</li>
						</ol>
					</details>
				</div>

				<!-- Submit (bottom of card) -->
				<div class="card-actions mt-auto justify-end pt-2">
					<button type="submit" class="btn btn-primary btn-sm">
						<Save class="h-5 w-5" />
						Save Settings
					</button>
				</div>
			</form>
		</div>
	</div>
{:else}
	<div class="card bg-base-200 shadow-primary/50 shadow-lg h-full">
		<div class="card-body p-3 md:p-4">
			<div class="alert alert-warning">
				<span>Admin privileges required to view and edit Telegram settings.</span>
			</div>
		</div>
	</div>
{/if}
