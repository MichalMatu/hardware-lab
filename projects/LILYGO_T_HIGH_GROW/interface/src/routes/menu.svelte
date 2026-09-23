<script lang="ts">
	import Users from '~icons/tabler/users';
	import Settings from '~icons/tabler/settings';
	import Health from '~icons/tabler/stethoscope';
	import Router from '~icons/tabler/router';
	import AP from '~icons/tabler/access-point';
	import Avatar from '~icons/tabler/user-circle';
	import Logout from '~icons/tabler/logout';
	import NTP from '~icons/tabler/clock-check';
	import FileText from '~icons/tabler/file-text';
	import ChartLine from '~icons/tabler/chart-line';
	import Adjustments from '~icons/tabler/adjustments';
	import Power from '~icons/tabler/power';
	import { page } from '$app/state';
	import { user } from '$lib/stores/user';

	let { closeMenu } = $props();

	type menuItem = {
		title: string;
		icon: any;
		href?: string;
		feature: boolean;
		active?: boolean;
		submenu?: subMenuItem[];
	};

	type subMenuItem = {
		title: string;
		icon: any;
		href: string;
		feature: boolean;
		active?: boolean;
	};

	let menuItems = $state([
		{
			title: 'Automation',
			icon: Adjustments,
			feature: true,
			submenu: [
				{
					title: 'Automation',
					icon: Adjustments,
					href: '/automation',
					feature: true
				},
				{
					title: 'Automation Rules',
					icon: Adjustments,
					href: '/automation/rules',
					feature: true
				},
				{
					title: 'Automation Schedules',
					icon: NTP,
					href: '/automation/schedules',
					feature: true
				}
			]
		},
		{
			title: 'Alarms',
			icon: Health,
			feature: true,
			submenu: [
				{
					title: 'Alarms',
					icon: Health,
					href: '/alarms',
					feature: true
				},
				{
					title: 'Alarm Rules',
					icon: Health,
					href: '/alarms/rules',
					feature: true
				},
				{
					title: 'Alarm History',
					icon: FileText,
					href: '/alarms/history',
					feature: true
				}
			]
		},
		{
			title: 'Charts',
			icon: ChartLine,
			href: '/charts',
			feature: true
		},
		{
			title: 'Connectivity',
			icon: Router,
			feature: true,
			submenu: [
				{
					title: 'WiFi Station',
					icon: Router,
					href: '/wifi/sta',
					feature: true
				},
				{
					title: 'Access Point',
					icon: AP,
					href: '/wifi/ap',
					feature: true
				},
				{
					title: 'Time',
					icon: NTP,
					href: '/connections/ntp',
					feature: page.data.features.ntp
				}
			]
		},
		{
			title: 'Settings',
			icon: Settings,
			feature: true,
			submenu: [
				{
					title: 'Sensor Calibration',
					icon: Adjustments,
					href: '/config',
					feature: true
				},
				{
					title: '433MHz',
					icon: Router,
					href: '/settings/integrations/433',
					feature: true
				},
				{
					title: 'Telegram Notifications',
					icon: Users,
					href: '/settings/notifications/telegram',
					feature: true
				},
				{
					title: 'Users',
					icon: Users,
					href: '/user',
					feature: page.data.features.security && $user.admin
				},
				{
					title: 'Power Settings',
					icon: Power,
					href: '/system/power',
					feature: true
				}
			]
		},
		{
			title: 'Diagnostics',
			icon: Health,
			feature: true,
			submenu: [
				{
					title: 'System Status',
					icon: Health,
					href: '/system/status',
					feature: true
				},
				{
					title: 'Data Logs',
					icon: FileText,
					href: '/logs',
					feature: true
				}
			]
		}
	] as menuItem[]);

	function setActiveMenuItem(targetTitle: string) {
		menuItems.forEach((item) => {
			item.active = item.title === targetTitle;
			item.submenu?.forEach((subItem) => {
				subItem.active = subItem.title === targetTitle;
			});
		});
		closeMenu();
	}

	$effect(() => {
		setActiveMenuItem(page.data.title);
	});
</script>

<div class="bg-base-200 text-base-content flex h-full w-[min(20rem,100vw)] flex-col p-4">
	<!-- Sidebar content here -->
	<a href="/" class="rounded-box mb-4 flex items-center" onclick={() => setActiveMenuItem('')}>
		<h1 class="px-4 text-2xl font-bold">PlantWatch</h1>
	</a>
	<ul class="menu w-full rounded-box menu-vertical flex-nowrap overflow-y-auto">
		{#each menuItems as menuItem (menuItem.title)}
			{#if menuItem.feature}
				<li>
					{#if menuItem.submenu}
						<details open={menuItem.submenu.some((subMenuItem) => subMenuItem.active)}>
							<summary class="text-lg font-bold">
								<menuItem.icon class="h-6 w-6" />
								{menuItem.title}
							</summary>
							<ul>
								{#each menuItem.submenu as subMenuItem}
									{#if subMenuItem.feature}
										<li class="hover-bordered">
											<a
												href={subMenuItem.href}
												class:bg-base-100={subMenuItem.active}
												class="text-ml font-bold"
												onclick={() => {
													setActiveMenuItem(subMenuItem.title);
												}}><subMenuItem.icon class="h-5 w-5" />{subMenuItem.title}</a
											>
										</li>
									{/if}
								{/each}
							</ul>
						</details>
					{:else}
						<a
							href={menuItem.href}
							class:bg-base-100={menuItem.active}
							class="text-lg font-bold"
							onclick={() => {
								setActiveMenuItem(menuItem.title);
							}}><menuItem.icon class="h-6 w-6" />{menuItem.title}</a
						>
					{/if}
				</li>
			{/if}
		{/each}
	</ul>

	<div class="flex-col"></div>
	<div class="grow"></div>

	{#if page.data.features.security}
		<div class="flex items-center">
			<Avatar class="h-8 w-8" />
			<span class="grow px-4 text-xl font-bold">{$user.username}</span>
			<!-- svelte-ignore a11y_click_events_have_key_events -->
			<!-- svelte-ignore a11y_no_static_element_interactions -->
			<div
				class="btn btn-ghost"
				onclick={() => {
					user.invalidate();
				}}
			>
				<Logout class="h-8 w-8 rotate-180" />
			</div>
		</div>
	{/if}
</div>
