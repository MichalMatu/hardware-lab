<script lang="ts">
	import PlannedApiCard from '$lib/components/PlannedApiCard.svelte';
	import { plannedApi } from '$lib/contracts/plannedApi';
</script>

<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-4 max-[320px]:px-2">
	<h2 class="text-2xl font-bold">Automation Schedules</h2>
	<p class="mt-2 opacity-80 leading-relaxed">
		Planned: configure time-based automations executed on the ESP32 runtime.
	</p>
	<ul class="list-disc ml-5 mt-3 text-sm opacity-80 leading-relaxed">
		<li>
			<b>Time source</b>: require a reliable clock (RTC/NTP) and define what to do if time is
			unknown.
		</li>
		<li>
			<b>Schedule types</b>: one-shot, daily, weekdays, interval-based (every N minutes/hours).
		</li>
		<li>
			<b>Action</b>: choose actuator + command (on/off/duration).
		</li>
		<li>
			<b>Timezone/DST</b>: define how local time is interpreted (important for “06:00 every day”).
		</li>
		<li>
			<b>Missed executions</b>: behavior after deep sleep or reboot (run immediately, skip, or catch
			up).
		</li>
		<li>
			<b>Enable + test</b>: toggle per schedule and a “run now” action.
		</li>
	</ul>

	<div class="mt-6">
		<PlannedApiCard section={plannedApi.automation} />
	</div>
</div>
