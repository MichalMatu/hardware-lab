<script lang="ts">
	import { goto } from '$app/navigation';
	import { modals } from 'svelte-modals';
	import type { ModalComponent } from 'svelte-modals';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';

	const toModalComponent = (component: unknown) => component as unknown as ModalComponent<any>;
	import { user } from '$lib/stores/user';
	import type { userProfile } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import EditUser from './EditUser.svelte';
	import Spinner from '$lib/components/Spinner.svelte';
	import Delete from '~icons/tabler/trash';
	import AddUser from '~icons/tabler/user-plus';
	import Edit from '~icons/tabler/pencil';
	import Admin from '~icons/tabler/key';
	import Users from '~icons/tabler/users';
	import Warning from '~icons/tabler/alert-triangle';
	import Cancel from '~icons/tabler/x';
	import Check from '~icons/tabler/check';

	type userSetting = {
		username: string;
		password: string;
		admin: boolean;
	};

	type SecuritySettings = {
		jwt_secret: string;
		users: userSetting[];
	};

	let securitySettings: SecuritySettings = $state({
		jwt_secret: '',
		users: []
	});

	async function getSecuritySettings() {
		try {
			const response = await fetch('/rest/securitySettings', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			securitySettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	async function postSecuritySettings(data: SecuritySettings) {
		try {
			const response = await fetch('/rest/securitySettings', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});

			securitySettings = await response.json();
			if (response.status == 200) {
				if (await validateUser($user)) {
					notifications.success('Security settings updated.', 3000);
				}
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	async function validateUser(userdata: userProfile) {
		try {
			const response = await fetch('/rest/verifyAuthorization', {
				method: 'GET',
				headers: {
					Authorization: 'Bearer ' + userdata.bearer_token,
					'Content-Type': 'application/json'
				}
			});
			if (response.status !== 200) {
				user.invalidate();
				return false;
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return true;
	}

	function confirmDelete(index: number) {
		modals.open(toModalComponent(ConfirmDialog), {
			isOpen: true,
			title: 'Confirm Delete User',
			message:
				'Are you sure you want to delete the user "' +
				securitySettings.users[index].username +
				'"?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Yes', icon: Check }
			},
			onConfirm: () => {
				securitySettings.users.splice(index, 1);
				securitySettings = { ...securitySettings, users: [...securitySettings.users] };
				modals.close();
				postSecuritySettings(securitySettings);
			}
		});
	}

	function handleEdit(index: number) {
		modals.open(toModalComponent(EditUser), {
			isOpen: true,
			title: 'Edit User',
			user: { ...securitySettings.users[index] }, // Shallow Copy
			onSaveUser: (editedUser: userSetting) => {
				securitySettings.users[index] = editedUser;
				modals.close();
				postSecuritySettings(securitySettings);
			}
		});
	}

	function handleNewUser() {
		modals.open(toModalComponent(EditUser), {
			isOpen: true,
			title: 'Add User',
			onSaveUser: (newUser: userSetting) => {
				securitySettings.users = [...securitySettings.users, newUser];
				modals.close();
				postSecuritySettings(securitySettings);
			}
		});
		//
	}
</script>

{#if $user.admin}
	<div class="w-full max-w-none 2xl:max-w-7xl mx-auto p-4 max-[320px]:px-2">
		<div class="grid grid-cols-1 md:grid-cols-2 gap-4">
			<!-- Left Column: User Management -->
			<div class="card bg-base-200 shadow-primary/50 shadow-lg">
				<div class="card-body p-4">
					<div class="flex items-center justify-between mb-2">
						<h2 class="card-title text-lg flex items-center">
							<Users class="mr-2 h-6 w-6" />
							<span>Manage Users</span>
						</h2>
						<button class="btn btn-primary btn-sm" onclick={handleNewUser}>
							<AddUser class="h-5 w-5" />
						</button>
					</div>

					{#await getSecuritySettings()}
						<Spinner />
					{:then _result}
						<div
							class="overflow-x-auto"
							transition:slide|local={{ duration: 300, easing: cubicOut }}
						>
							<table class="table w-full table-auto">
								<thead>
									<tr class="font-bold">
										<th align="left">Username</th>
										<th align="center">Admin</th>
										<th align="right" class="pr-2">Edit</th>
									</tr>
								</thead>
								<tbody>
									{#each securitySettings.users as user, index}
										<tr>
											<td align="left">{user.username}</td>
											<td align="center">
												{#if user.admin}
													<Admin class="text-secondary" />
												{/if}
											</td>
											<td align="right">
												<span class="my-auto inline-flex flex-row space-x-2">
													<button
														class="btn btn-ghost btn-circle btn-xs"
														onclick={() => handleEdit(index)}
													>
														<Edit class="h-5 w-5" /></button
													>
													<button
														class="btn btn-ghost btn-circle btn-xs"
														onclick={() => confirmDelete(index)}
													>
														<Delete class="text-error h-5 w-5" />
													</button>
												</span>
											</td>
										</tr>
									{/each}
								</tbody>
							</table>
						</div>
					{/await}
				</div>
			</div>

			<!-- Right Column: Security Settings -->
			<div class="card bg-base-200 shadow-primary/50 shadow-lg">
				<div class="card-body p-4">
					<h2 class="card-title text-lg mb-2">Security Settings</h2>

					{#await getSecuritySettings()}
						<Spinner />
					{:then _result}
						<div class="alert alert-warning shadow-lg mb-4">
							<Warning class="h-5 w-5 shrink-0" />
							<span class="text-sm"
								>The JWT secret is used to sign authentication tokens. If you modify the JWT Secret,
								all users will be signed out.</span
							>
						</div>
						<label class="label" for="secret">JWT Secret</label>
						<InputPassword bind:value={securitySettings.jwt_secret} id="secret" />
						<div class="mt-6 flex justify-end">
							<button
								class="btn btn-primary btn-sm"
								onclick={() => postSecuritySettings(securitySettings)}>Apply Settings</button
							>
						</div>
					{/await}
				</div>
			</div>
		</div>
	</div>
{:else}
	{goto('/')}
{/if}
