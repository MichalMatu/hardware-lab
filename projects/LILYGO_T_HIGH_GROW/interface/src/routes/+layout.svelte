<script lang="ts">
	import type { LayoutData } from './$types';
	import { onDestroy, onMount } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { Modals } from 'svelte-modals';
	import Toast from '$lib/components/toasts/Toast.svelte';
	import { fade } from 'svelte/transition';
	import { WebSocketManager } from '$lib/services/WebSocketManager';
	import { AuthService } from '$lib/services/AuthService';
	import { EventHandlers } from '$lib/services/EventHandlers';
	import '../app.css';
	import Menu from './menu.svelte';
	import Statusbar from './statusbar.svelte';
	import Login from './login.svelte';

	interface Props {
		data: LayoutData;
		children?: import('svelte').Snippet;
	}

	let { data: _data, children }: Props = $props();

	const eventHandlers = new EventHandlers();
	const wsManager = new WebSocketManager(_data);

	onMount(async () => {
		if ($user.bearer_token !== '') {
			const isValid = await AuthService.validateUser($user);
			if (!isValid) {
				user.invalidate();
			}
		}
		if (!(page.data.features.security && $user.bearer_token === '')) {
			initSocket();
		}
	});

	onDestroy(() => {
		wsManager.removeEventListeners();
	});

	const initSocket = () => {
		wsManager.init($user.bearer_token, {
			onOpen: eventHandlers.handleOpen,
			onClose: eventHandlers.handleClose,
			onError: eventHandlers.handleError,
			onRSSI: eventHandlers.handleNetworkStatus,
			onNotification: eventHandlers.handleNotification,

			onBattery: eventHandlers.handleBattery
		});
	};

	let menuOpen = $state(false);
</script>

<svelte:head>
	<title>{page.data?.title || 'PlantWatch'}</title>
</svelte:head>

{#if page.data.features.security && $user.bearer_token === ''}
	<Login signIn={initSocket} />
{:else}
	<div class="drawer lg:drawer-open w-full max-w-full overflow-x-hidden">
		<input id="main-menu" type="checkbox" class="drawer-toggle" bind:checked={menuOpen} />
		<div class="drawer-content flex w-full max-w-full flex-col">
			<!-- Status bar content here -->
			<Statusbar />

			<!-- Main page content here -->
			{@render children?.()}
		</div>
		<!-- Side Navigation -->
		<div class="drawer-side z-30 shadow-lg">
			<label for="main-menu" class="drawer-overlay"></label>
			<Menu
				closeMenu={() => {
					menuOpen = false;
				}}
			/>
		</div>
	</div>
{/if}

<Modals>
	<!-- svelte-ignore a11y_click_events_have_key_events -->
	{#snippet backdrop({ close })}
		<div
			class="fixed inset-0 z-40 max-h-full max-w-full bg-black/20 backdrop-blur-sm"
			transition:fade|global
			onclick={() => close()}
			role="button"
			tabindex="0"
			aria-label="Close modal"
		></div>
	{/snippet}
</Modals>

<Toast />
