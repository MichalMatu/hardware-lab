import { socket } from '$lib/stores/socket';
import type { LayoutData } from '../../routes/$types';
import type { RSSI, Battery } from '$lib/types/models';

export interface WebSocketEventHandlers {
	onOpen?: () => void;
	onClose?: () => void;
	onError?: (data: unknown) => void;
	onRSSI?: (data: RSSI) => void;
	onNotification?: (data: {
		type: 'info' | 'warning' | 'error' | 'success';
		message: string;
	}) => void;
	onBattery?: (data: Battery) => void;
}

export class WebSocketManager {
	private handlers: WebSocketEventHandlers = {};
	private pageData: LayoutData;

	constructor(pageData: LayoutData) {
		this.pageData = pageData;
	}

	init(bearerToken: string, handlers: WebSocketEventHandlers) {
		this.handlers = handlers;
		const wsToken = this.pageData.features.security ? `?access_token=${bearerToken}` : '';
		const wsProtocol = window.location.protocol === 'https:' ? 'wss' : 'ws';

		socket.init(`${wsProtocol}://${window.location.host}/ws/events${wsToken}`);

		this.addEventListeners();
	}

	private addEventListeners() {
		if (this.handlers.onOpen) socket.on('open', this.handlers.onOpen);
		if (this.handlers.onClose) socket.on('close', this.handlers.onClose);
		if (this.handlers.onError) socket.on('error', this.handlers.onError);
		if (this.handlers.onRSSI) socket.on('rssi', this.handlers.onRSSI);
		if (this.handlers.onNotification) socket.on('notification', this.handlers.onNotification);
		if (this.pageData.features.battery && this.handlers.onBattery) {
			socket.on('battery', this.handlers.onBattery);
		}
	}

	removeEventListeners() {
		if (this.handlers.onOpen) socket.off('open', this.handlers.onOpen as (data: unknown) => void);
		if (this.handlers.onClose)
			socket.off('close', this.handlers.onClose as (data: unknown) => void);
		if (this.handlers.onError)
			socket.off('error', this.handlers.onError as (data: unknown) => void);
		if (this.handlers.onRSSI) socket.off('rssi', this.handlers.onRSSI as (data: unknown) => void);
		if (this.handlers.onNotification)
			socket.off('notification', this.handlers.onNotification as (data: unknown) => void);
		if (this.handlers.onBattery)
			socket.off('battery', this.handlers.onBattery as (data: unknown) => void);
	}
}
