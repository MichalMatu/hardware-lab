<script lang="ts">
	import PlannedApiCard from '$lib/components/PlannedApiCard.svelte';
	import { plannedApi } from '$lib/contracts/plannedApi';
</script>

<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-4 max-[320px]:px-2">
	<h2 class="text-2xl font-bold">Alarms</h2>
	<p class="mt-2 opacity-80 leading-relaxed">
		Planned: configure alarm rules that run on the ESP32 and send notifications (e.g. Telegram).
	</p>
	<ul class="list-disc ml-5 mt-3 text-sm opacity-80 leading-relaxed">
		<li>
			<b>Rules</b>: thresholds and conditions on sensor readings/system state.
		</li>
		<li>
			<b>Noise handling</b>: hysteresis, debounce, and minimum active duration to avoid false
			positives.
		</li>
		<li>
			<b>Notification routing</b>: map alarm severity/type to channels (Telegram first).
		</li>
		<li>
			<b>Throttling</b>: rate limits, reminder intervals, and “send once until cleared” options.
		</li>
		<li>
			<b>Events</b>: keep a bounded local event history (fixed-size ring buffer) for diagnostics and
			the history view.
		</li>
	</ul>

	<div class="mt-6">
		<PlannedApiCard section={plannedApi.alarms} />
	</div>
</div>
