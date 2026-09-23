import { writable } from 'svelte/store';
import type { RSSI } from '../types/models';
import type { Battery } from '../types/models';

let telemetry_data = {
	rssi: {
		rssi: 0,
		ssid: '',
		disconnected: true
	},
	battery: {
		soc: 100,
		charging: false
	}
};

function createTelemetry() {
	const { subscribe, update } = writable(telemetry_data);

	return {
		subscribe,
		setRSSI: (data: RSSI) => {
			if (!isNaN(Number(data.rssi))) {
				update((telemetry_data) => ({
					...telemetry_data,
					rssi: { rssi: Number(data.rssi), ssid: data.ssid, disconnected: false }
				}));
			} else {
				update((telemetry_data) => ({
					...telemetry_data,
					rssi: { rssi: 0, ssid: data.ssid, disconnected: true }
				}));
			}
		},
		setBattery: (data: Battery) => {
			update((telemetry_data) => ({
				...telemetry_data,
				battery: { soc: data.soc, charging: data.charging }
			}));
		}
	};
}

export const telemetry = createTelemetry();
