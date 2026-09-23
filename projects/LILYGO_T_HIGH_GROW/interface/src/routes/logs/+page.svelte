<script lang="ts">
	import { onMount } from 'svelte';
	import Calendar from '~icons/tabler/calendar';
	import Download from '~icons/tabler/download';
	import Trash from '~icons/tabler/trash';
	import FileText from '~icons/tabler/file-text';
	import Database from '~icons/tabler/database';
	import ChartBar from '~icons/tabler/chart-bar';
	import Clock from '~icons/tabler/clock';
	import { modals } from 'svelte-modals';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { estimateRecordCount } from '$lib/utils/binaryLogParser';

	interface LogFile {
		name: string;
		size: number;
	}

	interface LogMonth {
		name: string;
		path: string;
		files: LogFile[];
	}

	const securityEnabled = page.data.features.security;

	let logs = $state<{ months: LogMonth[] }>({ months: [] });

	// Calculated statistics
	let totalFiles = $derived(logs.months.reduce((sum, month) => sum + month.files.length, 0));
	let totalSizeKB = $derived(
		logs.months.reduce((sum, month) => sum + month.files.reduce((s, f) => s + f.size, 0), 0) / 1024
	);
	let averageSizeKB = $derived(totalFiles > 0 ? totalSizeKB / totalFiles : 0);
	let oldestMonth = $derived(logs.months.length > 0 ? logs.months[0].name : '-');
	let newestMonth = $derived(
		logs.months.length > 0 ? logs.months[logs.months.length - 1].name : '-'
	);
	// Calculate estimated entries: for .bin files use exact formula, for .csv estimate ~100 bytes/entry
	let estimatedEntries = $derived(
		logs.months.reduce((sum, month) => {
			return (
				sum +
				month.files.reduce((s, f) => {
					if (f.name.endsWith('.bin')) {
						return s + estimateRecordCount(f.size);
					} else {
						// Legacy CSV: estimate ~100 bytes per entry
						return s + Math.floor(f.size / 100);
					}
				}, 0)
			);
		}, 0)
	);

	const authHeaders = (): Record<string, string> => {
		if (!securityEnabled) return {};
		return { Authorization: `Bearer ${$user.bearer_token}` };
	};

	const refreshLogs = () =>
		fetch('/api/logs', { headers: authHeaders() })
			.then((res) => res.json())
			.then((data) => {
				logs = data;
			})
			.catch((err) => {
				console.error('Logs fetch error:', err);
			});

	onMount(() => {
		refreshLogs();
	});

	function downloadLog(monthPath: string, filename: string) {
		const fullPath = `${monthPath}/${filename}`;
		const url = `/api/logs/download?file=${encodeURIComponent(fullPath)}`;
		if (!securityEnabled) {
			window.open(url, '_blank');
			return;
		}
		fetch(url, { headers: authHeaders() })
			.then((res) => res.blob())
			.then((blob) => {
				const link = document.createElement('a');
				link.href = URL.createObjectURL(blob);
				link.download = filename;
				link.click();
				URL.revokeObjectURL(link.href);
			});
	}

	function escapeHtml(value: string) {
		return value
			.replaceAll('&', '&amp;')
			.replaceAll('<', '&lt;')
			.replaceAll('>', '&gt;')
			.replaceAll('"', '&quot;')
			.replaceAll("'", '&#039;');
	}

	function confirmDelete(monthPath: string, filename: string) {
		modals.open(ConfirmDialog, {
			title: 'Delete Log File',
			messageHtml: `Are you sure you want to delete <strong>${escapeHtml(filename)}</strong>? This action cannot be undone.`,
			onConfirm: () => deleteLog(monthPath, filename)
		});
	}

	async function deleteLog(monthPath: string, filename: string) {
		const fullPath = `${monthPath}/${filename}`;

		try {
			const res = await fetch(`/api/logs/delete?file=${encodeURIComponent(fullPath)}`, {
				method: 'DELETE',
				headers: authHeaders()
			});

			if (res.ok) {
				refreshLogs();
			} else {
				const errorText = await res.text();
				console.error('[Delete] Failed:', res.status, errorText);
				alert(`Delete failed: ${errorText}`);
			}
		} catch (err) {
			console.error('[Delete] Error:', err);
			alert(`Delete error: ${err}`);
		}
	}
</script>

<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-4 max-[320px]:px-2">
	{#if logs.months.length === 0}
		<div class="grid grid-cols-1 md:grid-cols-2 gap-4">
			<div class="card bg-base-200 shadow-primary/50 shadow-lg">
				<div class="card-body p-4">
					<h2 class="card-title text-lg mb-2"><FileText class="h-6 w-6" /> Data Logs</h2>
					<div class="alert alert-info">
						<span>No logs available yet. Logs are created every 5 minutes.</span>
					</div>
				</div>
			</div>

			<div class="card bg-base-200 shadow-primary/50 shadow-lg">
				<div class="card-body p-4">
					<h2 class="card-title text-lg mb-2"><ChartBar class="h-6 w-6" /> Logging Info</h2>
					<div class="space-y-3">
						<div class="flex justify-between items-center">
							<span class="text-sm opacity-70">Log Interval:</span>
							<span class="font-bold">5 minutes</span>
						</div>
						<div class="flex justify-between items-center">
							<span class="text-sm opacity-70">Data Points per Day:</span>
							<span class="font-bold">~288</span>
						</div>
						<div class="flex justify-between items-center">
							<span class="text-sm opacity-70">Format:</span>
							<span class="font-bold">Binary (16 bytes/record)</span>
						</div>
						<div class="flex justify-between items-center">
							<span class="text-sm opacity-70">Storage:</span>
							<span class="font-bold">Daily .bin Files</span>
						</div>
						<div class="flex justify-between items-center">
							<span class="text-sm opacity-70">Format:</span>
							<span class="font-bold">CSV</span>
						</div>
						<div class="flex justify-between items-center">
							<span class="text-sm opacity-70">Storage:</span>
							<span class="font-bold">Daily Files</span>
						</div>
					</div>
					<div class="mt-4 text-sm opacity-70">
						<p>
							Logs include: temperature, humidity, light, soil moisture, salt, battery voltage and
							percentage.
						</p>
					</div>
				</div>
			</div>
		</div>
	{:else}
		<div class="grid grid-cols-1 md:grid-cols-2 gap-4">
			<!-- Left column: Log files -->
			<div class="space-y-4">
				{#each logs.months as month}
					<div class="card bg-base-200 shadow-primary/50 shadow-lg">
						<div class="card-body p-4">
							<h2 class="card-title text-lg mb-2"><Calendar class="h-6 w-6" /> {month.name}</h2>

							<div class="overflow-x-auto">
								<table class="table table-zebra table-sm">
									<thead>
										<tr>
											<th>Date</th>
											<th>Size</th>
											<th class="text-right">Action</th>
										</tr>
									</thead>
									<tbody>
										{#each month.files as file}
											<tr>
												<td class="text-sm">{file.name}</td>
												<td class="text-sm">{(file.size / 1024).toFixed(1)} KB</td>
												<td class="text-right">
													<div class="flex gap-1 justify-end">
														<button
															class="btn btn-xs btn-primary"
															onclick={() => downloadLog(month.path, file.name)}
															title="Download"
														>
															<Download class="w-3 h-3" />
														</button>
														<button
															class="btn btn-xs btn-error"
															onclick={() => confirmDelete(month.path, file.name)}
															title="Delete"
														>
															<Trash class="w-3 h-3" />
														</button>
													</div>
												</td>
											</tr>
										{/each}
									</tbody>
								</table>
							</div>
						</div>
					</div>
				{/each}
			</div>

			<!-- Right column: Statistics + Period -->
			<div class="space-y-4">
				<div class="card bg-base-200 shadow-primary/50 shadow-lg">
					<div class="card-body p-4">
						<h2 class="card-title text-lg mb-2"><ChartBar class="h-6 w-6" /> Statistics</h2>

						<div class="stats stats-vertical shadow w-full bg-base-300">
							<div class="stat py-2 px-3">
								<div class="stat-figure text-primary">
									<FileText class="w-6 h-6" />
								</div>
								<div class="stat-title text-xs">Total Files</div>
								<div class="stat-value text-lg text-primary">{totalFiles}</div>
							</div>

							<div class="stat py-2 px-3">
								<div class="stat-figure text-secondary">
									<Database class="w-6 h-6" />
								</div>
								<div class="stat-title text-xs">Total Size</div>
								<div class="stat-value text-lg text-secondary">{totalSizeKB.toFixed(1)} KB</div>
								<div class="stat-desc text-xs">{(totalSizeKB / 1024).toFixed(2)} MB</div>
							</div>

							<div class="stat py-2 px-3">
								<div class="stat-figure text-accent">
									<ChartBar class="w-6 h-6" />
								</div>
								<div class="stat-title text-xs">Avg File Size</div>
								<div class="stat-value text-lg text-accent">{averageSizeKB.toFixed(1)} KB</div>
							</div>

							<div class="stat py-2 px-3">
								<div class="stat-figure text-info">
									<FileText class="w-6 h-6" />
								</div>
								<div class="stat-title text-xs">Est. Entries</div>
								<div class="stat-value text-lg text-info">{estimatedEntries.toLocaleString()}</div>
								<div class="stat-desc text-xs">
									~{Math.floor(estimatedEntries / totalFiles)} per file
								</div>
							</div>
						</div>
					</div>
				</div>

				<div class="card bg-base-200 shadow-primary/50 shadow-lg">
					<div class="card-body p-4">
						<h2 class="card-title text-lg mb-2"><Clock class="h-6 w-6" /> Period</h2>

						<div class="space-y-3">
							<div class="flex justify-between items-center">
								<span class="text-sm opacity-70">Oldest Month:</span>
								<span class="font-bold">{oldestMonth}</span>
							</div>
							<div class="flex justify-between items-center">
								<span class="text-sm opacity-70">Newest Month:</span>
								<span class="font-bold">{newestMonth}</span>
							</div>
							<div class="flex justify-between items-center">
								<span class="text-sm opacity-70">Months with Data:</span>
								<span class="font-bold">{logs.months.length}</span>
							</div>
						</div>
					</div>
				</div>
			</div>
		</div>
	{/if}
</div>
