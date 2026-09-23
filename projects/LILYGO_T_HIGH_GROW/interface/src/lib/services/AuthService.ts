import type { userProfile } from '$lib/stores/user';

export class AuthService {
	static async validateUser(userdata: userProfile): Promise<boolean> {
		try {
			const response = await fetch('/rest/verifyAuthorization', {
				method: 'GET',
				headers: {
					Authorization: `Bearer ${userdata.bearer_token}`,
					'Content-Type': 'application/json'
				}
			});
			return response.status === 200;
		} catch (error) {
			console.error('Auth validation error:', error);
			return false;
		}
	}
}
