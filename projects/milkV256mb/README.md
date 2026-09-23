# Milk-V Duo 256 - Połączenie SSH przez USB

## Wymagania
- Kabel USB podłączony do Milk-V Duo
- macOS (obsługuje ECM natywnie, bez dodatkowych sterowników)

## Połączenie

```bash
ssh root@192.168.42.1
```

- **Użytkownik:** `root`
- **Hasło:** `milkv`

## Weryfikacja połączenia USB

```bash
ifconfig | grep -B 1 "192.168"
```

Interfejs USB pojawi się z adresem `192.168.42.x` (np. `en8`).

## Keepalive - zapobieganie zrywaniu połączenia

### Na Milk-V (jednorazowo)

```bash
mkdir -p /etc/default
echo 'DROPBEAR_ARGS="-K 30"' > /etc/default/dropbear
/etc/init.d/S50dropbear restart
```

### Na Macu (~/.ssh/config)

```
Host 192.168.42.1
    ServerAliveInterval 30
    ServerAliveCountMax 3
    IPQoS none
```

## RAM

Milk-V Duo 256 ma 256MB RAM, ale system widzi tylko ~166MB.
Pozostałe ~90MB jest zarezerwowane przez bootloader dla ISP/TPU (kamera, akcelerator AI).
