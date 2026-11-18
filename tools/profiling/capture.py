"""Reproduce a fixed-camera Sinen title-scene profile without editing game assets."""
import argparse
import json
import os
import re
from pathlib import Path
import subprocess
import time
import zipfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", choices=["baseline", "fps", "racing"], required=True)
    parser.add_argument("--mode", choices=["tracy", "renderdoc"], required=True)
    parser.add_argument("--name", required=True)
    parser.add_argument("--samples", type=int, default=300)
    parser.add_argument("--warmup", type=int, default=120)
    parser.add_argument("--msaa", type=int, choices=[1, 2, 4, 8], default=4)
    parser.add_argument("--no-text", action="store_true", help="Ablation: omit HUD text only in the temporary scene package")
    parser.add_argument("--exe", type=Path, default=Path("build/msvc2026-release/sinen.exe"))
    parser.add_argument("--tools", type=Path, default=Path("build/msvc2026-debug/profiling/tools/tracy"))
    parser.add_argument("--output", type=Path, default=Path("build/msvc2026-release/profiling"))
    parser.add_argument("--renderdoc", default="C:/Program Files/RenderDoc/renderdoc.dll")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    os.chdir(root)
    exe, tools = args.exe.resolve(), args.tools.resolve()
    output = (args.output / args.name).resolve()
    output.mkdir(parents=True, exist_ok=True)
    package = output / "scene.sna"
    if args.scene == "baseline":
        source = '''local sn = require("@sinen")
function draw()
    sn.Graphics.drawRect(sn.Rect.new(20, 20, 100, 80), sn.Color.new(1, 0, 0, 1))
end
'''
    else:
        source = (root / "game" / args.scene / "main.luau").read_text(encoding="utf-8-sig")
    capture_prefix = (output / "frame").as_posix()
    source += '''
-- Profiling wrapper: original title scene, fixed camera, original setup/draw.
local profileOldSetup, profileOldReady, profileOldUpdate = setup, ready, update
local profileStarted, profileFrame = false, 0
function setup()
    sn.Graphics.setMSAASampleCount(%d)
    if profileOldSetup then profileOldSetup() end
end
function ready()
    if profileOldReady then profileOldReady() end
    profileStarted = true
    print("PROFILE_READY", sn.Graphics.getBackendName(), sn.Window.size().x, sn.Window.size().y)
    if sn.RenderDoc.isAvailable() then
        assert(sn.RenderDoc.setCaptureFilePathTemplate(%s))
    end
end
function update()
    if profileOldUpdate then profileOldUpdate() end
    if not profileStarted then return end
    profileFrame += 1
    if profileFrame == %d and sn.RenderDoc.isAvailable() then
        assert(sn.RenderDoc.triggerCapture())
    end
    if profileFrame == %d then
        print("PROFILE_DONE", profileFrame, sn.RenderDoc.getNumCaptures())
        if sn.RenderDoc.getNumCaptures() > 0 then
            print("PROFILE_CAPTURE", sn.RenderDoc.getCaptureFilePath(0))
        end
        sn.Event.quit()
    end
end
''' % (args.msaa, json.dumps(capture_prefix), args.warmup + 10, args.warmup + args.samples)
    with zipfile.ZipFile(package, "w", zipfile.ZIP_DEFLATED) as archive:
        archive.writestr("sna/main.luau", source)
        if args.scene != "baseline":
            scene_dir = root / "game" / args.scene
            for path in sorted(scene_dir.rglob("*")):
                if path.is_file() and (path.suffix in [".luau", ".wav", ".png", ".jpg", ".exr", ".ktx2"]):
                    relative = path.relative_to(scene_dir)
                    if relative.as_posix() != "main.luau":
                        if args.no_text and relative.as_posix() == "hud.luau":
                            hud = path.read_text(encoding="utf-8-sig")
                            hud, replacements = re.subn(r"(?m)^([ \t]*)pass:drawText\([^\n]*\)", r"\1-- Profiling ablation: text omitted", hud)
                            if replacements != 1:
                                raise RuntimeError("Unexpected HUD text call layout")
                            archive.writestr("sna/" + relative.as_posix(), hud)
                        else:
                            archive.write(path, "sna/" + relative.as_posix())
    env = os.environ.copy()
    env.pop("SINEN_RENDERDOC_LIBRARY", None)
    if args.mode == "renderdoc":
        env["SINEN_RENDERDOC_LIBRARY"] = args.renderdoc
    flags = subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0
    collector = None
    started = time.perf_counter()
    with open(output / "capture.log", "w") as collector_log, open(output / "stdout.log", "w") as stdout, open(output / "stderr.log", "w") as stderr:
        try:
            if args.mode == "tracy":
                collector = subprocess.Popen([str(tools / "tracy-capture.exe"), "-a", "127.0.0.1", "-o", str(output / "capture.tracy"), "-f"], stdout=collector_log, stderr=subprocess.STDOUT, creationflags=flags)
                time.sleep(0.3)
            app = subprocess.run([str(exe), str(package)], cwd=output, env=env, stdout=stdout, stderr=stderr, creationflags=flags, timeout=120)
            if app.returncode:
                raise RuntimeError("Sinen exited with code %s; see %s" % (app.returncode, output / "stderr.log"))
            if collector:
                collector.wait(timeout=30)
                if collector.returncode:
                    raise RuntimeError("Tracy failed; see capture.log")
        finally:
            if collector and collector.poll() is None:
                collector.terminate()
    if "PROFILE_DONE" not in (output / "stdout.log").read_text():
        raise RuntimeError("Scene did not complete; see logs")
    if args.mode == "tracy":
        for name, options in [("zones.csv", []), ("events.csv", ["-u"]), ("self.csv", ["-e"])]:
            with open(output / name, "w") as result:
                subprocess.run([str(tools / "tracy-csvexport.exe"), *options, str(output / "capture.tracy")], stdout=result, check=True, creationflags=flags)
    metadata = dict(vars(args))
    metadata = {key: str(value) if isinstance(value, Path) else value for key, value in metadata.items()}
    metadata["wall_seconds"] = time.perf_counter() - started
    (output / "metadata.json").write_text(json.dumps(metadata, indent=2), encoding="utf-8")
    print(json.dumps({"output": str(output), "wall_seconds": metadata["wall_seconds"]}), flush=True)


if __name__ == "__main__":
    main()
