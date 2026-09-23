<script lang="ts">
	import type { PlannedApiSection } from '$lib/contracts/plannedApi';

	type Props = {
		section: PlannedApiSection;
	};

	let { section }: Props = $props();
</script>

<div class="card bg-base-200 shadow-primary/50 shadow-lg">
	<div class="card-body p-4">
		<h3 class="card-title text-lg mb-2">{section.title}</h3>
		{#if section.notes?.length}
			<ul class="list-disc ml-5 text-sm opacity-80 leading-relaxed">
				{#each section.notes as note}
					<li>{note}</li>
				{/each}
			</ul>
		{/if}

		<div class="mt-3 overflow-x-auto">
			<table class="table table-sm">
				<thead>
					<tr>
						<th>Method</th>
						<th>Path</th>
						<th>Purpose</th>
						<th>Req</th>
						<th>Resp</th>
						<th>Storage</th>
					</tr>
				</thead>
				<tbody>
					{#each section.endpoints as ep (ep.method + ep.path)}
						<tr>
							<td class="font-mono">{ep.method}</td>
							<td class="font-mono">{ep.path}</td>
							<td>{ep.purpose}</td>
							<td class="font-mono text-xs">{ep.request || '-'}</td>
							<td class="font-mono text-xs">{ep.response || '-'}</td>
							<td class="text-xs">{ep.storage || '-'}</td>
						</tr>
					{/each}
				</tbody>
			</table>
		</div>
	</div>
</div>
