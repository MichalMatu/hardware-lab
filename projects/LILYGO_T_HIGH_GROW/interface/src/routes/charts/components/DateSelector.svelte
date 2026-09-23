<script lang="ts">
	export let selectedDate = '';
	export let currentMonth = '';
	export let availableDates: string[] = [];
	export let showCalendar = false;

	// Extract unique months from available dates
	$: availableMonths = Array.from(new Set(availableDates.map((date) => date.slice(0, 7)))).sort();

	// Check if we can navigate to prev/next month
	$: currentMonthIndex = availableMonths.indexOf(currentMonth);
	$: canGoPrev = currentMonthIndex > 0;
	$: canGoNext = currentMonthIndex < availableMonths.length - 1 && currentMonthIndex >= 0;

	// Auto-select newest month with data when available dates change
	$: if (availableMonths.length > 0 && !availableMonths.includes(currentMonth)) {
		currentMonth = availableMonths[availableMonths.length - 1];
	}

	function changeMonth(delta: number) {
		if (delta < 0 && !canGoPrev) return;
		if (delta > 0 && !canGoNext) return;

		const newIndex = currentMonthIndex + delta;
		if (newIndex >= 0 && newIndex < availableMonths.length) {
			currentMonth = availableMonths[newIndex];
		}
	}

	function getDaysInMonth(yearMonth: string) {
		const [year, month] = yearMonth.split('-').map(Number);
		return new Date(year, month, 0).getDate();
	}

	function handleDateSelect(dateStr: string) {
		selectedDate = dateStr;
		showCalendar = false;
	}
</script>

<div class="mb-4">
	<div class="flex items-center gap-2">
		<button class="btn btn-sm" disabled={!canGoPrev} on:click={() => changeMonth(-1)}>◀</button>
		<button class="btn btn-sm flex-1" on:click={() => (showCalendar = !showCalendar)}>
			{selectedDate || currentMonth}
			{#if availableMonths.length > 0}
				<span class="text-xs opacity-60 ml-1">
					({currentMonthIndex + 1}/{availableMonths.length})
				</span>
			{/if}
		</button>
		<button class="btn btn-sm" disabled={!canGoNext} on:click={() => changeMonth(1)}>▶</button>
	</div>
	{#if showCalendar}
		<div class="mt-2 p-2 bg-base-200 rounded">
			<div class="grid grid-cols-7 gap-1">
				{#each Array(getDaysInMonth(currentMonth)) as _, i}
					{@const day = i + 1}
					{@const dateStr = `${currentMonth}-${String(day).padStart(2, '0')}`}
					{@const hasData = availableDates.includes(dateStr)}
					<button
						class="btn btn-xs {selectedDate === dateStr
							? 'btn-primary'
							: hasData
								? 'btn-outline'
								: 'btn-ghost'}"
						class:font-bold={hasData}
						disabled={!hasData}
						on:click={() => handleDateSelect(dateStr)}
					>
						{day}
					</button>
				{/each}
			</div>
		</div>
	{/if}
</div>
