from __future__ import annotations

from collections.abc import Mapping, Sequence
from typing import Any

Detection = Sequence[Any]


def is_tracking_active(
    boxes: Sequence[Detection],
    *,
    now: float,
    last_positive_at: float,
    active_track_seconds: float,
) -> bool:
    """Return whether the most recent positive detection is still considered visible."""
    return bool(boxes) and now - last_positive_at <= active_track_seconds


def choose_detect_interval(*, active_tracking: bool, detection: Mapping[str, Any]) -> float:
    """Choose the configured inference cadence for the current tracking state."""
    key = "active_detect_seconds" if active_tracking else "idle_detect_seconds"
    return float(detection[key])


def visible_boxes(
    boxes: Sequence[Detection],
    *,
    now: float,
    last_positive_at: float,
    active_track_seconds: float,
) -> list[Detection]:
    """Return a defensive copy of detections that are still inside the tracking window."""
    if not is_tracking_active(
        boxes,
        now=now,
        last_positive_at=last_positive_at,
        active_track_seconds=active_track_seconds,
    ):
        return []
    return list(boxes)


def count_target_labels(boxes: Sequence[Detection]) -> tuple[int, int]:
    """Count dog/person detections using the repository's normalized box layout."""
    dog_count = 0
    person_count = 0
    for box in boxes:
        if len(box) <= 4:
            continue
        if box[4] == "dog":
            dog_count += 1
        elif box[4] == "person":
            person_count += 1
    return dog_count, person_count


def trigger_visible(*, trigger_labels: Sequence[str], dog_count: int, person_count: int) -> bool:
    labels = set(trigger_labels)
    return ("dog" in labels and dog_count > 0) or ("person" in labels and person_count > 0)


def update_consecutive_hits(
    hits: Mapping[str, int],
    *,
    should_detect: bool,
    dog_count: int,
    person_count: int,
) -> dict[str, int]:
    """Advance hit counters only on frames where inference actually ran."""
    updated = {
        "dog": int(hits.get("dog", 0)),
        "person": int(hits.get("person", 0)),
    }
    if not should_detect:
        return updated
    updated["dog"] = updated["dog"] + 1 if dog_count else 0
    updated["person"] = updated["person"] + 1 if person_count else 0
    return updated


def trigger_ready(
    *,
    trigger_is_visible: bool,
    trigger_labels: Sequence[str],
    hits: Mapping[str, int],
    min_hits_dog: int,
    min_hits_person: int,
) -> bool:
    if not trigger_is_visible:
        return False
    labels = set(trigger_labels)
    dog_ready = "dog" in labels and int(hits.get("dog", 0)) >= min_hits_dog
    person_ready = "person" in labels and int(hits.get("person", 0)) >= min_hits_person
    return dog_ready or person_ready
