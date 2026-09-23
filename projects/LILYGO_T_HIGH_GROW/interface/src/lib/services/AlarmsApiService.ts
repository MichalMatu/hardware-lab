import { apiAuthHeaders } from '$lib/services/apiAuthHeaders';
import type { AlarmEventsPage, AlarmRulesConfig } from '$lib/types/alarms/alarms';

export class AlarmsApiService {
	static async getRules(securityEnabled: boolean, bearerToken: string): Promise<AlarmRulesConfig> {
		const res = await fetch('/api/alarms/rules', {
			method: 'GET',
			headers: apiAuthHeaders(securityEnabled, bearerToken)
		});
		if (!res.ok) throw new Error(`GET /api/alarms/rules failed: ${res.status}`);
		return res.json();
	}

	static async putRules(
		securityEnabled: boolean,
		bearerToken: string,
		payload: AlarmRulesConfig
	): Promise<AlarmRulesConfig> {
		const res = await fetch('/api/alarms/rules', {
			method: 'PUT',
			headers: {
				...apiAuthHeaders(securityEnabled, bearerToken),
				'Content-Type': 'application/json'
			},
			body: JSON.stringify(payload)
		});
		if (!res.ok) throw new Error(`PUT /api/alarms/rules failed: ${res.status}`);
		return res.json();
	}

	static async getEvents(
		securityEnabled: boolean,
		bearerToken: string,
		cursor?: string
	): Promise<AlarmEventsPage> {
		const url = cursor
			? `/api/alarms/events?cursor=${encodeURIComponent(cursor)}`
			: '/api/alarms/events';
		const res = await fetch(url, {
			method: 'GET',
			headers: apiAuthHeaders(securityEnabled, bearerToken)
		});
		if (!res.ok) throw new Error(`GET /api/alarms/events failed: ${res.status}`);
		return res.json();
	}

	static async clearEvents(
		securityEnabled: boolean,
		bearerToken: string
	): Promise<{ ok: boolean }> {
		const res = await fetch('/api/alarms/events', {
			method: 'DELETE',
			headers: apiAuthHeaders(securityEnabled, bearerToken)
		});
		if (!res.ok) throw new Error(`DELETE /api/alarms/events failed: ${res.status}`);
		return res.json();
	}

	static async test(
		securityEnabled: boolean,
		bearerToken: string,
		payload?: { severity?: string; message?: string }
	): Promise<{ ok: boolean; error?: string }> {
		const res = await fetch('/api/alarms/test', {
			method: 'POST',
			headers: {
				...apiAuthHeaders(securityEnabled, bearerToken),
				'Content-Type': 'application/json'
			},
			body: JSON.stringify(payload ?? {})
		});
		if (!res.ok) throw new Error(`POST /api/alarms/test failed: ${res.status}`);
		return res.json();
	}
}
