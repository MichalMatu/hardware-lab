# Telegram Notifications

## Overview

The firmware supports sending notifications to Telegram using the bot API. Notifications are sent via HTTPS (TLS) using a dedicated FreeRTOS task to prevent stack overflow.

## Configuration

### Build Flags (secrets.ini)

Create a `secrets.ini` file in the project root (gitignored):

```ini
[secrets]
build_flags = 
    -DTELEGRAM_BOT_TOKEN="YOUR_BOT_TOKEN_HERE"
    -DTELEGRAM_CHAT_ID="YOUR_CHAT_ID_HERE"
```

### How to Get Credentials

1. **Bot Token**: 
   - Talk to [@BotFather](https://t.me/BotFather) on Telegram
   - Use `/newbot` command to create a new bot
   - Copy the token (format: `123456789:ABCDEF...`)

2. **Chat ID**:
   - Start a chat with your bot
   - Send any message
   - Visit: `https://api.telegram.org/bot<YOUR_BOT_TOKEN>/getUpdates`
   - Find `"chat":{"id":YOUR_CHAT_ID}` in the response

## API Endpoint

### POST /api/notifications/telegram/test

Send a test notification to Telegram.

**Authentication**: JWT token (admin role required)

**Request Body**:
```json
{
  "text": "Your message here"
}
```

Notes:
- The backend also accepts legacy `{ "message": "..." }` for compatibility.
- Responses are always JSON (including errors).

**Response** (success):
```json
{
  "ok": true,
  "configured": true,
  "httpCode": 200,
  "error": "",
  "tlsError": "",
  "response": "{\"ok\":true,\"result\":{...}}"
}
```

**Response** (not configured):
```json
{
  "ok": false,
  "configured": false,
  "error": "Telegram not configured (TELEGRAM_BOT_TOKEN/TELEGRAM_CHAT_ID missing)"
}
```

## Implementation Details

### Stack Overflow Prevention

TLS handshake requires significant stack space (~8-12KB). To prevent stack overflow in HTTP handlers (which typically have 3-4KB stack), Telegram operations run in a dedicated FreeRTOS task:

```cpp
xTaskCreate(
    telegramSendTask,
    "telegram_send",
    12 * 1024,  // 12KB stack for TLS
    &payload,
    5,          // priority
    nullptr
);
```

### TLS Configuration

- **Default Mode**: `setInsecure()` (no certificate validation)
- **Optional (recommended)**: Root CA validation (certificate verification)
- **Handshake Timeout**: 15 seconds
- **Time/Internet Requirement (hard policy)**: WiFi must be connected, DNS + TCP probe to `api.telegram.org:443` must succeed, and system time must be in a valid window (firmware waits briefly for time to become valid).

#### Enabling TLS Certificate Validation

To enable certificate verification for Telegram, add a build flag:

```ini
-DTELEGRAM_TLS_VERIFY=1
```

When enabled, Telegram uses `setCACert(...)` with a pinned Root CA certificate (stored in flash) to validate the server certificate chain.
This approach is intentionally small to fit the default 2MB app partition.

If Telegram (or its CDN) changes to a different CA, you may need to update the pinned Root CA in the firmware and reflash.

> Note: The repository also contains a bundle generator ([scripts/generate_cert_bundle.py](scripts/generate_cert_bundle.py)), but the Telegram notifier does **not** require a CA bundle.
> Using a full bundle can significantly increase firmware size on ESP32.

## Troubleshooting (quick debug)

### 1) Always start with `/api/notifications/telegram/test`

This endpoint returns structured JSON so you can debug without guessing:

- `ok`: overall result
- `configured`: whether token/chat id are present
- `httpCode`: HTTP status from Telegram (or a negative error code)
- `error`: high-level firmware error string
- `tlsError`: last TLS error from the TLS stack (when available)
- `response`: Telegram JSON response when `httpCode` is 2xx

### 2) Common failure patterns

- `configured=false` + `error="Telegram not configured (...)"`
  - Missing build flags `TELEGRAM_BOT_TOKEN` / `TELEGRAM_CHAT_ID`.

- `error` starts with `offline/...`
  - `offline/wifi_off` / `offline/wifi_not_connected`: WiFi not connected.
  - `offline/dns_failed`: DNS failed (no internet / DNS issue).
  - `offline/tcp_connect_failed`: cannot connect to `api.telegram.org:443` (blocked internet).
  - `offline/time_invalid`: system time not yet valid (wait for NTP/time sync).

- `httpCode` is 401/403 with a Telegram JSON `response`
  - Usually wrong bot token or bot cannot message that chat.

- `httpCode < 0` and `tlsError` is set
  - Most likely TLS handshake/cert validation problem (see next section).

### 3) TLS verify problems (Pinned Root CA)

If `-DTELEGRAM_TLS_VERIFY=1` is enabled, the firmware validates Telegram's certificate chain against a pinned Root CA.
This is much smaller than a full CA bundle (fits the default 2MB app partition), but it's intentionally narrow.

If Telegram changes to a different Root CA, TLS verification may start failing.
Symptoms:

- `/api/notifications/telegram/test` returns `ok=false`, `httpCode<0` and a meaningful `tlsError`.

#### How to update the pinned Root CA

On your development machine, fetch the current certificate chain and identify the Root CA:

```bash
echo | openssl s_client -servername api.telegram.org -connect api.telegram.org:443 -showcerts 2>/dev/null \
  | awk '/BEGIN CERTIFICATE/{i++} {print > ("cert" i ".pem")}'

# Inspect subjects/issuers
for f in cert*.pem; do
  echo "=== $f ==="
  openssl x509 -noout -subject -issuer -dates -in "$f"
done
```

Then update the pinned PEM inside:

- Firmware file: [src/notifications/TelegramNotifier.cpp](../src/notifications/TelegramNotifier.cpp)
- Symbol: `kTelegramRootCaPem`

Rebuild and flash.

#### Temporary fallback (for diagnosis only)

If you need to confirm that “only TLS verify is failing”, rebuild without verification:

- remove `-DTELEGRAM_TLS_VERIFY=1`
- or set `-DTELEGRAM_TLS_VERIFY=0`

This reverts to `setInsecure()` (encryption without cert validation).

### 4) Debug report template (copy/paste)

When reporting a Telegram notification issue, paste the following (redact secrets):

**Environment**
- Firmware env: `esp32dev_dbg` / `esp32dev_rel`
- `TELEGRAM_TLS_VERIFY`: `0/1`
- Device IP:

**API test output**
```bash
# Login
curl -s http://<DEVICE_IP>/rest/signIn \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin"}'

# Telegram test
curl -s http://<DEVICE_IP>/api/notifications/telegram/test \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <JWT_TOKEN>" \
  -d '{"text":"debug test"}'
```

**What to include from the response**
- `ok`, `configured`, `httpCode`, `error`, `tlsError`
- `response` (if present)

**Connectivity checks**
```bash
ping -c 1 <DEVICE_IP>
curl -s -m 5 http://<DEVICE_IP>/rest/signIn -H "Content-Type: application/json" -d '{"username":"admin","password":"admin"}'
```

**Serial logs (first ~30 seconds after boot)**
- Start monitor: `pio device monitor -b 115200 --filter=esp32_exception_decoder`
- Paste logs showing WiFi IP + NTP start + any TLS errors.

### Error Handling

Common errors:
- `offline/wifi_off` / `offline/wifi_not_connected`: WiFi is not started or not connected
- `offline/dns_failed`: DNS resolution failed (no internet / DNS issue)
- `offline/tcp_connect_failed`: cannot connect to `api.telegram.org:443` (no internet / blocked)
- `offline/time_invalid`: system time not in valid range (wait for NTP/time sync)
- `input/text_too_long`: message exceeds the firmware limit (1024 chars)
- `Task timeout`: TLS handshake took > 30s (network issue)
- `busy/telegram_test_in_progress`: another `/api/notifications/telegram/test` is currently running (endpoint is single-flight; retry later)

Telegram HTTP status mapping (non-2xx responses still include the Telegram JSON in `response`):
- `telegram/http_401_unauthorized`
- `telegram/http_403_forbidden`
- `telegram/http_404_not_found`
- `telegram/http_429_rate_limited`
- `telegram/http_4xx_<code>` / `telegram/http_5xx_<code>`

## Usage Example

```bash
# Get JWT token
TOKEN=$(curl -s http://192.168.0.49/rest/signIn \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin"}' | jq -r '.access_token')

# Send notification
curl http://192.168.0.49/api/notifications/telegram/test \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"text":"Sensor alert: Temperature exceeded 30°C"}'
```

## Future Enhancements

- [ ] Move credentials from compile-time to LittleFS config (runtime editable)
- [ ] Add optional CA certificate bundle for other integrations (size permitting)
- [ ] Support notification templates
- [ ] Add rate limiting
- [ ] Support sending images/charts
- [ ] Integration with alarm system (auto-notify on threshold breach)
