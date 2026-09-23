<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import uPlot from 'uplot';
	import 'uplot/dist/uPlot.min.css';
	import Temperature from '~icons/tabler/temperature';
	import Droplet from '~icons/tabler/droplet';
	import { formatTimeOnlyTick } from '$lib/utils/chartHelpers';

	export let tempData: (number | null)[] = [];
	export let humidData: (number | null)[] = [];
	export let timestamps: number[] = [];

	let chart: uPlot | undefined;
	let chartViewport: HTMLElement;
	let chartMount: HTMLElement;
	const chartId = 'tempHumidChart';
	const SCROLL_TRIGGER_WIDTH = 420;
	const SCROLL_MIN_PLOT_WIDTH = 520;
	let shouldScroll = false;

	onMount(() => {
		renderChart();
	});

	onDestroy(() => {
		if (chart) chart.destroy();
	});

	function renderChart() {
		if (!chartViewport || !chartMount || tempData.length === 0 || timestamps.length === 0) return;

		if (chart) chart.destroy();

		// uPlot expects data as [timestamps, series1, series2, ...]
		const data = [timestamps, tempData, humidData] as uPlot.AlignedData;

		const viewportWidth = chartViewport.clientWidth;
		const compact = viewportWidth < 360;
		const ultraCompact = viewportWidth < 320;
		shouldScroll = viewportWidth < SCROLL_TRIGGER_WIDTH;
		const plotWidth = shouldScroll ? Math.max(SCROLL_MIN_PLOT_WIDTH, viewportWidth) : viewportWidth;
		chartMount.style.width = `${plotWidth}px`;

		let height = 280;
		let padding: [number, number, number, number] = [14, 14, 0, 0];
		let axisSize = 46;
		let tickSize = 6;
		let axisGap = 5;

		if (ultraCompact) {
			height = 210;
			padding = [8, 8, 0, 0];
			axisSize = 30;
			tickSize = 3;
			axisGap = 3;
		} else if (compact) {
			height = 235;
			padding = [10, 10, 0, 0];
			axisSize = 36;
			axisGap = 4;
		} else if (shouldScroll) {
			padding = [12, 12, 0, 0];
			axisSize = 40;
			axisGap = 4;
		}

		const opts: uPlot.Options = {
			width: plotWidth,
			height,
			padding,
			class: 'chart-temperature-humidity',
			cursor: {
				drag: { x: false, y: false },
				focus: { prox: 16 },
				points: {
					size: (u, seriesIdx) => (seriesIdx > 0 ? 8 : 0),
					width: (u, seriesIdx) => (seriesIdx > 0 ? 2 : 0)
				}
			},
			scales: {
				x: { time: true },
				temp: { auto: true },
				humid: { auto: true, range: [0, 100] }
			},
			series: [
				{ label: 'Time' },
				{
					label: 'Temperature',
					stroke: '#ef4444',
					width: 2.5,
					scale: 'temp',
					points: { show: false },
					value: (u, v) => (v == null ? '-' : v.toFixed(1) + ' °C')
				},
				{
					label: 'Humidity',
					stroke: '#3b82f6',
					width: 2.5,
					scale: 'humid',
					points: { show: false },
					value: (u, v) => (v == null ? '-' : v.toFixed(1) + ' %')
				}
			],
			axes: [
				{
					stroke: '#9ca3af',
					grid: { stroke: '#374151', width: 1, dash: [4, 4] },
					ticks: { stroke: '#4b5563', width: 1, size: tickSize },
					size: axisSize,
					gap: axisGap,
					space: compact ? 90 : shouldScroll ? 70 : 50,
					values: (_u, vals) => vals.map(formatTimeOnlyTick)
				},
				{
					scale: 'temp',
					side: 3,
					stroke: '#ef4444',
					grid: { stroke: '#374151', width: 1, dash: [4, 4] },
					ticks: { stroke: '#4b5563', width: 1, size: tickSize },
					size: axisSize,
					gap: axisGap,
					values: (u, vals) => vals.map((v) => (compact ? v.toFixed(0) : v.toFixed(1)) + '°')
				},
				{
					scale: 'humid',
					side: 1,
					stroke: '#3b82f6',
					grid: { show: false },
					ticks: { stroke: '#4b5563', width: 1, size: tickSize },
					size: axisSize,
					gap: axisGap,
					values: (u, vals) => vals.map((v) => v.toFixed(0) + '%')
				}
			],
			legend: { show: false }
		};

		chart = new uPlot(opts, data, chartMount);
	}

	// Reactive re-render
	$: if (tempData.length > 0 && humidData.length > 0 && timestamps.length > 0) {
		renderChart();
	}

	// Handle window resize
	let resizeObserver: ResizeObserver | undefined;
	onMount(() => {
		if (chartViewport && typeof ResizeObserver !== 'undefined') {
			resizeObserver = new ResizeObserver(() => {
				if (chart && chartViewport && chartMount) {
					const viewportWidth = chartViewport.clientWidth;
					const compact = viewportWidth < 360;
					const ultraCompact = viewportWidth < 320;
					shouldScroll = viewportWidth < SCROLL_TRIGGER_WIDTH;
					const plotWidth = shouldScroll
						? Math.max(SCROLL_MIN_PLOT_WIDTH, viewportWidth)
						: viewportWidth;
					chartMount.style.width = `${plotWidth}px`;

					const height = ultraCompact ? 210 : compact ? 235 : 280;
					chart.setSize({ width: plotWidth, height });
				}
			});
			resizeObserver.observe(chartViewport);
		}
	});

	onDestroy(() => {
		if (resizeObserver) resizeObserver.disconnect();
	});
</script>

<div class="card bg-base-200 card-shadow-base card-shadow-info">
	<div class="card-body p-2">
		<h2 class="card-title text-sm px-2 flex gap-2 items-center">
			<Temperature class="w-4 h-4 text-red-500" />
			<span class="text-red-500">Temperature</span>
			<span class="text-gray-500">&</span>
			<Droplet class="w-4 h-4 text-blue-500" />
			<span class="text-blue-500">Humidity</span>
		</h2>
		<div
			bind:this={chartViewport}
			class="relative -mt-2 w-full overflow-x-auto overscroll-x-contain snap-x snap-mandatory uplot-scroll"
			class:uplot-scroll--active={shouldScroll}
		>
			<div id={chartId} bind:this={chartMount} class="snap-start"></div>
			{#if shouldScroll}
				<div
					class="pointer-events-none absolute inset-y-0 right-0 w-8 bg-gradient-to-l from-base-200 to-transparent"
				></div>
			{/if}
		</div>
	</div>
</div>
