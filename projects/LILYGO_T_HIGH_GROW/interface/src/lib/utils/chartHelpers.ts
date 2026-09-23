/**
 * Shared utilities for uPlot charts
 */

/**
 * Common axis configuration for charts
 */
export const commonAxisStyles = {
	stroke: '#9ca3af',
	grid: { stroke: '#374151', width: 1, dash: [4, 4] as [number, number] },
	ticks: { stroke: '#4b5563', width: 1, size: 6 },
	size: 50,
	gap: 5
};

/**
 * Common cursor configuration
 */
export const commonCursorConfig = {
	drag: { x: false, y: false },
	focus: { prox: 16 },
	points: { size: 8, width: 2 }
};

export function formatTimeOnlyTick(ts: number) {
	const ms = ts < 100_000_000_000 ? ts * 1000 : ts;
	const d = new Date(ms);
	return new Intl.DateTimeFormat('pl-PL', {
		hour: '2-digit',
		minute: '2-digit'
	}).format(d);
}

/**
 * Common series configuration for single line chart
 */
export function createSingleSeriesConfig(
	label: string,
	color: string,
	unit: string,
	decimals: number = 1
) {
	return [
		{ label: 'Time' },
		{
			label,
			stroke: color,
			width: 2.5,
			points: { show: false },
			// eslint-disable-next-line @typescript-eslint/no-explicit-any
			value: (_u: any, v: any) => (v == null ? '-' : v.toFixed(decimals) + ' ' + unit)
		}
	];
}

/**
 * Common axes configuration for single scale chart
 */
export function createSingleAxesConfig(
	unit: string,
	decimals: number = 0,
	// eslint-disable-next-line @typescript-eslint/no-explicit-any
	_range?: [number, number] | ((u: any, min: number, max: number) => [number, number]),
	options?: { compact?: boolean; tiny?: boolean }
) {
	const compact = options?.compact === true;
	const tiny = options?.tiny === true;
	const axisBase = {
		...commonAxisStyles,
		size: tiny ? 32 : compact ? 38 : commonAxisStyles.size,
		gap: tiny ? 3 : compact ? 4 : commonAxisStyles.gap,
		ticks: {
			...commonAxisStyles.ticks,
			size: tiny ? 3 : compact ? 4 : commonAxisStyles.ticks.size
		}
	};

	return [
		{ ...axisBase },
		{
			...axisBase,
			// eslint-disable-next-line @typescript-eslint/no-explicit-any
			values: (_u: any, vals: number[]) =>
				vals.map((v) => {
					const base = v.toFixed(decimals);
					return compact ? base : base + unit;
				})
		}
	];
}
