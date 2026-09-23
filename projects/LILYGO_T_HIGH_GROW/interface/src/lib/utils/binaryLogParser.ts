/**
 * Binary log file format parser
 *
 * File structure:
 * - Header: 8 bytes (magic, version, recordSize, reserved)
 * - Records: N × 16 bytes each
 *
 * Record layout (16 bytes, little-endian):
 * - timestamp (uint32, 4B): Unix timestamp in seconds
 * - temp_10x (int16, 2B): Temperature × 10 (-200 to +600 = -20.0°C to +60.0°C)
 * - humid_10x (uint16, 2B): Humidity × 10 (0 to 1000 = 0.0% to 100.0%)
 * - lux (uint16, 2B): Light intensity (0 to 65535 lux)
 * - salt (uint16, 2B): Soil conductivity (0 to 2000 µS/cm)
 * - batVolt_mv (uint16, 2B): Battery voltage in millivolts
 * - soil (uint8, 1B): Soil moisture (0 to 100%)
 * - batPerc (uint8, 1B): Battery percentage (0 to 100%)
 */

const BINARY_MAGIC = 0x504c4e54; // "PLNT" in ASCII
const EXPECTED_VERSION = 1;
const HEADER_SIZE = 8;
const RECORD_SIZE = 16;

export interface BinaryLogHeader {
	magic: number;
	version: number;
	recordSize: number;
	reserved: number;
}

export interface ParsedLogData {
	timestamps: number[]; // Unix timestamps in seconds
	temps: (number | null)[];
	humids: (number | null)[];
	lux: (number | null)[];
	soils: (number | null)[];
	salts: (number | null)[];
	batteries: (number | null)[]; // battery percentage
}

export class BinaryLogParseError extends Error {
	constructor(message: string) {
		super(message);
		this.name = 'BinaryLogParseError';
	}
}

/**
 * Parse binary log file header
 */
function parseHeader(view: DataView): BinaryLogHeader {
	const header: BinaryLogHeader = {
		magic: view.getUint32(0, true), // little-endian
		version: view.getUint8(4),
		recordSize: view.getUint8(5),
		reserved: view.getUint16(6, true)
	};

	if (header.magic !== BINARY_MAGIC) {
		throw new BinaryLogParseError(
			`Invalid magic: 0x${header.magic.toString(16)} (expected 0x${BINARY_MAGIC.toString(16)})`
		);
	}

	if (header.version !== EXPECTED_VERSION) {
		throw new BinaryLogParseError(
			`Unsupported version: ${header.version} (expected ${EXPECTED_VERSION})`
		);
	}

	if (header.recordSize !== RECORD_SIZE) {
		throw new BinaryLogParseError(
			`Invalid record size: ${header.recordSize} (expected ${RECORD_SIZE})`
		);
	}

	return header;
}

/**
 * Parse single binary log record
 */
function parseRecord(view: DataView, offset: number) {
	const timestamp = view.getUint32(offset, true);
	const temp_10x = view.getInt16(offset + 4, true);
	const humid_10x = view.getUint16(offset + 6, true);
	const lux = view.getUint16(offset + 8, true);
	const salt = view.getUint16(offset + 10, true);
	const batVolt_mv = view.getUint16(offset + 12, true);
	const soil = view.getUint8(offset + 14);
	const batPerc = view.getUint8(offset + 15);

	// Convert fixed-point values to floats
	// INT16_MIN is used as NaN marker
	const temp = temp_10x === -32768 ? null : temp_10x / 10.0;
	const humid = humid_10x / 10.0;
	const _batVolt = batVolt_mv / 1000.0; // Reserved for future use

	return {
		timestamp,
		temp,
		humid,
		lux,
		soil,
		salt,
		batPerc
	};
}

/**
 * Parse complete binary log file
 * @param arrayBuffer Raw binary data from file
 * @returns Parsed data arrays suitable for charting
 */
export function parseBinaryLog(arrayBuffer: ArrayBuffer): ParsedLogData {
	const view = new DataView(arrayBuffer);

	// Parse and validate header
	if (view.byteLength < HEADER_SIZE) {
		throw new BinaryLogParseError('File too small (no header)');
	}

	parseHeader(view);

	// Parse records
	const recordCount = Math.floor((view.byteLength - HEADER_SIZE) / RECORD_SIZE);
	const data: ParsedLogData = {
		timestamps: [],
		temps: [],
		humids: [],
		lux: [],
		soils: [],
		salts: [],
		batteries: []
	};

	for (let i = 0; i < recordCount; i++) {
		const offset = HEADER_SIZE + i * RECORD_SIZE;
		try {
			const record = parseRecord(view, offset);
			data.timestamps.push(record.timestamp);
			data.temps.push(record.temp);
			data.humids.push(record.humid);
			data.lux.push(record.lux);
			data.soils.push(record.soil);
			data.salts.push(record.salt);
			data.batteries.push(record.batPerc);
		} catch (err) {
			console.warn(`Failed to parse record ${i} at offset ${offset}:`, err);
			// Push nulls to maintain array length consistency
			data.timestamps.push(0);
			data.temps.push(null);
			data.humids.push(null);
			data.lux.push(null);
			data.soils.push(null);
			data.salts.push(null);
			data.batteries.push(null);
		}
	}

	return data;
}

/**
 * Calculate estimated record count from file size
 */
export function estimateRecordCount(fileSize: number): number {
	if (fileSize <= HEADER_SIZE) return 0;
	return Math.floor((fileSize - HEADER_SIZE) / RECORD_SIZE);
}
