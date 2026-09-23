import type { Enabled, EntityId, ISODateTimeString, Severity } from '$lib/types/common';

export type AlarmState = 'triggered' | 'cleared';

export type AlarmCondition = {
	signal: string;
	op: 'lt' | 'lte' | 'gt' | 'gte' | 'eq' | 'neq';
	value: number;
	forSeconds?: number;
};

export type AlarmRule = {
	id: EntityId;
	enabled: Enabled;
	name: string;
	severity: Severity;
	condition: AlarmCondition;
	// Optional stabilization
	hysteresis?: number;
	cooldownSeconds?: number;
	reminderSeconds?: number;
	// Notify until cleared, or send periodically
	notifyMode?: 'once-until-cleared' | 'reminder';
	createdAt?: ISODateTimeString;
	updatedAt?: ISODateTimeString;
};

export type AlarmRulesConfig = {
	schemaVersion: 1;
	rules: AlarmRule[];
};

export type AlarmEvent = {
	id: EntityId;
	at: ISODateTimeString;
	ruleId?: EntityId;
	ruleName?: string;
	severity: Severity;
	state: AlarmState;
	message: string;
	// Optional: delivery status summary
	delivery?: {
		telegram?: {
			ok: boolean;
			error?: string;
		};
	};
};

export type AlarmEventsPage = {
	schemaVersion: 1;
	events: AlarmEvent[];
	nextCursor?: string;
};
