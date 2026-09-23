<script lang="ts">
	import { onMount } from 'svelte';
	import { telemetry } from '$lib/stores/telemetry';
	import Temperature from '~icons/tabler/temperature';
	import Droplet from '~icons/tabler/droplet';
	import Sun from '~icons/tabler/sun';
	import Leaf from '~icons/tabler/leaf';
	import Grain from '~icons/tabler/grain';
	import BatteryIndicator from '$lib/components/BatteryIndicator.svelte';
	import CloudFog from '~icons/tabler/cloud-fog';
	import Sparkline from '$lib/components/Sparkline.svelte';
	import { parseBinaryLog } from '$lib/utils/binaryLogParser';

	let sensorData = $state({
		time: '',
		lux: 0,
		temp: 0,
		humid: 0,
		soil: 0,
		salt: 0,
		batPerc: 0,
		batVolt: 0,
		pressure: 0
	});

	const MAX_HISTORY = 48; // Last ~48 points from today's data
	let history = $state({
		temp: [] as number[],
		humid: [] as number[],
		lux: [] as number[],
		soil: [] as number[],
		salt: [] as number[],
		batPerc: [] as number[],
		pressure: [] as number[]
	});

	onMount(() => {
		const fetchCurrentData = () => {
			fetch('/api/sensors')
				.then((res) => res.json())
				.then((d) => {
					sensorData = d;
					telemetry.setBattery({ soc: d.batPerc, charging: false });
				})
				.catch((err) => console.error('Sensor fetch error:', err));
		};

		const fetchChartData = async () => {
			try {
				const res = await fetch('/api/charts');
				if (!res.ok) return;

				const arrayBuffer = await res.arrayBuffer();
				const parsed = parseBinaryLog(arrayBuffer);

				const toFiniteTail = (values: (number | null)[]): number[] => {
					return values
						.slice(-MAX_HISTORY)
						.filter((v): v is number => v !== null && Number.isFinite(v));
				};

				// Take last MAX_HISTORY points from today's data
				history.temp = toFiniteTail(parsed.temps);
				history.humid = toFiniteTail(parsed.humids);
				history.lux = toFiniteTail(parsed.lux);
				history.soil = toFiniteTail(parsed.soils);
				history.salt = toFiniteTail(parsed.salts);
				history.batPerc = toFiniteTail(parsed.batteries);
				// Note: pressure not in charts API currently
			} catch (err) {
				console.error('Chart data fetch error:', err);
			}
		};

		fetchCurrentData();
		fetchChartData();

		const currentInterval = setInterval(fetchCurrentData, 5000);
		const chartInterval = setInterval(fetchChartData, 60000); // Refresh charts every minute

		return () => {
			clearInterval(currentInterval);
			clearInterval(chartInterval);
		};
	});
</script>

<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-3 max-[320px]:px-2 sm:p-4 md:p-6">
	<div class="grid grid-cols-1 gap-3 max-[320px]:gap-1 sm:gap-4 md:grid-cols-2 xl:grid-cols-3">
		<div class="card bg-base-200 card-shadow-base card-shadow-error">
			<div class="card-body p-4 sm:p-5">
				<h2 class="card-title text-sm sm:text-base mb-2 flex items-center gap-2">
					<Temperature class="w-5 h-5 flex-shrink-0" />
					<span>Temperature</span>
				</h2>
				<div
					class="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-3 min-w-0"
				>
					<p class="text-4xl sm:text-5xl font-bold tabular-nums flex-shrink-0">
						{sensorData.temp != null ? sensorData.temp.toFixed(1) : '--'}°C
					</p>
					{#if history.temp.length > 1}
						<div
							class="w-full sm:w-auto sm:max-w-[160px] min-w-0 opacity-50 hover:opacity-80 transition-opacity"
						>
							<Sparkline
								data={history.temp}
								width={160}
								height={60}
								color="#f87171"
								strokeWidth={2.5}
							/>
						</div>
					{/if}
				</div>
				<progress
					class="progress progress-error mt-2"
					value={Math.min(Math.max(sensorData.temp, 0), 50)}
					max="50"
				></progress>
			</div>
		</div>

		<div class="card bg-base-200 card-shadow-base card-shadow-info">
			<div class="card-body p-4 sm:p-5">
				<h2 class="card-title text-sm sm:text-base mb-2 flex items-center gap-2">
					<Droplet class="w-5 h-5 flex-shrink-0" />
					<span>Humidity</span>
				</h2>
				<div
					class="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-3 min-w-0"
				>
					<p class="text-4xl sm:text-5xl font-bold tabular-nums flex-shrink-0">
						{sensorData.humid != null ? sensorData.humid.toFixed(1) : '--'}%
					</p>
					{#if history.humid.length > 1}
						<div
							class="w-full sm:w-auto sm:max-w-[160px] min-w-0 opacity-50 hover:opacity-80 transition-opacity"
						>
							<Sparkline
								data={history.humid}
								width={160}
								height={60}
								color="#38bdf8"
								strokeWidth={2.5}
								domainMin={0}
								domainMax={100}
							/>
						</div>
					{/if}
				</div>
				<progress class="progress progress-info mt-2" value={sensorData.humid} max="100"></progress>
			</div>
		</div>

		<div class="card bg-base-200 card-shadow-base card-shadow-warning">
			<div class="card-body p-4 sm:p-5">
				<h2 class="card-title text-sm sm:text-base mb-2 flex items-center gap-2">
					<Sun class="w-5 h-5 flex-shrink-0" />
					<span>Light</span>
				</h2>
				<div
					class="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-3 min-w-0"
				>
					<p class="text-4xl sm:text-5xl font-bold tabular-nums flex-shrink-0">
						{sensorData.lux != null ? sensorData.lux.toFixed(0) : '--'}
						<span class="text-2xl font-normal ml-1">lux</span>
					</p>
					{#if history.lux.length > 1}
						<div
							class="w-full sm:w-auto sm:max-w-[160px] min-w-0 opacity-50 hover:opacity-80 transition-opacity"
						>
							<Sparkline
								data={history.lux}
								width={160}
								height={60}
								color="#facc15"
								strokeWidth={2.5}
							/>
						</div>
					{/if}
				</div>
				<progress
					class="progress progress-warning mt-2"
					value={Math.min(sensorData.lux, 2000)}
					max="2000"
				></progress>
			</div>
		</div>

		<div class="card bg-base-200 card-shadow-base card-shadow-success">
			<div class="card-body p-4 sm:p-5">
				<h2 class="card-title text-sm sm:text-base mb-2 flex items-center gap-2">
					<Leaf class="w-5 h-5 flex-shrink-0" />
					<span>Soil Moisture</span>
				</h2>
				<div
					class="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-3 min-w-0"
				>
					<p class="text-4xl sm:text-5xl font-bold tabular-nums flex-shrink-0">
						{sensorData.soil}%
					</p>
					{#if history.soil.length > 1}
						<div
							class="w-full sm:w-auto sm:max-w-[160px] min-w-0 opacity-50 hover:opacity-80 transition-opacity"
						>
							<Sparkline
								data={history.soil}
								width={160}
								height={60}
								color="#4ade80"
								strokeWidth={2.5}
								domainMin={0}
								domainMax={100}
							/>
						</div>
					{/if}
				</div>
				<progress class="progress progress-success mt-2" value={sensorData.soil} max="100"
				></progress>
			</div>
		</div>

		<div class="card bg-base-200 card-shadow-base card-shadow-accent">
			<div class="card-body p-4 sm:p-5">
				<h2 class="card-title text-sm sm:text-base mb-2 flex items-center gap-2">
					<Grain class="w-5 h-5 flex-shrink-0" />
					<span>Salt</span>
				</h2>
				<div
					class="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-3 min-w-0"
				>
					<p class="text-4xl sm:text-5xl font-bold tabular-nums flex-shrink-0">{sensorData.salt}</p>
					{#if history.salt.length > 1}
						<div
							class="w-full sm:w-auto sm:max-w-[160px] min-w-0 opacity-50 hover:opacity-80 transition-opacity"
						>
							<Sparkline
								data={history.salt}
								width={160}
								height={60}
								color="#c084fc"
								strokeWidth={2.5}
							/>
						</div>
					{/if}
				</div>
				<progress
					class="progress progress-accent mt-2"
					value={Math.min(sensorData.salt, 2000)}
					max="2000"
				></progress>
			</div>
		</div>

		<div class="card bg-base-200 card-shadow-base card-shadow-primary">
			<div class="card-body p-4 sm:p-5">
				<div class="flex justify-between items-start mb-2">
					<h2 class="card-title text-sm sm:text-base flex items-center gap-2">
						<BatteryIndicator soc={sensorData.batPerc} charging={false} />
						<span>Battery</span>
					</h2>
					<span class="text-xs sm:text-sm font-semibold badge badge-ghost"
						>{sensorData.batVolt != null ? sensorData.batVolt.toFixed(2) : '--'}V</span
					>
				</div>
				<div
					class="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-3 min-w-0"
				>
					<p class="text-4xl sm:text-5xl font-bold tabular-nums flex-shrink-0">
						{sensorData.batPerc}%
					</p>
					{#if history.batPerc.length > 1}
						<div
							class="w-full sm:w-auto sm:max-w-[160px] min-w-0 opacity-50 hover:opacity-80 transition-opacity"
						>
							<Sparkline
								data={history.batPerc}
								width={160}
								height={60}
								color="#60a5fa"
								strokeWidth={2.5}
								domainMin={0}
								domainMax={100}
							/>
						</div>
					{/if}
				</div>
				<progress class="progress progress-primary mt-2" value={sensorData.batPerc} max="100"
				></progress>
			</div>
		</div>

		{#if sensorData.pressure > 0}
			<div class="card bg-base-200 card-shadow-base card-shadow-neutral">
				<div class="card-body p-4 sm:p-5">
					<h2 class="card-title text-sm sm:text-base mb-2 flex items-center gap-2">
						<CloudFog class="w-5 h-5 flex-shrink-0" />
						<span>Pressure</span>
					</h2>
					<div
						class="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-3 min-w-0"
					>
						<p class="text-4xl sm:text-5xl font-bold tabular-nums flex-shrink-0">
							{sensorData.pressure != null ? sensorData.pressure.toFixed(0) : '--'}
							<span class="text-2xl font-normal ml-1">hPa</span>
						</p>
						{#if history.pressure.length > 1}
							<div
								class="w-full sm:w-auto sm:max-w-[160px] min-w-0 opacity-50 hover:opacity-80 transition-opacity"
							>
								<Sparkline
									data={history.pressure}
									width={160}
									height={60}
									color="#94a3b8"
									strokeWidth={2.5}
								/>
							</div>
						{/if}
					</div>
				</div>
			</div>
		{/if}
	</div>
</div>
