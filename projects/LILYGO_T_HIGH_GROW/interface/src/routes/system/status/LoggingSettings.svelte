<script lang="ts">
	import Settings from '~icons/tabler/settings';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';

	interface LoggingConfigDto {
		level: string;
	}

	const securityEnabled = page.data.features.security;

	let loggingConfig = $state<LoggingConfigDto>({
		level: 'info'
	});
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
		} catch (err) {
			console.error('Failed to save logging config:', err);
		} finally {
			savingConfig = false;
		}
	};

	$effect(() => {
		refreshLoggingConfig();
	});
</script>

<div class="card bg-base-200 shadow-primary/50 shadow-lg h-full">
	<div class="card-body p-4 space-y-3">
		<div class="flex items-center gap-2">
			<Settings class="w-5 h-5" />
			<h2 class="card-title text-lg">Logging Settings</h2>
		</div>

		<div class="form-control">
			<label class="label text-xs uppercase opacity-70" for="log-normal-level">Log level</label>
			<select
				id="log-normal-level"
				class="select select-bordered select-sm"
				bind:value={loggingConfig.level}
			>
				{#each levels as lvl}
					<option value={lvl}>{lvl}</option>
				{/each}
			</select>
		</div>

		<div class="form-control">
			<label class="label text-xs uppercase opacity-70">Ring buffer size</label>
			<div class="text-sm py-2 px-3 bg-base-200 rounded">20 (fixed)</div>
		</div>

		<div class="flex justify-end gap-2">
			<button
				class="btn btn-sm btn-primary"
				class:loading={savingConfig}
				onclick={saveLoggingSettings}
			>
				Save
			</button>
			<button class="btn btn-sm" onclick={refreshLoggingConfig}>Reload</button>
		</div>
	</div>
</div>
