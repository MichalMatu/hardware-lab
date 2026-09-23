<script lang="ts">
	import PlannedApiCard from '$lib/components/PlannedApiCard.svelte';
	import { plannedApi } from '$lib/contracts/plannedApi';
</script>

<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-4 max-[320px]:px-2">
	<h2 class="text-2xl font-bold">Automation Rules</h2>
	<p class="mt-2 opacity-80 leading-relaxed">
		Planned: configure condition-based rules executed on the ESP32 runtime.
	</p>
	<ul class="list-disc ml-5 mt-3 text-sm opacity-80 leading-relaxed">
		<li>
			<b>Trigger</b>: choose the input signal (sensor reading, derived value, system state) and the
			condition (e.g. soil moisture below a threshold).
		</li>
		<li>
			<b>Stability</b>: configure hysteresis and/or debounce time so the rule does not oscillate.
		</li>
		<li>
			<b>Action</b>: choose an actuator (433MHz) and command (on/off/duration).
		</li>
		<li>
			<b>Limits</b>: add guardrails (max run time, min off time, daily cap) to protect pumps/relays.
		</li>
		<li>
			<b>Cooldown</b>: minimum time between executions.
		</li>
		<li>
			<b>Enable + test</b>: ability to enable/disable per rule and run a dry-run/test action.
		</li>
	</ul>

	<div class="mt-6">
		<PlannedApiCard section={plannedApi.automation} />
	</div>
</div>
