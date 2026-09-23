<script lang="ts">
	import InputPassword from '$lib/components/InputPassword.svelte';
	import { user } from '$lib/stores/user';
	import { notifications } from '$lib/components/toasts/notifications';
	import { fade, fly } from 'svelte/transition';
	import Login from '~icons/tabler/login';

	type SignInData = {
		password: string;
		username: string;
	};

	let { signIn } = $props();

	let username = $state('');
	let password = $state('');

	let loginFailed = $state(false);

	let token = { access_token: '' };

	async function signInUser(data: SignInData) {
		try {
			const response = await fetch('/rest/signIn', {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});
			if (response.status === 200) {
				token = await response.json();
				user.init(token.access_token);
				let username = $user.username;
				notifications.success('User ' + username + ' signed in', 5000);
				signIn();
			} else {
				username = '';
				password = '';
				notifications.error('Wrong Username or Password!', 5000);
				loginFailed = true;
				setTimeout(() => {
					loginFailed = false;
				}, 1500);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}
</script>

<div class="hero login-hero from-base-200 to-base-100 min-h-screen bg-linear-to-br">
	<div class="login-hero-grid" aria-hidden="true"></div>
	<div
		class="card login-card bg-base-100 shadow-primary/50 face shadow-lg {loginFailed
			? 'failure border-error border-2'
			: ''}"
		in:fly={{ delay: 200, y: 100, duration: 500 }}
		out:fade={{ duration: 200 }}
	>
		<div class="card-body w-[min(20rem,100vw)]">
			<h2 class="card-title text-2xl">Login</h2>
			<form class="fieldset w-full max-w-xs">
				<label class="label" for="user">Username</label>
				<input
					type="text"
					class="input w-full max-w-xs"
					id="user"
					autocomplete="username"
					bind:value={username}
				/>

				<label class="label" for="pwd">Password </label>
				<InputPassword id="pwd" bind:value={password} />

				<div class="card-actions mt-4 justify-end">
					<button
						class="btn btn-primary inline-flex items-center"
						onclick={() => {
							signInUser({ username: username, password: password });
						}}><Login class="mr-2 h-5 w-5" /><span>Login</span></button
					>
				</div>
			</form>
		</div>
	</div>
</div>

<style>
	.login-hero {
		position: relative;
		isolation: isolate;
		overflow: hidden;
	}

	.login-hero-grid {
		position: absolute;
		inset: 0;
		z-index: 0;
		pointer-events: none;
	}

	.login-hero-grid::before {
		content: '';
		position: absolute;
		inset: 0;
		/* Tech grid */
		background-image:
			linear-gradient(
				color-mix(in oklab, var(--color-base-content) 7%, transparent) 1px,
				transparent 1px
			),
			linear-gradient(
				90deg,
				color-mix(in oklab, var(--color-base-content) 7%, transparent) 1px,
				transparent 1px
			);
		background-size: 28px 28px;
		opacity: 0.14;
	}

	.login-hero-grid::after {
		content: '';
		position: absolute;
		inset: -20%;
		/* Slow moving spotlight */
		background: radial-gradient(
			circle at 30% 25%,
			color-mix(in oklab, var(--color-primary) 16%, var(--color-base-100)),
			transparent 55%
		);
		filter: blur(18px);
		transform: translate3d(0, 0, 0);
		animation: login-spot 14s ease-in-out infinite;
		opacity: 0.9;
	}

	.login-hero::before {
		content: '';
		position: absolute;
		inset: -30vmax;
		z-index: 0;
		background:
			radial-gradient(
				closest-side,
				color-mix(in oklab, var(--color-primary) 26%, var(--color-base-100)),
				transparent 70%
			),
			radial-gradient(
				closest-side,
				color-mix(in oklab, var(--color-secondary) 18%, var(--color-base-100)),
				transparent 70%
			),
			radial-gradient(
				closest-side,
				color-mix(in oklab, var(--color-accent) 16%, var(--color-base-100)),
				transparent 70%
			);
		filter: blur(50px) saturate(115%);
		transform: translate3d(-6%, -4%, 0) rotate(0deg);
		animation: login-blobs 18s ease-in-out infinite;
	}

	.login-hero::after {
		content: '';
		position: absolute;
		inset: 0;
		z-index: 0;
		pointer-events: none;
		background-image:
			repeating-linear-gradient(
				0deg,
				color-mix(in oklab, var(--color-base-content) 10%, transparent) 0 1px,
				transparent 1px 4px
			),
			repeating-linear-gradient(
				90deg,
				color-mix(in oklab, var(--color-base-content) 8%, transparent) 0 1px,
				transparent 1px 5px
			);
		opacity: 0.25;
		mix-blend-mode: overlay;
	}

	@keyframes login-blobs {
		0% {
			transform: translate3d(-6%, -4%, 0) rotate(0deg) scale(1);
		}
		33% {
			transform: translate3d(8%, -2%, 0) rotate(40deg) scale(1.05);
		}
		66% {
			transform: translate3d(2%, 10%, 0) rotate(85deg) scale(0.98);
		}
		100% {
			transform: translate3d(-6%, -4%, 0) rotate(120deg) scale(1);
		}
	}

	@keyframes login-spot {
		0% {
			transform: translate3d(-4%, -2%, 0);
		}
		50% {
			transform: translate3d(6%, 4%, 0);
		}
		100% {
			transform: translate3d(-4%, -2%, 0);
		}
	}

	.login-card {
		position: relative;
		z-index: 1;
		overflow: hidden;
	}

	.login-card::before {
		content: '';
		position: absolute;
		inset: -40%;
		pointer-events: none;
		background: linear-gradient(
			115deg,
			transparent 40%,
			color-mix(in oklab, var(--color-primary) 12%, transparent) 50%,
			transparent 60%
		);
		transform: translateX(-60%) rotate(12deg);
		animation: login-shine 3.6s ease-in-out infinite;
	}

	@keyframes login-shine {
		0% {
			transform: translateX(-60%) rotate(12deg);
			opacity: 0;
		}
		20% {
			opacity: 1;
		}
		50% {
			transform: translateX(60%) rotate(12deg);
			opacity: 0.9;
		}
		100% {
			transform: translateX(60%) rotate(12deg);
			opacity: 0;
		}
	}

	@media (prefers-reduced-motion: reduce) {
		.login-hero::before {
			animation: none;
		}

		.login-hero-grid::after {
			animation: none;
		}

		.login-card::before {
			animation: none;
			opacity: 0;
		}
	}

	.failure {
		animation: shake 0.82s cubic-bezier(0.36, 0.07, 0.19, 0.97) both;
		transform: translate3d(0, 0, 0);
		backface-visibility: hidden;
		perspective: 1000px;
	}
	@keyframes shake {
		10%,
		90% {
			transform: translatex(-1px);
		}

		20%,
		80% {
			transform: translatex(2px);
		}

		30%,
		50%,
		70% {
			transform: translatex(-4px);
		}

		40%,
		60% {
			transform: translatex(4px);
		}
	}
</style>
