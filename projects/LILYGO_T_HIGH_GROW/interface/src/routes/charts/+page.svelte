<script lang="ts">
	import { onMount } from 'svelte';
	import DateSelector from './components/DateSelector.svelte';
	import TemperatureHumidityChart from './components/charts/TemperatureHumidityChart.svelte';
	import SoilSaltChart from './components/charts/SoilSaltChart.svelte';
	import LightChart from './components/charts/LightChart.svelte';
	import BatteryChart from './components/charts/BatteryChart.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { parseBinaryLog, BinaryLogParseError } from '$lib/utils/binaryLogParser';

	type Series = (number | null)[];

	const securityEnabled = page.data.features.security;

	let chartData: {
		timestamps: number[];
		temp: Series;
		humid: Series;
		lux: Series;
		soil: Series;
		salt: Series;
		battery: Series;
	} = { timestamps: [], temp: [], humid: [], lux: [], soil: [], salt: [], battery: [] };
	let availableDates: string[] = [];
	let selectedDate = '';
	let currentMonth = new Date().toISOString().slice(0, 7);
	let showCalendar = false;
	let isLoading = false;

	const authHeaders = (): Record<string, string> => {
		if (!securityEnabled) return {};
		return { Authorization: `Bearer ${$user.bearer_token}` };
	};

	onMount(() => {
		fetchLogs();
	});

	function fetchLogs() {
		fetch('/api/logs', { headers: authHeaders() })
			.then((res) => res.json())
			.then((data) => {
				availableDates = [];
				data.months?.forEach((m: any) => {
					m.files?.forEach((f: any) => {
						if (f.name.endsWith('.bin')) {
							availableDates.push(f.name.replace('.bin', ''));
						}
					});
				});

				availableDates.sort();

				if (availableDates.length > 0) {
					const latest = availableDates[availableDates.length - 1];
					selectedDate = latest;
					currentMonth = latest.slice(0, 7);
				} else {
					// No historical logs, fetch today's data
					fetchData();
				}
			});
	}

	async function fetchData(date?: string) {
		isLoading = true;

		if (!date) {
			// Today's data - fetch binary from /api/charts
			try {
				const res = await fetch('/api/charts', { headers: authHeaders() });
				if (!res.ok) {
					throw new Error(`HTTP ${res.status}: ${res.statusText}`);
				}
				const arrayBuffer = await res.arrayBuffer();
				const parsed = parseBinaryLog(arrayBuffer);

				chartData = {
					timestamps: parsed.timestamps,
					temp: parsed.temps,
					humid: parsed.humids,
					lux: parsed.lux,
					soil: parsed.soils,
					salt: parsed.salts,
					battery: parsed.batteries
				};
			} catch (err) {
				console.error('Failed to fetch/parse binary chart data:', err);
				if (err instanceof BinaryLogParseError) {
					alert(`Binary format error: ${err.message}`);
				}
				chartData = {
					timestamps: [],
					temp: [],
					humid: [],
					lux: [],
					soil: [],
					salt: [],
					battery: []
				};
			} finally {
				isLoading = false;
			}
			return;
		}

		// Historical data - binary format only
		const [year, month] = date.split('-');
		const binFile = `/data/${year}-${month}/${date}.bin`;

		try {
			const res = await fetch(`/api/logs/download?file=${encodeURIComponent(binFile)}`, {
				headers: authHeaders()
			});

			if (!res.ok) {
				throw new Error(`HTTP ${res.status}: ${res.statusText}`);
			}

			const arrayBuffer = await res.arrayBuffer();
			const parsed = parseBinaryLog(arrayBuffer);

			chartData = {
				timestamps: parsed.timestamps,
				temp: parsed.temps,
				humid: parsed.humids,
				lux: parsed.lux,
				soil: parsed.soils,
				salt: parsed.salts,
				battery: parsed.batteries
			};
		} catch (err) {
			console.error('Failed to fetch historical data:', err);
			if (err instanceof BinaryLogParseError) {
				alert(`Binary format error: ${err.message}`);
			}
			chartData = { timestamps: [], temp: [], humid: [], lux: [], soil: [], salt: [], battery: [] };
		} finally {
			isLoading = false;
		}
	}

	$: if (selectedDate) {
		fetchData(selectedDate);
	}
</script>

<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-4 max-[320px]:px-2">
	<DateSelector bind:selectedDate bind:currentMonth bind:availableDates bind:showCalendar />

	{#if isLoading}
		<div class="alert alert-info mb-2">Loading charts...</div>
	{/if}

	{#if chartData.timestamps.length === 0}
		<div class="alert alert-info">
			<span>No data available yet. Charts will appear after first log entry (5 minutes).</span>
		</div>
	{:else}
		<div class="grid grid-cols-1 gap-2 max-[320px]:gap-1 lg:grid-cols-2">
			<div class="lg:col-span-2">
				<TemperatureHumidityChart
					tempData={chartData.temp}
					humidData={chartData.humid}
					timestamps={chartData.timestamps}
				/>
			</div>
			<div class="lg:col-span-2">
				<SoilSaltChart
					soilData={chartData.soil}
					saltData={chartData.salt}
					timestamps={chartData.timestamps}
				/>
			</div>
			<div class="lg:col-span-2">
				<LightChart data={chartData.lux} timestamps={chartData.timestamps} />
			</div>
			<div class="lg:col-span-2">
				<BatteryChart data={chartData.battery} timestamps={chartData.timestamps} />
			</div>
		</div>
	{/if}
</div>
