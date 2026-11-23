"""Run with qrenderdoc --python; SINEN_PROFILE_RDC/SINEN_PROFILE_REPLAY select files."""
import json
import os
import traceback
from collections import Counter

import renderdoc as rd


def replay(path):
    capture = rd.OpenCaptureFile()
    controller = None
    try:
        status = capture.OpenFile(path, "", None)
        if status != rd.ResultCode.Succeeded:
            raise RuntimeError(str(status))
        status, controller = capture.OpenCapture(rd.ReplayOptions(), None)
        if status != rd.ResultCode.Succeeded:
            raise RuntimeError(str(status))
        actions = []

        def visit(nodes):
            for action in nodes:
                actions.append(action)
                visit(action.children)

        visit(controller.GetRootActions())
        structured = controller.GetStructuredFile()
        draws = [action for action in actions if action.flags & rd.ActionFlags.Drawcall]
        action_names = Counter(action.GetName(structured) for action in actions)
        counters = [counter for counter in controller.EnumerateCounters() if 0 < int(counter) <= 13]
        values = controller.FetchCounters(counters)
        durations = {}
        totals = {}
        for value in values:
            desc = controller.DescribeCounter(value.counter)
            number = value.value.d if int(value.counter) == 1 else value.value.u64
            totals[desc.name] = totals.get(desc.name, 0) + number
            if int(value.counter) == 1:
                durations[value.eventId] = number * 1000
        draw_details = []
        for action in draws:
            controller.SetFrameEvent(action.eventId, True)
            pipeline = controller.GetPipelineState()
            draw_details.append({
                "event": action.eventId,
                "name": action.GetName(structured),
                "indices": action.numIndices,
                "instances": action.numInstances,
                "gpu_ms": durations.get(action.eventId),
                "vertex_shader": str(pipeline.GetShader(rd.ShaderStage.Vertex)),
                "pixel_shader": str(pipeline.GetShader(rd.ShaderStage.Pixel)),
            })
        textures = [{"resource": str(tex.resourceId), "width": tex.width, "height": tex.height,
                     "mips": tex.mips, "array_size": tex.arraysize, "samples": tex.msSamp,
                     "bytes": tex.byteSize, "format": tex.format.Name()} for tex in controller.GetTextures()]
        thumbnail = capture.GetThumbnail(rd.FileType.PNG, 0)
        with open(os.environ["SINEN_PROFILE_REPLAY"] + ".png", "wb") as file:
            file.write(bytes(thumbnail.data))
        return {"capture": path, "api": str(controller.GetAPIProperties().pipelineType),
                "debug_messages": [message.description for message in controller.GetDebugMessages()],
                "draw_count": len(draws), "action_count": len(actions),
                "action_names": dict(action_names),
                "copy_count": sum(bool(action.flags & rd.ActionFlags.Copy) for action in actions),
                "resolve_count": sum(bool(action.flags & rd.ActionFlags.Resolve) for action in actions),
                "dispatch_count": sum(bool(action.flags & rd.ActionFlags.Dispatch) for action in actions),
                "counter_totals": totals,
                "counter_status": "available" if values else "unavailable: FetchCounters returned no results",
                "pipeline_state_available": any(draw["vertex_shader"] != "ResourceId::0" for draw in draw_details),
                "gpu_event_ms": {str(event): duration for event, duration in durations.items()},
                "draws": draw_details, "textures": textures,
                "texture_bytes": sum(tex["bytes"] for tex in textures),
                "note": "Replay counters are diagnostic; they are not live frame latency or vsync wait time."}
    finally:
        if controller:
            controller.Shutdown()
        capture.Shutdown()


try:
    result = replay(os.environ["SINEN_PROFILE_RDC"])
except Exception:
    result = {"error": traceback.format_exc()}
with open(os.environ["SINEN_PROFILE_REPLAY"], "w") as file:
    json.dump(result, file, indent=2)
# This script runs before the Qt main window. Exit after the replay finishes.
os._exit(0)
