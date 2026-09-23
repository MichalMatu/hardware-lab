import { apiAuthHeaders } from '$lib/services/apiAuthHeaders';
import type { TelegramNotificationsConfig } from '$lib/types/notifications/telegram';

export class TelegramApiService {
	static async getSettings(
		securityEnabled: boolean,
		bearerToken: string
	): Promise<TelegramNotificationsConfig> {
		const res = await fetch('/api/notifications/telegram', {
			method: 'GET',
			headers: apiAuthHeaders(securityEnabled, bearerToken)
		});
		if (!res.ok) throw new Error(`GET /api/notifications/telegram failed: ${res.status}`);
		return res.json();
	}

	static async putSettings(
		securityEnabled: boolean,
		bearerToken: string,
		payload: TelegramNotificationsConfig
	): Promise<TelegramNotificationsConfig> {
		const res = await fetch('/api/notifications/telegram', {
			method: 'PUT',
			headers: {
				...apiAuthHeaders(securityEnabled, bearerToken),
				'Content-Type': 'application/json'
			},
			body: JSON.stringify(payload)
		});
		if (!res.ok) throw new Error(`PUT /api/notifications/telegram failed: ${res.status}`);
		return res.json();
	}

	static async test(
		securityEnabled: boolean,
		bearerToken: string,
		payload?: { text?: string; message?: string },
		options?: { signal?: AbortSignal }
	): Promise<{
		ok: boolean;
		configured?: boolean;
		httpCode?: number;
		error?: string;
		tlsError?: string;
		response?: string;
	}> {
		const text = payload?.text ?? payload?.message;
		const res = await fetch('/api/notifications/telegram/test', {
			method: 'POST',
			headers: {
				...apiAuthHeaders(securityEnabled, bearerToken),
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ text: text ?? '' }),
			signal: options?.signal
		});

		let data:
			| {
					ok: boolean;
					configured?: boolean;
					httpCode?: number;
					error?: string;
					tlsError?: string;
					response?: string;
			  }
			| undefined;
		try {
			data = await res.json();
		} catch {
			data = undefined;
		}

		// Backend returns a stable JSON contract even on errors like 429.
		// However, auth failures (401) may not be JSON, so keep a safe fallback.
		if (!res.ok) {
			if (data) return data;
			throw new Error(`POST /api/notifications/telegram/test failed: ${res.status}`);
		}

		if (!data) {
			throw new Error('POST /api/notifications/telegram/test failed: invalid JSON response');
		}
		return data;
	}

	static async fetchChatId(
		securityEnabled: boolean,
		bearerToken: string,
		botToken: string,
		options?: { signal?: AbortSignal }
	): Promise<{
		ok: boolean;
		configured?: boolean;
		httpCode?: number;
		error?: string;
		tlsError?: string;
		chats?: Array<{
			id: string;
			type: string;
			name?: string;
		}>;
	}> {
		const res = await fetch('/api/notifications/telegram/get-chat-id', {
			method: 'POST',
			headers: {
				...apiAuthHeaders(securityEnabled, bearerToken),
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ bot_token: botToken }),
			signal: options?.signal
		});

		let data:
			| {
					ok: boolean;
					configured?: boolean;
					httpCode?: number;
					error?: string;
					tlsError?: string;
					chats?: Array<{
						id: string;
						type: string;
						name?: string;
					}>;
			  }
			| undefined;
		try {
			data = await res.json();
		} catch {
			data = undefined;
		}

		if (!res.ok) {
			if (data) return data;
			throw new Error(`POST /api/notifications/telegram/get-chat-id failed: ${res.status}`);
		}

		if (!data) {
			throw new Error('POST /api/notifications/telegram/get-chat-id failed: invalid JSON response');
		}
		return data;
	}
}
