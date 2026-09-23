<script lang="ts">
	import { page } from '$app/state';
	import { user } from '$lib/stores/user';
	import { notifications } from '$lib/components/toasts/notifications';
	import Send from '~icons/tabler/send';
	import Telegram from '~icons/tabler/brand-telegram';

	type TelegramTestResult = {
		ok: boolean;
		configured?: boolean;
		httpCode?: number;
		error?: string;
		tlsError?: string;
		response?: string;
	};

	let testText: string = $state('Hello from ESP32! 🌱');
	let sending: boolean = $state(false);
	let lastResult: TelegramTestResult | null = $state(null);
	let lastError: string | null = $state(null);

	const isAdminAllowed = () => !page.data.features.security || $user.admin;

	async function sendTest() {
		lastError = null;
		lastResult = null;

		if (!isAdminAllowed()) {
			lastError = 'Admin authorization required.';
			notifications.error('Admin authorization required.', 3000);
			return;
		}

		const text = testText.trim();
		if (!text) {
			lastError = 'Please enter a test message.';
			return;
		}

		sending = true;
		const controller = new AbortController();
		const timeoutId = window.setTimeout(() => controller.abort(), 35000);

		try {
			const response = await fetch('/api/notifications/telegram/test', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ text }),
				signal: controller.signal
			});

			const res: TelegramTestResult = await response.json();
			lastResult = res;

			if (res.ok) {
				notifications.success('Telegram test message sent successfully!', 3000);
			} else if (res.configured === false) {
				notifications.error(
					'Telegram is not configured. Please save bot token and chat ID first.',
					5000
				);
			} else {
				notifications.error(res.error ?? 'Telegram test failed.', 4000);
			}
		} catch (e) {
			const msg = e instanceof Error ? e.message : String(e);
			const isAbort =
				e instanceof DOMException &&
				(e.name === 'AbortError' || e.message.toLowerCase().includes('aborted'));
			if (isAbort) {
				lastError = 'Timed out waiting for device response (possible reconnect during TLS).';
				notifications.error(lastError, 5000);
				return;
			}
			lastError = msg;
			notifications.error(msg, 4000);
		} finally {
			window.clearTimeout(timeoutId);
			sending = false;
		}
	}
</script>

<div class="card bg-base-200 shadow-primary/50 shadow-lg h-full">
	<div class="card-body p-3 md:p-4 space-y-3">
		<h3 class="card-title text-lg">
			<Telegram class="h-6 w-6" />
			Test Message
		</h3>

		{#if page.data.features.security && !$user.admin}
			<p class="text-sm opacity-70">
				This action requires an <b>admin</b> account.
			</p>
		{/if}

		<label class="form-control w-full">
			<div class="label">
				<span class="label-text">Test message</span>
				<span class="label-text-alt opacity-70">Max 1024 chars</span>
			</div>
			<textarea
				class="textarea textarea-bordered w-full"
				rows="3"
				maxlength="1024"
				placeholder="Type a test message to send via Telegram..."
				bind:value={testText}
				disabled={sending}
			></textarea>
		</label>

		<div class="flex items-center gap-3">
			{#if lastError}
				<span class="text-sm text-error flex-1">{lastError}</span>
			{/if}
			<button
				class="btn btn-primary btn-sm ml-auto"
				disabled={sending || !isAdminAllowed()}
				onclick={sendTest}
			>
				{#if sending}
					<span class="loading loading-spinner loading-sm"></span>
					Sending...
				{:else}
					<Send class="h-5 w-5" />
					Send Test
				{/if}
			</button>
		</div>

		{#if lastResult}
			<div class="p-3 rounded-lg {lastResult.ok ? 'bg-success/20' : 'bg-error/20'}">
				<div class="text-sm space-y-1">
					{#if lastResult.ok}
						<p class="font-semibold text-success">
							✓ Sent successfully (HTTP {lastResult.httpCode})
						</p>
					{:else if lastResult.configured === false}
						<p class="font-semibold text-error">✗ Telegram is not configured on the device</p>
						<p class="text-xs opacity-80">
							Please save bot token and chat ID in the settings above.
						</p>
					{:else}
						<p class="font-semibold text-error">
							✗ Failed (HTTP {lastResult.httpCode ?? 'n/a'}): {lastResult.error ?? 'unknown error'}
						</p>
					{/if}

					{#if lastResult.tlsError}
						<p class="text-xs opacity-80 pt-1">TLS error: {lastResult.tlsError}</p>
					{/if}

					{#if lastResult.response}
						{@const parsed = (() => {
							try {
								return JSON.parse(lastResult.response);
							} catch {
								return null;
							}
						})()}

						<details class="text-xs opacity-70 pt-1">
							<summary class="cursor-pointer hover:opacity-100">Response details</summary>
							<div class="mt-2 p-3 bg-base-300 rounded space-y-2">
								{#if parsed}
									<div class="space-y-1.5">
										<div class="flex gap-2">
											<span class="font-semibold opacity-60 min-w-[100px]">Status:</span>
											<span>{parsed.ok ? '✓ Success' : '✗ Failed'}</span>
										</div>

										{#if parsed.result}
											<div class="flex gap-2">
												<span class="font-semibold opacity-60 min-w-[100px]">Message ID:</span>
												<span>{parsed.result.message_id}</span>
											</div>

											{#if parsed.result.from}
												<div class="flex gap-2">
													<span class="font-semibold opacity-60 min-w-[100px]">Bot:</span>
													<span
														>{parsed.result.from.first_name} (@{parsed.result.from.username})</span
													>
												</div>
											{/if}

											{#if parsed.result.chat}
												<div class="flex gap-2">
													<span class="font-semibold opacity-60 min-w-[100px]">Sent to:</span>
													<span>{parsed.result.chat.first_name} (ID: {parsed.result.chat.id})</span>
												</div>
											{/if}

											{#if parsed.result.date}
												<div class="flex gap-2">
													<span class="font-semibold opacity-60 min-w-[100px]">Timestamp:</span>
													<span>{new Date(parsed.result.date * 1000).toLocaleString()}</span>
												</div>
											{/if}

											{#if parsed.result.text}
												<div class="flex gap-2">
													<span class="font-semibold opacity-60 min-w-[100px]">Text sent:</span>
													<span class="italic">"{parsed.result.text}"</span>
												</div>
											{/if}
										{/if}

										{#if parsed.description}
											<div class="flex gap-2">
												<span class="font-semibold opacity-60 min-w-[100px]">Description:</span>
												<span>{parsed.description}</span>
											</div>
										{/if}
									</div>

									<details class="mt-3 pt-2 border-t border-base-content/10">
										<summary
											class="cursor-pointer hover:opacity-100 text-[11px] uppercase tracking-wide opacity-50"
										>
											Raw JSON
										</summary>
										<pre
											class="mt-2 p-2 bg-base-100 rounded overflow-x-auto text-[11px] leading-relaxed">{JSON.stringify(
												parsed,
												null,
												2
											)}</pre>
									</details>
								{:else}
									<pre class="overflow-x-auto">{lastResult.response}</pre>
								{/if}
							</div>
						</details>
					{/if}
				</div>
			</div>
		{/if}

		<div class="mt-4 text-sm opacity-70 leading-relaxed space-y-1">
			<p><strong>Note:</strong> Testing requires:</p>
			<ul class="list-disc ml-5 space-y-0.5">
				<li>Valid bot token and chat ID saved in settings above</li>
				<li>Active WiFi connection</li>
				<li>Correct time (NTP recommended for HTTPS/TLS)</li>
			</ul>
		</div>
	</div>
</div>
