<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import uPlot from 'uplot';
	import 'uplot/dist/uPlot.min.css';
	import type { Component } from 'svelte';
	import ChartCard from '../ChartCard.svelte';
	import {
		commonCursorConfig,
		createSingleSeriesConfig,
		createSingleAxesConfig,
		formatTimeOnlyTick
	} from '$lib/utils/chartHelpers';

	interface Props {
		data: (number | null)[];
		timestamps: number[];
		title: string;
		icon: Component;
		chartId: string;
		color: string;
		unit: string;
		decimals?: number;
		shadowColor?: 'warning' | 'primary';
		yRange?: [number, number] | ((_u: any, _min: number, _max: number) => [number, number]);
	}

	let {
		data,
		timestamps,
		title,
		icon,
		chartId,
		color,
		unit,
		decimals = 1,
		shadowColor = 'primary',
		yRange
	}: Props = $props();

	let chart: uPlot | undefined;
	let chartViewport: HTMLElement;
	let chartMount: HTMLElement;
	let lastHeight = 250;
	const SCROLL_TRIGGER_WIDTH = 420;
	const SCROLL_MIN_PLOT_WIDTH = 520;
	let shouldScroll = $state(false);
	let compact = false;
	let ultraCompact = false;

	onMount(() => {
		renderChart();
		setupResizeObserver();
	});

	onDestroy(() => {
		cleanup();
	});

	function renderChart() {
		if (!chartViewport || !chartMount || data.length === 0 || timestamps.length === 0) return;

		if (chart) chart.destroy();

		const plotData = [timestamps, data] as uPlot.AlignedData;

		const viewportWidth = chartViewport.clientWidth;
		compact = viewportWidth < 360;
		ultraCompact = viewportWidth < 320;
		shouldScroll = viewportWidth < SCROLL_TRIGGER_WIDTH;
		const plotWidth = shouldScroll ? Math.max(SCROLL_MIN_PLOT_WIDTH, viewportWidth) : viewportWidth;
		chartMount.style.width = `${plotWidth}px`;
		lastHeight = ultraCompact ? 190 : compact ? 210 : 250;
		const padding: [number, number, number, number] = ultraCompact
			? [8, 6, 0, 0]
			: compact
				? [10, 8, 0, 0]
				: shouldScroll
					? [12, 10, 0, 0]
					: [16, 12, 0, 0];

		const opts: uPlot.Options = {
			width: plotWidth,
			height: lastHeight,
			padding,
			cursor: commonCursorConfig,
			scales: {
				x: { time: true },
				y: yRange ? { auto: false, range: yRange } : { auto: true }
			},
			series: createSingleSeriesConfig(title, color, unit, decimals),
			axes: createSingleAxesConfig(unit, decimals, undefined, { compact, tiny: shouldScroll }),
			legend: { show: false }
		};

		// Encourage fewer x-axis labels on very small widths.
		if (opts.axes && opts.axes[0]) {
			opts.axes[0].space = compact ? 90 : shouldScroll ? 70 : 50;
			opts.axes[0].values = (_u, vals) => vals.map(formatTimeOnlyTick);
		}

		chart = new uPlot(opts, plotData, chartMount);
	}

	$effect(() => {
		if (data.length > 0 && timestamps.length > 0) {
			renderChart();
		}
	});

	let resizeObserver: ResizeObserver | undefined;
	function setupResizeObserver() {
		if (chartViewport && typeof ResizeObserver !== 'undefined') {
			resizeObserver = new ResizeObserver(() => {
				if (chart && chartViewport && chartMount) {
					const viewportWidth = chartViewport.clientWidth;
					compact = viewportWidth < 360;
					ultraCompact = viewportWidth < 320;
					shouldScroll = viewportWidth < SCROLL_TRIGGER_WIDTH;
					const plotWidth = shouldScroll
						? Math.max(SCROLL_MIN_PLOT_WIDTH, viewportWidth)
						: viewportWidth;
					chartMount.style.width = `${plotWidth}px`;
					const height = ultraCompact ? 190 : compact ? 210 : 250;
					chart.setSize({ width: plotWidth, height });
				}
			});
			resizeObserver.observe(chartViewport);
		}
	}

	function cleanup() {
		if (chart) chart.destroy();
		if (resizeObserver) resizeObserver.disconnect();
	}
</script>

<ChartCard {title} {icon} {chartId} {shadowColor}>
	<div
		bind:this={chartViewport}
		class="relative w-full overflow-x-auto overscroll-x-contain snap-x snap-mandatory uplot-scroll"
		class:uplot-scroll--active={shouldScroll}
	>
		<div bind:this={chartMount} class="snap-start"></div>
		{#if shouldScroll}
			<div
				class="pointer-events-none absolute inset-y-0 right-0 w-8 bg-gradient-to-l from-base-200 to-transparent"
			></div>
		{/if}
	</div>
</ChartCard>
