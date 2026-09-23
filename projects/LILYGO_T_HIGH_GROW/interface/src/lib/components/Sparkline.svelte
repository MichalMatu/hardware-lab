<script lang="ts">
	interface Props {
		data: number[];
		width?: number;
		height?: number;
		color?: string;
		strokeWidth?: number;
		domainMin?: number;
		domainMax?: number;
		padding?: number;
	}

	let {
		data,
		width = 80,
		height = 40,
		color = 'currentColor',
		strokeWidth = 2,
		domainMin,
		domainMax,
		padding
	}: Props = $props();

	const points = $derived(() => {
		if (data.length < 2) return '';
		if (width <= 0 || height <= 0) return '';

		const pad = padding ?? Math.max(1, strokeWidth / 2 + 0.5);
		const innerWidth = Math.max(1, width - 2 * pad);
		const innerHeight = Math.max(1, height - 2 * pad);

		let min = Math.min(...data);
		let max = Math.max(...data);

		if (Number.isFinite(domainMin) && Number.isFinite(domainMax) && domainMax! > domainMin!) {
			min = domainMin!;
			max = domainMax!;
		}

		if (max === min) {
			const delta = Math.max(1, Math.abs(max) * 0.01);
			min = min - delta;
			max = max + delta;
		}

		const range = max - min;
		if (range <= 0 || !Number.isFinite(range)) return '';

		const xStep = innerWidth / (data.length - 1);
		const yScale = innerHeight / range;

		return data
			.map((value, i) => {
				const x = pad + i * xStep;
				let y = pad + (max - value) * yScale;
				y = Math.min(height - pad, Math.max(pad, y));
				return `${x},${y}`;
			})
			.join(' ');
	});
</script>

<svg
	{width}
	{height}
	viewBox="0 0 {width} {height}"
	class="inline-block w-full h-auto max-w-full"
	preserveAspectRatio="xMaxYMid meet"
>
	<polyline
		points={points()}
		fill="none"
		stroke={color}
		stroke-width={strokeWidth}
		stroke-linecap="round"
		stroke-linejoin="round"
		vector-effect="non-scaling-stroke"
	/>
</svg>
