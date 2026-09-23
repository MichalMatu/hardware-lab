/**
 * RF433 API types
 */

export interface Rf433Device {
	id: string;
	label: string;
	code_on: number;
	code_off: number;
	bit_length: number;
	protocol: number;
	pulse_length: number;
	repeat: number;
}

export interface Rf433DeviceState {
	device_id: string;
	last_command_on: boolean;
	last_transmit_ms: number;
	age_ms: number;
	age_sec: number;
}

export interface Rf433DevicesResponse {
	devices: Rf433Device[];
	count: number;
	max_devices: number;
}

export interface Rf433StateResponse {
	states: Rf433DeviceState[];
	count: number;
	controller_ready: boolean;
	rf433_enabled: boolean;
}

export interface Rf433SendCommandRequest {
	device_id: string;
	command: 'on' | 'off';
	sync?: boolean;
}

export interface Rf433SendCommandResponse {
	success: boolean;
	device_id: string;
	command: string;
	error?: string;
}
