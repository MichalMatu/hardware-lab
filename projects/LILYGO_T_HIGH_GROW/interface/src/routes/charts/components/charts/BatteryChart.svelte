<script lang="ts">
	import Battery from '~icons/tabler/battery';
	import BaseChart from './BaseChart.svelte';

	export let data: (number | null)[] = [];
	export let timestamps: number[] = [];
</script>

<BaseChart
	{data}
	{timestamps}
	title="Battery"
	icon={Battery}
	chartId="batteryChart"
	color="#8b5cf6"
	unit="%"
	decimals={0}
	yRange={[0, 100]}
	shadowColor="primary"
/>
