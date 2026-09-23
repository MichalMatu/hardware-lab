<script lang="ts">
	import type { Component, Snippet } from 'svelte';

	export let title: string;
	export let icon: Component;
	export let chartId: string;
	export let children: Snippet;
	export let shadowColor: 'warning' | 'primary' = 'primary';
</script>

<div class="card bg-base-200 card-shadow-base card-shadow-{shadowColor}">
	<div class="card-body p-2">
		<h2 class="card-title text-sm px-2">
			<svelte:component this={icon} class="w-4 h-4" />
			{title}
		</h2>
		<div id={chartId} class="-mt-2">
			{@render children()}
		</div>
	</div>
</div>
