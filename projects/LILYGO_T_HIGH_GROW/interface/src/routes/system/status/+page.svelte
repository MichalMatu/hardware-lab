<script lang="ts">
	import SystemStatus from './SystemStatus.svelte';
	import LiveTail from './LiveTail.svelte';
</script>

<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-4 max-[320px]:px-2">
	<LiveTail />

	<!-- Original System Status with action buttons -->
	<div class="mt-4">
		<SystemStatus />
	</div>
</div>
