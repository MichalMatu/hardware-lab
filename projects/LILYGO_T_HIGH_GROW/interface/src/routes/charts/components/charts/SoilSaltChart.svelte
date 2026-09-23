<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import uPlot from 'uplot';
	import 'uplot/dist/uPlot.min.css';
	import Leaf from '~icons/tabler/leaf';
	import Grain from '~icons/tabler/grain';
	import { formatTimeOnlyTick } from '$lib/utils/chartHelpers';

	export let soilData: (number | null)[] = [];
	export let saltData: (number | null)[] = [];
	export let timestamps: number[] = [];

	let chart: uPlot | undefined;
	let chartViewport: HTMLElement;
	let chartMount: HTMLElement;
	const chartId = 'soilSaltChart';
	let lastHeight = 280;
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
		if (!chartViewport || !chartMount || soilData.length === 0 || timestamps.length === 0) return;

		if (chart) chart.destroy();

		// uPlot expects data as [timestamps, series1, series2, ...]
		const data = [timestamps, soilData, saltData] as uPlot.AlignedData;

		const viewportWidth = chartViewport.clientWidth;
		const compact = viewportWidth < 360;
		const ultraCompact = viewportWidth < 320;
		shouldScroll = viewportWidth < SCROLL_TRIGGER_WIDTH;
		const plotWidth = shouldScroll ? Math.max(SCROLL_MIN_PLOT_WIDTH, viewportWidth) : viewportWidth;
		chartMount.style.width = `${plotWidth}px`;
		lastHeight = ultraCompact ? 210 : compact ? 235 : 280;
		const padding: [number, number, number, number] = ultraCompact
			? [8, 8, 0, 0]
			: compact
				? [10, 10, 0, 0]
				: shouldScroll
					? [12, 12, 0, 0]
					: [16, 16, 0, 0];
		const axisSize = ultraCompact ? 30 : compact ? 36 : shouldScroll ? 40 : 50;
		const tickSize = ultraCompact ? 3 : compact ? 4 : 6;
		const axisGap = ultraCompact ? 3 : compact ? 4 : shouldScroll ? 4 : 5;

		const opts: uPlot.Options = {
			width: plotWidth,
			height: lastHeight,
			padding,
			class: 'chart-soil-salt',
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
				soil: { auto: false, range: [0, 100] },
				salt: { auto: true, range: (u, min, max) => [0, Math.max(max, 100)] }
			},
			series: [
				{ label: 'Time' },
				{
					label: 'Soil Moisture',
					stroke: '#10b981',
					width: 2.5,
					scale: 'soil',
					points: { show: false },
					value: (u, v) => (v == null ? '-' : v.toFixed(1) + ' %')
				},
				{
					label: 'Salt',
					stroke: '#c084fc',
					width: 2.5,
					scale: 'salt',
					points: { show: false },
					value: (u, v) => (v == null ? '-' : v.toFixed(0) + ' ppm')
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
					scale: 'soil',
					side: 3,
					stroke: '#10b981',
					grid: { stroke: '#374151', width: 1, dash: [4, 4] },
					ticks: { stroke: '#4b5563', width: 1, size: tickSize },
					size: axisSize,
					gap: axisGap,
					values: (u, vals) => vals.map((v) => v.toFixed(0) + '%')
				},
				{
					scale: 'salt',
					side: 1,
					stroke: '#c084fc',
					grid: { show: false },
					ticks: { stroke: '#4b5563', width: 1, size: tickSize },
					size: axisSize,
					gap: axisGap,
					values: (u, vals) => vals.map((v) => v.toFixed(0))
				}
			],
			legend: { show: false }
		};

		chart = new uPlot(opts, data, chartMount);
	}

	// Reactive re-render
	$: if (soilData.length > 0 && saltData.length > 0 && timestamps.length > 0) {
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

<div class="card bg-base-200 card-shadow-base card-shadow-success">
	<div class="card-body p-2">
		<h2 class="card-title text-sm px-2 flex gap-2 items-center">
			<Leaf class="w-4 h-4 text-green-500" />
			<span class="text-green-500">Soil Moisture</span>
			<span class="text-gray-500">&</span>
			<Grain class="w-4 h-4 text-purple-400" />
			<span class="text-purple-400">Salt</span>
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
