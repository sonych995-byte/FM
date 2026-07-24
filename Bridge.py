import os
import time
import json
from pathlib import Path


class Bridge:

    def __init__(self, runtime="runtime"):
        self.runtime = Path(runtime)
        self.runtime.mkdir(parents=True, exist_ok=True)

    def send(self, data, name):
        tmp_file = self.runtime / f"{name}.tmp"
        json_file = self.runtime / f"{name}.json"
        with tmp_file.open("w", encoding="utf-8") as stream:
            if isinstance(data, str):
                stream.write(data)
            else:
                json.dump(data, stream, ensure_ascii=False)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(tmp_file, json_file)

    def receive(self, name, timeout=None):
        json_file = self.runtime / f"{name}.json"
        deadline = None if timeout is None else time.monotonic() + timeout
        while True:
            if json_file.exists():
                try:
                    with json_file.open("r", encoding="utf-8") as stream:
                        raw = stream.read()
                    try:
                        data = json.loads(raw)
                    except json.JSONDecodeError:
                        data = raw
                    json_file.unlink(missing_ok=True)
                    return data
                except (OSError, json.JSONDecodeError):
                    # A writer may still be completing its atomic handoff.
                    pass
            if deadline is not None and time.monotonic() >= deadline:
                raise TimeoutError(f"Timed out waiting for {name}.json")
            time.sleep(0.01)