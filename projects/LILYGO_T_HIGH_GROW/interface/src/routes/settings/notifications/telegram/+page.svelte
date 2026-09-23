<script lang="ts">
	import TelegramSettings from './TelegramSettings.svelte';
	import TelegramTest from './TelegramTest.svelte';
</script>

<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-4 max-[320px]:px-2">
	<div class="grid grid-cols-1 md:grid-cols-2 gap-3 md:gap-4 items-start">
		<TelegramSettings />
		<TelegramTest />
	</div>
</div>
