# Sound Generator Web UI

A small local web interface for `soundgen.py`, including one-click WAV previews.

## Files

Keep this `soundgen_web` folder beside `soundgen.py`:

```text
project/
├── soundgen.py
└── soundgen_web/
    ├── server.py
    ├── index.html
    └── README.md
```

## Run

From the `soundgen_web` directory:

```bash
python3 server.py
```

Open http://127.0.0.1:8765 in your browser. Stop the server with Ctrl+C.

If `soundgen.py` is elsewhere, set its path before starting the server:

```bash
SOUNDGEN_SCRIPT=/absolute/path/to/soundgen.py python3 server.py
```

Optional port override: `SOUNDGEN_UI_PORT=8766 python3 server.py`. The server binds to localhost only.

## Previewing sounds

Select a preset and click **Preview selected sound**. The server renders a temporary WAV, streams it to the browser, and the built-in audio player plays it. Preview duration is limited to 15 seconds and the preview WAV is not saved in the output folder. Use **Generate selected preset** to save a regular output file.
