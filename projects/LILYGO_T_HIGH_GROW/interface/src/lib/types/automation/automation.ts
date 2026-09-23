import type { Enabled, EntityId, ISODateTimeString } from '$lib/types/common';
import type { ActuatorId } from '$lib/types/integrations/integrations';

export type ComparisonOperator = 'lt' | 'lte' | 'gt' | 'gte' | 'eq' | 'neq';

export type AutomationAction = {
	actuatorId: ActuatorId;
	// e.g. "on", "off", "toggle", "pulse"
	command: string;
	// optional parameters (duration_ms, pwm, etc.)
	params?: Record<string, number | string | boolean>;
};

export type AutomationCondition = {
	// e.g. "soil_moisture", "temp_c", "battery_soc", or a derived signal.
	signal: string;
	op: ComparisonOperator;
	value: number;
	// Optional: require the condition to hold continuously for N seconds.
	forSeconds?: number;
};

export type AutomationRule = {
	id: EntityId;
	enabled: Enabled;
	name: string;
	condition: AutomationCondition;
	action: AutomationAction;
	// Optional stabilization
	hysteresis?: number;
	cooldownSeconds?: number;
	maxRunsPerDay?: number;
	createdAt?: ISODateTimeString;
	updatedAt?: ISODateTimeString;
};

export type ScheduleType = 'once' | 'daily' | 'weekdays' | 'interval';

export type AutomationSchedule = {
	id: EntityId;
	enabled: Enabled;
	name: string;
	type: ScheduleType;
	// For once/daily/weekdays: local time "HH:MM".
	at?: string;
	// For interval: run every N minutes.
	everyMinutes?: number;
	action: AutomationAction;
	// Optional:
	timezone?: string;
	missedBehavior?: 'skip' | 'run-once' | 'catch-up';
	createdAt?: ISODateTimeString;
	updatedAt?: ISODateTimeString;
};

export type AutomationRulesConfig = {
	schemaVersion: 1;
	rules: AutomationRule[];
};

export type AutomationSchedulesConfig = {
	schemaVersion: 1;
	schedules: AutomationSchedule[];
};
