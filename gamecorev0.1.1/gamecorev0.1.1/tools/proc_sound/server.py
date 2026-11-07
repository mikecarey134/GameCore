#!/usr/bin/env python3
"""Local web UI bridge for soundgen.py. Uses only Python's standard library."""
import json
import os
import subprocess
import sys
import threading
import tempfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlparse

BASE_DIR = Path(__file__).resolve().parent
print("Base DIR :" ,BASE_DIR)
SOUNDGEN = Path(os.environ.get("SOUNDGEN_SCRIPT", BASE_DIR/ "soundgen.py")).expanduser().resolve()
WEB_DIR = BASE_DIR
LOCK = threading.Lock()
STATE = {"busy": False, "message": "Ready", "lines": [], "offset": 0, "process": None}


def load_catalog():
    if not SOUNDGEN.is_file():
        raise FileNotFoundError(f"soundgen.py not found: {SOUNDGEN}. Set SOUNDGEN_SCRIPT if it is elsewhere.")
    # The script defines ASSET_CATALOG; import it by path so the UI always reflects the actual catalog.
    import importlib.util
    spec = importlib.util.spec_from_file_location("soundgen_module", SOUNDGEN)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return {category: sorted(set(names.split())) for category, names in module.ASSET_CATALOG.items()}


def append_line(line):
    with LOCK:
        STATE["lines"].append(line)
        if len(STATE["lines"]) > 2500:
            del STATE["lines"][:1000]


def run_job(command):
    try:
        append_line("$ " + " ".join(subprocess.list2cmdline([x]) for x in command))
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                   text=True, bufsize=1, cwd=str(SOUNDGEN.parent))
        with LOCK:
            STATE["process"] = process
        assert process.stdout is not None
        for line in process.stdout:
            append_line(line.rstrip())
        code = process.wait()
        message = "Finished successfully" if code == 0 else f"Generator exited with code {code}"
        append_line(message)
    except Exception as exc:
        code = -1
        message = f"ERROR: {exc}"
        append_line(message)
    finally:
        with LOCK:
            STATE["busy"] = False
            STATE["message"] = message
            STATE["process"] = None


class Handler(BaseHTTPRequestHandler):
    server_version = "SoundGenUI/1.0"

    def send_json(self, data, status=200):
        body = json.dumps(data).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        path = urlparse(self.path).path
        if path == "/api/config":
            try:
                self.send_json({"categories": load_catalog(), "script": str(SOUNDGEN)})
            except Exception as exc:
                self.send_json({"error": str(exc)}, 500)
            return
        if path == "/api/status":
            with LOCK:
                new_lines = STATE["lines"][STATE["offset"]:]
                STATE["offset"] = len(STATE["lines"])
                result = {"busy": STATE["busy"], "message": STATE["message"], "new_lines": new_lines}
            self.send_json(result)
            return
        if path in ("/", "/index.html"):
            try:
                body = (WEB_DIR / "index.html").read_bytes()
                self.send_response(200)
                self.send_header("Content-Type", "text/html; charset=utf-8")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)
            except OSError as exc:
                self.send_error(500, str(exc))
            return
        self.send_error(404)

    def do_POST(self):
        path = urlparse(self.path).path
        if path == "/api/preview":
            self.handle_preview()
            return
        if path != "/api/generate":
            self.send_error(404)
            return
        try:
            length = int(self.headers.get("Content-Length", "0"))
            if length < 1 or length > 20000:
                raise ValueError("Invalid request body size")
            payload = json.loads(self.rfile.read(length))
            categories = load_catalog()
            mode = payload.get("mode")
            category = payload.get("category", "")
            preset = payload.get("preset", "")
            duration = float(payload.get("duration", 1))
            sample_rate = int(payload.get("sample_rate", 44100))
            frequency = float(payload.get("frequency", 220))
            volume = float(payload.get("volume", 1))
            count = int(payload.get("random_count", 25))
            seed = payload.get("seed")
            output = str(payload.get("output", "generated_sounds")).strip()
            if not output:
                output = "generated_sounds"
            if duration <= 0 or duration > 600:
                raise ValueError("Duration must be greater than 0 and no more than 600 seconds")
            if sample_rate < 8000 or sample_rate > 192000:
                raise ValueError("Sample rate must be between 8000 and 192000 Hz")
            if frequency <= 0 or frequency > 96000:
                raise ValueError("Frequency must be greater than 0 and no more than 96000 Hz")
            if not 0 <= volume <= 1:
                raise ValueError("Volume must be between 0 and 1")
            if count < 1 or count > 1000:
                raise ValueError("Random count must be between 1 and 1000")
            if seed is not None:
                seed = int(seed)
            command = [sys.executable, str(SOUNDGEN), "--duration", str(duration),
                       "--sample-rate", str(sample_rate), "--frequency", str(frequency),
                       "--volume", str(volume)]
            if seed is not None:
                command += ["--seed", str(seed)]
            if mode == "preset":
                if preset not in {name for names in categories.values() for name in names}:
                    raise ValueError("Unknown preset selected")
                output_path = Path(output).expanduser()
                if not output_path.is_absolute():
                    output_path = (SOUNDGEN.parent / output_path).resolve()
                output_path.mkdir(parents=True, exist_ok=True)
                command += [preset, "--output", str(output_path / f"{preset}.wav")]
            elif mode in ("category", "random", "all"):
                output_path = Path(output).expanduser()
                if not output_path.is_absolute():
                    output_path = (SOUNDGEN.parent / output_path).resolve()
                output_path.mkdir(parents=True, exist_ok=True)
                command += ["--output-dir", str(output_path)]
                if mode == "category":
                    if category not in categories:
                        raise ValueError("Unknown category selected")
                    command += ["--batch", category]
                elif mode == "random":
                    command += ["--random", str(count)]
                else:
                    command += ["--all"]
            else:
                raise ValueError("Unknown generation mode")
            with LOCK:
                if STATE["busy"]:
                    self.send_json({"error": "A generation job is already running"}, 409)
                    return
                STATE["busy"] = True
                STATE["message"] = "Generating audio…"
                STATE["lines"] = []
                STATE["offset"] = 0
            thread = threading.Thread(target=run_job, args=(command,), daemon=True)
            thread.start()
            self.send_json({"message": "Generation started"}, 202)
        except (ValueError, TypeError, json.JSONDecodeError, OSError) as exc:
            self.send_json({"error": str(exc)}, 400)

    def handle_preview(self):
        """Generate one short WAV and stream it directly to the browser."""
        try:
            length = int(self.headers.get("Content-Length", "0"))
            if length < 1 or length > 20000:
                raise ValueError("Invalid request body size")
            payload = json.loads(self.rfile.read(length))
            categories = load_catalog()
            preset = str(payload.get("preset", ""))
            if preset not in {name for names in categories.values() for name in names}:
                raise ValueError("Choose a valid preset before previewing")

            duration = float(payload.get("duration", 1))
            sample_rate = int(payload.get("sample_rate", 44100))
            frequency = float(payload.get("frequency", 220))
            volume = float(payload.get("volume", 1))
            seed = payload.get("seed")
            # Previews are deliberately short so accidental clicks cannot create long jobs.
            if duration <= 0 or duration > 15:
                raise ValueError("Preview duration must be greater than 0 and no more than 15 seconds")
            if sample_rate < 8000 or sample_rate > 192000:
                raise ValueError("Sample rate must be between 8000 and 192000 Hz")
            if frequency <= 0 or frequency > 96000:
                raise ValueError("Frequency must be greater than 0 and no more than 96000 Hz")
            if not 0 <= volume <= 1:
                raise ValueError("Volume must be between 0 and 1")
            if seed is not None:
                seed = int(seed)

            with tempfile.TemporaryDirectory(prefix="soundgen-preview-") as temp_dir:
                wav_path = Path(temp_dir) / "preview.wav"
                command = [sys.executable, str(SOUNDGEN), "--duration", str(duration),
                           "--sample-rate", str(sample_rate), "--frequency", str(frequency),
                           "--volume", str(volume)]
                if seed is not None:
                    command += ["--seed", str(seed)]
                command += [preset, "--output", str(wav_path)]
                try:
                    result = subprocess.run(command, cwd=str(SOUNDGEN.parent),
                                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                            text=True, timeout=60, check=False)
                except subprocess.TimeoutExpired:
                    self.send_json({"error": "Preview timed out after 60 seconds"}, 504)
                    return
                if result.returncode != 0:
                    detail = (result.stdout or "Generator failed without output").strip()
                    self.send_json({"error": detail[-4000:]}, 500)
                    return
                if not wav_path.is_file():
                    self.send_json({"error": "Generator finished but did not create a WAV file"}, 500)
                    return
                audio = wav_path.read_bytes()

            self.send_response(200)
            self.send_header("Content-Type", "audio/wav")
            self.send_header("Content-Length", str(len(audio)))
            self.send_header("Cache-Control", "no-store")
            self.send_header("Content-Disposition", 'inline; filename="soundgen-preview.wav"')
            self.end_headers()
            self.wfile.write(audio)
        except (ValueError, TypeError, json.JSONDecodeError, OSError) as exc:
            self.send_json({"error": str(exc)}, 400)

    def log_message(self, fmt, *args):
        # Keep the terminal quiet apart from startup messages.
        pass


def main():
    if not SOUNDGEN.is_file():
        print(f"ERROR: soundgen.py not found at {SOUNDGEN}", file=sys.stderr)
        print("Set SOUNDGEN_SCRIPT=/path/to/soundgen.py if needed.", file=sys.stderr)
        raise SystemExit(1)
    host, port = "127.0.0.1", int(os.environ.get("SOUNDGEN_UI_PORT", "8765"))
    server = ThreadingHTTPServer((host, port), Handler)
    print(f"Sound Generator UI: http://{host}:{port}")
    print(f"Using generator: {SOUNDGEN}")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server…")
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
