import { apiAuthHeaders } from '$lib/services/apiAuthHeaders';
import type { Integration433Config } from '$lib/types/integrations/integrations';

export class IntegrationsApiService {
	static async get433(
		securityEnabled: boolean,
		bearerToken: string
	): Promise<Integration433Config> {
		const res = await fetch('/api/integrations/433', {
			method: 'GET',
			headers: apiAuthHeaders(securityEnabled, bearerToken)
		});
		if (!res.ok) throw new Error(`GET /api/integrations/433 failed: ${res.status}`);
		return res.json();
	}

	static async put433(
		securityEnabled: boolean,
		bearerToken: string,
		payload: Integration433Config
	): Promise<Integration433Config> {
		const res = await fetch('/api/integrations/433', {
			method: 'PUT',
			headers: {
				...apiAuthHeaders(securityEnabled, bearerToken),
				'Content-Type': 'application/json'
			},
			body: JSON.stringify(payload)
		});
		if (!res.ok) throw new Error(`PUT /api/integrations/433 failed: ${res.status}`);
		return res.json();
	}

	static async test433(
		securityEnabled: boolean,
		bearerToken: string,
		payload: { profileId: string; command: string }
	): Promise<{ ok: boolean; error?: string }> {
		const res = await fetch('/api/integrations/433/test', {
			method: 'POST',
			headers: {
				...apiAuthHeaders(securityEnabled, bearerToken),
				'Content-Type': 'application/json'
			},
			body: JSON.stringify(payload)
		});
		if (!res.ok) throw new Error(`POST /api/integrations/433/test failed: ${res.status}`);
		return res.json();
	}
}
