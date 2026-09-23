import type { EntityId, Enabled } from '$lib/types/common';

export type IntegrationType = '433mhz';

export type ActuatorId = EntityId;

export type Actuator = {
	id: ActuatorId;
	enabled: Enabled;
	name: string;
	integration: IntegrationType;
	// Integration-specific target reference (e.g. 433 profile id + command)
	targetRef: string;
};

export type IntegrationConfigBase = {
	schemaVersion: 1;
	enabled: Enabled;
};

export type Remote433Profile = {
	id: EntityId;
	enabled: Enabled;
	name: string;
	// Implementation-defined: raw code, protocol name, or a structured descriptor.
	commands: Record<string, string>;
};

export type Integration433Config = IntegrationConfigBase & {
	integration: '433mhz';
	txGpio?: number;
	repeat?: number;
	profiles: Remote433Profile[];
};
