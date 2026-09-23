import type { Enabled } from '$lib/types/common';

export type TelegramRecipient = {
	name: string;
	// Chat ID as string to avoid JS integer precision issues.
	chatId: string;
	enabled: Enabled;
};

export type TelegramNotificationsConfig = {
	schemaVersion: 1;
	enabled: Enabled;
	botToken: string;
	recipients: TelegramRecipient[];
	// Optional routing by severity (implementation-defined)
	severityRouting?: Record<string, string[]>;
	// Anti-spam / retries
	cooldownSeconds?: number;
	maxPerHour?: number;
};
