<script lang="ts">
	import Activity from '~icons/tabler/activity';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';

	interface TailLine {
		timestampMs: number;
		level: string;
		tag: string;
		message: string;
	}

	interface LoggingConfigDto {
		level: string;
	}

	const securityEnabled = page.data.features.security;
	const isAdmin = $derived(page.data.features.security ? $user.admin : true);

	let tail = $state<TailLine[]>([]);
	let tailError = $state('');
	let tailInterval: ReturnType<typeof setInterval> | undefined;

	let loggingConfig = $state<LoggingConfigDto>({ level: 'info' });
	let savingConfig = $state(false);
	const levels = ['none', 'error', 'warn', 'info', 'debug'];

	const authHeaders = (): Record<string, string> => {
		if (!securityEnabled) return {};
		return { Authorization: `Bearer ${$user.bearer_token}` };
	};

	const refreshLoggingConfig = () =>
		fetch('/api/config', { headers: authHeaders() })
			.then((res) => res.json())
			.then((data) => {
				if (data.logging) {
					loggingConfig = {
						level: data.logging.level ?? 'info'
					};
				}
			})
			.catch(() => {
				/* ignore */
			});

	const saveLoggingSettings = async () => {
		if (!isAdmin) return;
		savingConfig = true;
		try {
			const res = await fetch('/api/config', {
				method: 'GET',
				headers: authHeaders()
			});
			const config = await res.json();
			config.logging = { level: loggingConfig.level };

			await fetch('/api/config', {
				method: 'POST',
				headers: {
					...authHeaders(),
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(config)
			});

			await refreshLoggingConfig();
			refreshTail();
		} catch (err) {
			console.error('Failed to save logging config:', err);
		} finally {
			savingConfig = false;
		}
	};

	const refreshTail = () => {
		if (!isAdmin) return;
		const requestedLines = 20;

		fetch(`/rest/logs/tail?lines=${requestedLines}`, { headers: authHeaders() })
			.then((res) => {
				if (!res.ok) throw new Error(res.statusText);
				return res.json();
			})
			.then((data) => {
				tail = data.lines ?? [];
				tailError = '';
			})
			.catch((err) => {
				tailError = `Tail fetch error: ${err}`;
			});
	};

	const refreshAll = () => {
		refreshTail();
		refreshLoggingConfig();
	};

	$effect(() => {
		if (!isAdmin) return;
		refreshAll();
		tailInterval = setInterval(refreshTail, 5000);

		return () => {
			if (tailInterval) {
				clearInterval(tailInterval);
			}
		};
	});
</script>

<div class="card bg-base-200 shadow-primary/50 shadow-lg">
	<div class="card-body p-4 space-y-3">
		<div class="flex items-center gap-2">
			<Activity class="w-5 h-5" />
			<h2 class="card-title text-lg">Live Tail {isAdmin ? '(admin)' : ''}</h2>
		</div>

		{#if !isAdmin}
			<div class="alert alert-info text-sm">
				<span>Admin only</span>
			</div>
		{:else}
			{#if tailError}
				<div class="alert alert-warning text-xs">{tailError}</div>
			{/if}
			<div class="bg-base-300 rounded-lg p-2 max-h-80 overflow-y-auto font-mono text-xs space-y-1">
				{#if tail.length === 0}
					<div class="opacity-60">No log lines yet.</div>
				{:else}
					{#each tail as line}
						<div class="flex gap-2">
							<span class="w-10 shrink-0 text-primary">{line.level}</span>
							<span class="w-24 shrink-0 text-accent">{line.tag}</span>
							<span class="flex-1 break-all">{line.message}</span>
						</div>
					{/each}
				{/if}
			</div>

			<div class="shrink-0">
				<div class="divider my-2">Logging Settings</div>

				<div class="grid grid-cols-1 md:grid-cols-3 gap-2 items-end">
					<div class="form-control">
						<label class="label text-xs uppercase opacity-70" for="log-level">Log level</label>
						<select
							id="log-level"
							class="select select-bordered select-sm"
							bind:value={loggingConfig.level}
						>
							{#each levels as lvl}
								<option value={lvl}>{lvl}</option>
							{/each}
						</select>
					</div>

					<div class="form-control">
						<div class="label text-xs uppercase opacity-70">Ring buffer size</div>
						<div class="text-sm py-2 px-3 bg-base-200 rounded">20 (fixed)</div>
					</div>

					<div class="flex justify-end gap-2">
						<button
							class="btn btn-xs btn-primary"
							class:loading={savingConfig}
							onclick={saveLoggingSettings}
						>
							Save
						</button>
						<button class="btn btn-xs" onclick={refreshAll}>Refresh</button>
					</div>
				</div>
			</div>
		{/if}
	</div>
</div>
