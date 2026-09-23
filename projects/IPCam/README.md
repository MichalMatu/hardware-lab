# IPCam

Panel do podgladu streamu z IP Webcam i wykrywania `dog` / `person` przez YOLO.

## Start

```bash
python -m venv venv
./venv/bin/pip install -r requirements.txt
YOLO_DEVICE=auto ./venv/bin/python app.py
```

Panel dziala pod:

```text
http://localhost:8000
```

Domyslny stream kamery:

```text
http://192.168.0.13:8080/video
```

Adres mozna zmienic w panelu bez restartu aplikacji.

## Modele

Modele YOLO sa lokalnymi artefaktami i nie sa trzymane w Git. Umiesc je w katalogu projektu, np.:

```text
yolov8s.pt
yolov8n.pt
yolov8m.pt
yolov8s_openvino_model/
```

Domyslnie aplikacja uzywa `yolov8s.pt`, chyba ze ustawisz inna sciezke przez panel albo zmienna `YOLO_MODEL`.

## Pliki

- `app.py` - serwer Flask, API i panel webowy.
- `monitor.py` - pipeline streamu, detekcji, zapisu i statusu.
- `detector.py` - integracja YOLO, device i mapowanie ROI.
- `source.py` - zrodlo klatek IP Webcam.
- `roi.py` - zapis i przeliczanie ROI.
- `recorder.py` - zapis zdjec i wideo zdarzen.
- `capture.py` - proste przechwytywanie klatek.
- `dog_detector.py` - pipeline z terminala bez panelu Flask.

## Nagrania

Wyniki trafiaja do `output/images` i `output/videos`. Katalog `output/` jest ignorowany przez Git.

Przykladowy tryb terminalowy:

```bash
./venv/bin/python dog_detector.py \
  --url http://192.168.0.13:8080/video \
  --model yolov8n.pt \
  --target-labels dog,person \
  --post-roll 60 \
  --min-detections 2 \
  --device auto
```
