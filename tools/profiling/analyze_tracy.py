"""Summarize the final complete steady-state frames exported by Tracy 0.13.1."""
import argparse
from bisect import bisect_right
import csv
import json
from pathlib import Path
import statistics


def stats(values):
    ordered = sorted(values)
    if not ordered:
        return {}

    def percentile(fraction):
        position = (len(ordered) - 1) * fraction
        low = int(position)
        high = min(low + 1, len(ordered) - 1)
        return ordered[low] + (ordered[high] - ordered[low]) * (position - low)

    return {"n": len(ordered), "mean": statistics.mean(ordered), "p50": percentile(.5),
            "p95": percentile(.95), "p99": percentile(.99), "max": ordered[-1]}


def analyze(directory, sample_count):
    with (directory / "events.csv").open(newline="", encoding="utf-8-sig") as file:
        events = list(csv.DictReader(file))
    for event in events:
        event["start"] = int(event["ns_since_start"])
        event["duration"] = int(event["exec_time_ns"])
        event["end"] = event["start"] + event["duration"]
    frames = sorted((event for event in events if event["name"] == "Frame"), key=lambda event: event["start"])
    rendering = sorted(event["start"] for event in events if event["name"] == "Graphics::render")
    # The quit callback creates one short Frame without rendering. Exclude it.
    frames = [frame for frame in frames if (index := bisect_right(rendering, frame["start"])) < len(rendering) and rendering[index] < frame["end"]][-sample_count:]
    if len(frames) != sample_count:
        raise RuntimeError("Not enough complete steady frames: %s" % len(frames))
    starts = [frame["start"] for frame in frames]
    zones = {}
    counts = {}
    for event in events:
        index = bisect_right(starts, event["start"]) - 1
        if index >= 0 and event["end"] <= frames[index]["end"] and event["thread"] == frames[index]["thread"]:
            name = event["name"]
            zones.setdefault(name, [0.0] * len(frames))[index] += event["duration"] / 1e6
            counts[name] = counts.get(name, 0) + 1
    without_waits = [frame["duration"] / 1e6 - zones.get("GPU submit", [0] * sample_count)[index] - zones.get("GPU wait idle", [0] * sample_count)[index] for index, frame in enumerate(frames)]
    result = {"sample_frames": len(frames),
              "frame_interval_ms": stats([(starts[index + 1] - starts[index]) / 1e6 for index in range(len(starts) - 1)]),
              "frame_without_submit_idle_ms": stats(without_waits),
              "zones_ms_per_frame": {name: stats(values) for name, values in zones.items()},
              "zone_calls_per_frame": {name: count / len(frames) for name, count in counts.items()},
              "startup_workers_ms": [{"name": event["name"], "duration_ms": event["duration"] / 1e6} for event in events if event["name"].startswith("IBL ")],
              "measurement_start_ns": frames[0]["start"], "measurement_end_ns": frames[-1]["end"]}
    (directory / "summary.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
    return {"run": directory.name, "interval": result["frame_interval_ms"],
            "work": result["frame_without_submit_idle_ms"],
            "zones_mean_ms": {name: value["mean"] for name, value in result["zones_ms_per_frame"].items()},
            "calls": result["zone_calls_per_frame"], "startup": result["startup_workers_ms"]}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directories", nargs="+", type=Path)
    parser.add_argument("--samples", type=int, default=300)
    args = parser.parse_args()
    for directory in args.directories:
        print(json.dumps(analyze(directory, args.samples)), flush=True)
