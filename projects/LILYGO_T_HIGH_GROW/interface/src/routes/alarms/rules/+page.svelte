<script lang="ts">
	import PlannedApiCard from '$lib/components/PlannedApiCard.svelte';
	import { plannedApi } from '$lib/contracts/plannedApi';
</script>

<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-4 max-[320px]:px-2">
	<h2 class="text-2xl font-bold">Alarm Rules</h2>
	<p class="mt-2 opacity-80 leading-relaxed">
		Planned: configure alarm triggers and how notifications are sent.
	</p>
	<ul class="list-disc ml-5 mt-3 text-sm opacity-80 leading-relaxed">
		<li>
			<b>Trigger</b>: choose signal + condition (e.g. “Soil moisture &lt; X% for Y minutes”).
		</li>
		<li>
			<b>Severity</b>: info/warn/critical (used for routing and throttling).
		</li>
		<li>
			<b>Noise control</b>: hysteresis, debounce, and “clear” criteria.
		</li>
		<li>
			<b>Delivery</b>: select channels/recipients (Telegram config lives in Notifications settings).
		</li>
		<li>
			<b>Anti-spam</b>: cooldown, reminder interval, and max notifications per hour/day.
		</li>
		<li>
			<b>Test</b>: “simulate alarm” / send test notification.
		</li>
	</ul>

	<div class="mt-6">
		<PlannedApiCard section={plannedApi.alarms} />
	</div>
</div>
