export type HttpMethod = 'GET' | 'PUT' | 'POST' | 'DELETE';

export type PlannedEndpoint = {
	method: HttpMethod;
	path: string;
	purpose: string;
	request?: string;
	response?: string;
	storage?: string;
};

export type PlannedApiSection = {
	title: string;
	notes?: string[];
	endpoints: PlannedEndpoint[];
};

export const plannedApi: Record<string, PlannedApiSection> = {
	automation: {
		title: 'Automation (planned API)',
		notes: [
			'Persist rules/schedules in LittleFS as a versioned JSON config.',
			'Runtime should load config on boot and keep an in-memory copy for fast evaluation.',
			'Actions should be safe in sleep modes (wake → evaluate → act → sleep).'
		],
		endpoints: [
			{
				method: 'GET',
				path: '/api/automation/rules',
				purpose: 'Fetch all automation rules.',
				response: 'AutomationRulesConfig'
			},
			{
				method: 'PUT',
				path: '/api/automation/rules',
				purpose: 'Replace all automation rules (atomic save).',
				request: 'AutomationRulesConfig',
				response: 'AutomationRulesConfig',
				storage: 'LittleFS (config file)'
			},
			{
				method: 'GET',
				path: '/api/automation/schedules',
				purpose: 'Fetch all schedules.',
				response: 'AutomationSchedulesConfig'
			},
			{
				method: 'PUT',
				path: '/api/automation/schedules',
				purpose: 'Replace all schedules (atomic save).',
				request: 'AutomationSchedulesConfig',
				response: 'AutomationSchedulesConfig',
				storage: 'LittleFS (config file)'
			},
			{
				method: 'POST',
				path: '/api/automation/run',
				purpose: 'Run a rule/schedule action now (test / manual trigger).',
				request: '{ id: string }',
				response: '{ ok: boolean; error?: string }'
			}
		]
	},
	alarms: {
		title: 'Alarms (planned API)',
		notes: [
			'Persist alarm rules in LittleFS as a versioned JSON config.',
			'Alarm event history must be strictly bounded: fixed-size ring buffer (no unbounded append-only logs).',
			'Pre-release assumption: no migrations. If the stored format changes, we can clear/reset stored data.'
		],
		endpoints: [
			{
				method: 'GET',
				path: '/api/alarms/rules',
				purpose: 'Fetch all alarm rules.',
				response: 'AlarmRulesConfig'
			},
			{
				method: 'PUT',
				path: '/api/alarms/rules',
				purpose: 'Replace all alarm rules (atomic save).',
				request: 'AlarmRulesConfig',
				response: 'AlarmRulesConfig',
				storage: 'LittleFS (config file)'
			},
			{
				method: 'GET',
				path: '/api/alarms/events',
				purpose: 'Fetch alarm event history (paged).',
				response: 'AlarmEventsPage'
			},
			{
				method: 'DELETE',
				path: '/api/alarms/events',
				purpose: 'Clear alarm event history.',
				response: '{ ok: boolean }',
				storage: 'LittleFS (events log)'
			},
			{
				method: 'POST',
				path: '/api/alarms/test',
				purpose: 'Simulate an alarm and send a test notification.',
				request: '{ severity?: string; message?: string }',
				response: '{ ok: boolean; error?: string }'
			}
		]
	},
	integrationsEspNow: {
		title: 'ESP-NOW Integration (planned API)',
		notes: [
			'Persist peers and mapping in LittleFS.',
			'Keep last-seen and counters in RAM (optionally persist snapshots).'
		],
		endpoints: [
			{
				method: 'GET',
				path: '/api/integrations/espnow',
				purpose: 'Fetch ESP-NOW peer configuration.',
				response: 'EspNowIntegrationConfig'
			},
			{
				method: 'PUT',
				path: '/api/integrations/espnow',
				purpose: 'Save ESP-NOW peer configuration.',
				request: 'EspNowIntegrationConfig',
				response: 'EspNowIntegrationConfig',
				storage: 'LittleFS (config file)'
			},
			{
				method: 'POST',
				path: '/api/integrations/espnow/test',
				purpose: 'Send a test command to a peer.',
				request: '{ peerId: string; command: string }',
				response: '{ ok: boolean; error?: string }'
			}
		]
	},
	integrations433: {
		title: '433MHz Integration (planned API)',
		notes: ['Persist profiles and TX settings in LittleFS.'],
		endpoints: [
			{
				method: 'GET',
				path: '/api/integrations/433',
				purpose: 'Fetch 433MHz profiles and TX settings.',
				response: 'Integration433Config'
			},
			{
				method: 'PUT',
				path: '/api/integrations/433',
				purpose: 'Save 433MHz profiles and TX settings.',
				request: 'Integration433Config',
				response: 'Integration433Config',
				storage: 'LittleFS (config file)'
			},
			{
				method: 'POST',
				path: '/api/integrations/433/test',
				purpose: 'Transmit a test command.',
				request: '{ profileId: string; command: string }',
				response: '{ ok: boolean; error?: string }'
			}
		]
	},
	telegram: {
		title: 'Telegram Notifications (planned API)',
		notes: [
			'Persist token/recipients in LittleFS; keep sensitive values out of logs.',
			'HTTPS/TLS is required to call api.telegram.org directly (or use a proxy).'
		],
		endpoints: [
			{
				method: 'GET',
				path: '/api/notifications/telegram',
				purpose: 'Fetch Telegram notification settings.',
				response: 'TelegramNotificationsConfig'
			},
			{
				method: 'PUT',
				path: '/api/notifications/telegram',
				purpose: 'Save Telegram notification settings.',
				request: 'TelegramNotificationsConfig',
				response: 'TelegramNotificationsConfig',
				storage: 'LittleFS (config file)'
			},
			{
				method: 'POST',
				path: '/api/notifications/telegram/test',
				purpose: 'Send a test message.',
				request: '{ message?: string }',
				response: '{ ok: boolean; error?: string }'
			}
		]
	}
};
