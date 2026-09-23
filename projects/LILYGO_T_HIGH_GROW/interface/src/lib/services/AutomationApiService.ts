import { apiAuthHeaders } from '$lib/services/apiAuthHeaders';
import type {
	AutomationRulesConfig,
	AutomationSchedulesConfig
} from '$lib/types/automation/automation';

export class AutomationApiService {
	static async getRules(
		securityEnabled: boolean,
		bearerToken: string
	): Promise<AutomationRulesConfig> {
		const res = await fetch('/api/automation/rules', {
			method: 'GET',
			headers: apiAuthHeaders(securityEnabled, bearerToken)
		});
		if (!res.ok) throw new Error(`GET /api/automation/rules failed: ${res.status}`);
		return res.json();
	}

	static async putRules(
		securityEnabled: boolean,
		bearerToken: string,
		payload: AutomationRulesConfig
	): Promise<AutomationRulesConfig> {
		const res = await fetch('/api/automation/rules', {
			method: 'PUT',
			headers: {
				...apiAuthHeaders(securityEnabled, bearerToken),
				'Content-Type': 'application/json'
			},
			body: JSON.stringify(payload)
		});
		if (!res.ok) throw new Error(`PUT /api/automation/rules failed: ${res.status}`);
		return res.json();
	}

	static async getSchedules(
		securityEnabled: boolean,
		bearerToken: string
	): Promise<AutomationSchedulesConfig> {
		const res = await fetch('/api/automation/schedules', {
			method: 'GET',
			headers: apiAuthHeaders(securityEnabled, bearerToken)
		});
		if (!res.ok) throw new Error(`GET /api/automation/schedules failed: ${res.status}`);
		return res.json();
	}

	static async putSchedules(
		securityEnabled: boolean,
		bearerToken: string,
		payload: AutomationSchedulesConfig
	): Promise<AutomationSchedulesConfig> {
		const res = await fetch('/api/automation/schedules', {
			method: 'PUT',
			headers: {
				...apiAuthHeaders(securityEnabled, bearerToken),
				'Content-Type': 'application/json'
			},
			body: JSON.stringify(payload)
		});
		if (!res.ok) throw new Error(`PUT /api/automation/schedules failed: ${res.status}`);
		return res.json();
	}

	static async runNow(
		securityEnabled: boolean,
		bearerToken: string,
		id: string
	): Promise<{ ok: boolean; error?: string }> {
		const res = await fetch('/api/automation/run', {
			method: 'POST',
			headers: {
				...apiAuthHeaders(securityEnabled, bearerToken),
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ id })
		});
		if (!res.ok) throw new Error(`POST /api/automation/run failed: ${res.status}`);
		return res.json();
	}
}
