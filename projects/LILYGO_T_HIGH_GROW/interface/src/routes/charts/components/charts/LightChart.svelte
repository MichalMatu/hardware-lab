<script lang="ts">
	import Sun from '~icons/tabler/sun';
	import BaseChart from './BaseChart.svelte';

	export let data: (number | null)[] = [];
	export let timestamps: number[] = [];
</script>

<BaseChart
	{data}
	{timestamps}
	title="Light"
	icon={Sun}
	chartId="luxChart"
	color="#f59e0b"
	unit="lux"
	decimals={0}
	shadowColor="warning"
/>
