from monitor_logic import (
    choose_detect_interval,
    count_target_labels,
    is_tracking_active,
    trigger_ready,
    trigger_visible,
    update_consecutive_hits,
    visible_boxes,
)


def box(label: str):
    return (0, 0, 10, 10, label, 0.9)


def test_tracking_window_and_cadence():
    detection = {"active_detect_seconds": 0.2, "idle_detect_seconds": 1.5}
    boxes = [box("dog")]

    assert is_tracking_active(boxes, now=10.0, last_positive_at=9.5, active_track_seconds=1.0)
    assert choose_detect_interval(active_tracking=True, detection=detection) == 0.2
    assert choose_detect_interval(active_tracking=False, detection=detection) == 1.5
    assert visible_boxes(boxes, now=10.0, last_positive_at=9.5, active_track_seconds=1.0) == boxes
    assert visible_boxes(boxes, now=11.0, last_positive_at=9.5, active_track_seconds=1.0) == []


def test_counts_and_trigger_labels_are_independent():
    boxes = [box("dog"), box("dog"), box("person"), box("cat")]
    dog_count, person_count = count_target_labels(boxes)

    assert (dog_count, person_count) == (2, 1)
    assert trigger_visible(trigger_labels=["dog"], dog_count=dog_count, person_count=person_count)
    assert trigger_visible(trigger_labels=["person"], dog_count=0, person_count=person_count)
    assert not trigger_visible(trigger_labels=["person"], dog_count=dog_count, person_count=0)


def test_hit_counters_only_advance_on_detection_frames():
    hits = {"dog": 1, "person": 2}

    unchanged = update_consecutive_hits(
        hits, should_detect=False, dog_count=0, person_count=0
    )
    assert unchanged == hits

    advanced = update_consecutive_hits(
        hits, should_detect=True, dog_count=1, person_count=0
    )
    assert advanced == {"dog": 2, "person": 0}


def test_trigger_requires_visibility_and_configured_minimum_hits():
    assert trigger_ready(
        trigger_is_visible=True,
        trigger_labels=["dog"],
        hits={"dog": 3, "person": 0},
        min_hits_dog=3,
        min_hits_person=2,
    )
    assert not trigger_ready(
        trigger_is_visible=True,
        trigger_labels=["dog"],
        hits={"dog": 2, "person": 10},
        min_hits_dog=3,
        min_hits_person=2,
    )
    assert not trigger_ready(
        trigger_is_visible=False,
        trigger_labels=["dog", "person"],
        hits={"dog": 10, "person": 10},
        min_hits_dog=1,
        min_hits_person=1,
    )
