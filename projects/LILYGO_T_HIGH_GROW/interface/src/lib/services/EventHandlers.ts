import { telemetry } from '$lib/stores/telemetry';
import { batteryHistory } from '$lib/stores/battery';
import { notifications } from '$lib/components/toasts/notifications';
import type { RSSI, Battery } from '$lib/types/models';

export class EventHandlers {
	private wasConnected: boolean | null = null;

	handleOpen = () => {
		// Only show reconnect notification, not on initial connection
		if (this.wasConnected === false) {
			notifications.success('Connection to device established', 5000);
		}
		this.wasConnected = true;
	};

	handleClose = () => {
		// Only show disconnect notification if we were previously connected
		if (this.wasConnected === true) {
			notifications.error('Connection to device lost', 5000);
			this.wasConnected = false;
		}
		telemetry.setRSSI({ rssi: 0, ssid: '' });
	};

	handleError = (_error: unknown) => {
		// Suppress error logs during reconnection
	};

	handleNotification = (data: {
		type: 'info' | 'warning' | 'error' | 'success';
		message: string;
	}) => {
		switch (data.type) {
			case 'info':
				notifications.info(data.message, 5000);
				break;
			case 'warning':
				notifications.warning(data.message, 5000);
				break;
			case 'error':
				notifications.error(data.message, 5000);
				break;
			case 'success':
				notifications.success(data.message, 5000);
				break;
			default:
				break;
		}
	};

	handleNetworkStatus = (data: RSSI) => {
		telemetry.setRSSI(data);
	};

	handleBattery = (data: Battery) => {
		telemetry.setBattery(data);
		batteryHistory.addData(data);
	};

	getConnectionState() {
		return this.wasConnected;
	}
}
