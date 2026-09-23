<script lang="ts">
	import PlannedApiCard from '$lib/components/PlannedApiCard.svelte';
	import { plannedApi } from '$lib/contracts/plannedApi';
</script>

<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-4 max-[320px]:px-2">
	<h2 class="text-2xl font-bold">Automation</h2>
	<p class="mt-2 opacity-80 leading-relaxed">
		Planned: define automations that run locally on the ESP32 (rules and schedules) and control
		actuators via integrations (e.g. 433MHz).
	</p>
	<ul class="list-disc ml-5 mt-3 text-sm opacity-80 leading-relaxed">
		<li>
			<b>Action targets</b>: pick which actuator to control (configured under Integrations) and what
			action to send (on/off, duration, etc.).
		</li>
		<li>
			<b>Execution model</b>: rules should be non-blocking and safe in low-power modes (wake →
			evaluate → act → sleep).
		</li>
		<li>
			<b>Debounce/hysteresis</b>: avoid flapping behavior (especially for soil moisture thresholds).
		</li>
		<li>
			<b>Conflict handling</b>: define what happens when multiple rules target the same actuator
			(priority or “last write wins”).
		</li>
		<li>
			<b>Audit trail</b>: log automation executions as events (useful for diagnostics and future
			UI).
		</li>
	</ul>

	<div class="mt-6">
		<PlannedApiCard section={plannedApi.automation} />
	</div>
</div>
