#!/usr/bin/env python3
"""
Batch Texture Rescaler
----------------------
Resize all supported images in a folder, optionally including subfolders.

Examples:
  python3 texture_rescaler.py ./textures
  python3 texture_rescaler.py ./textures --scale 0.5
  python3 texture_rescaler.py ./textures --scale 0.25 --method lanczos
  python3 texture_rescaler.py ./textures --width 512 --height 512 --fit contain
  python3 texture_rescaler.py ./textures --output ./textures_rescaled --recursive
  python3 texture_rescaler.py ./textures --scale 0.5 --overwrite

Dependencies:
  python3 -m pip install Pillow
"""

import argparse
import sys
from pathlib import Path
from PIL import Image, ImageOps, UnidentifiedImageError

SUPPORTED_EXTENSIONS = {
    ".png", ".jpg", ".jpeg", ".bmp", ".tga",
    ".tif", ".tiff", ".webp", ".gif"
}

RESAMPLING = {
    "nearest": Image.Resampling.NEAREST,
    "bilinear": Image.Resampling.BILINEAR,
    "bicubic": Image.Resampling.BICUBIC,
    "lanczos": Image.Resampling.LANCZOS,
    "box": Image.Resampling.BOX,
}


def parse_args():
    parser = argparse.ArgumentParser(
        description="Batch-resize image textures while preserving originals."
    )
    parser.add_argument("input", type=Path, help="Folder containing textures")
    parser.add_argument(
        "-o", "--output", type=Path,
        help="Output folder (default: <input>/rescaled)"
    )

    sizing = parser.add_mutually_exclusive_group(required=True)
    sizing.add_argument(
        "-s", "--scale", type=float,
        help="Scale factor, e.g. 0.5 for half size or 2 for double size"
    )
    sizing.add_argument("--width", type=int, help="Target width in pixels")
    parser.add_argument(
        "--height", type=int,
        help="Target height in pixels; use with --width for exact dimensions"
    )

    parser.add_argument(
        "--method", choices=RESAMPLING.keys(), default="lanczos",
        help="Resampling filter (default: lanczos)"
    )
    parser.add_argument(
        "--fit", choices=("stretch", "contain", "cover"),
        default="stretch",
        help="Exact-dimension behavior: stretch, contain, or cover (default: stretch)"
    )
    parser.add_argument(
        "--recursive", action="store_true",
        help="Process subfolders and preserve their relative folder structure"
    )
    parser.add_argument(
        "--overwrite", action="store_true",
        help="Overwrite output images if they already exist"
    )
    parser.add_argument(
        "--suffix", default="",
        help="Optional filename suffix, e.g. _half (default: none)"
    )
    parser.add_argument(
        "--format", choices=("keep", "png", "jpg", "webp", "tiff", "bmp"),
        default="keep",
        help="Output format (default: keep original format)"
    )
    parser.add_argument(
        "--quality", type=int, default=95,
        help="Quality for JPEG/WebP output, 1–100 (default: 95)"
    )
    parser.add_argument(
        "--no-optimize", action="store_true",
        help="Disable PNG/JPEG optimization"
    )
    parser.add_argument(
        "--dry-run", action="store_true",
        help="List planned conversions without writing files"
    )
    return parser.parse_args()


def get_target_size(image_size, args):
    old_w, old_h = image_size

    if args.scale is not None:
        if args.scale <= 0:
            raise ValueError("--scale must be greater than zero")
        new_w = max(1, round(old_w * args.scale))
        new_h = max(1, round(old_h * args.scale))
        return new_w, new_h

    if args.width is None or args.height is None:
        raise ValueError("Specify --scale, or provide both --width and --height")
    if args.width < 1 or args.height < 1:
        raise ValueError("--width and --height must be positive")

    return args.width, args.height


def resize_image(image, target_size, method, fit):
    resample = RESAMPLING[method]
    target_w, target_h = target_size

    if fit == "stretch":
        return image.resize((target_w, target_h), resample=resample)

    src_w, src_h = image.size
    ratio = (
        min(target_w / src_w, target_h / src_h)
        if fit == "contain"
        else max(target_w / src_w, target_h / src_h)
    )
    scaled_size = (max(1, round(src_w * ratio)), max(1, round(src_h * ratio)))
    resized = image.resize(scaled_size, resample=resample)

    if fit == "contain":
        # Keep transparency if present; otherwise use a black background.
        if "A" in resized.getbands():
            background = Image.new("RGBA", (target_w, target_h), (0, 0, 0, 0))
            resized = resized.convert("RGBA")
        else:
            background = Image.new(resized.mode, (target_w, target_h), 0)
        x = (target_w - resized.width) // 2
        y = (target_h - resized.height) // 2
        background.paste(resized, (x, y), resized if "A" in resized.getbands() else None)
        return background

    # cover: resize to fill, then crop the center.
    left = (resized.width - target_w) // 2
    top = (resized.height - target_h) // 2
    return resized.crop((left, top, left + target_w, top + target_h))


def output_path_for(source, root, output_root, args):
    relative = source.relative_to(root)
    out_dir = output_root / relative.parent
    fmt = args.format
    extension = source.suffix.lower() if fmt == "keep" else f".{fmt}"
    if fmt == "jpg":
        extension = ".jpg"
    return out_dir / f"{source.stem}{args.suffix}{extension}"


def save_image(image, destination, args, original_format):
    destination.parent.mkdir(parents=True, exist_ok=True)
    fmt = args.format
    if fmt == "keep":
        fmt = original_format.lower()
    if fmt in ("jpg", "jpeg"):
        fmt = "jpeg"
        if image.mode not in ("RGB", "L"):
            # JPEG does not support alpha. Composite transparency on black.
            if "A" in image.getbands():
                rgba = image.convert("RGBA")
                background = Image.new("RGB", rgba.size, (0, 0, 0))
                background.paste(rgba, mask=rgba.getchannel("A"))
                image = background
            else:
                image = image.convert("RGB")
    elif fmt == "png" and image.mode not in ("RGB", "RGBA", "L", "LA", "P"):
        image = image.convert("RGBA" if "A" in image.getbands() else "RGB")
    elif fmt == "webp" and image.mode not in ("RGB", "RGBA", "L"):
        image = image.convert("RGBA" if "A" in image.getbands() else "RGB")

    options = {}
    if fmt in ("jpeg", "webp"):
        options["quality"] = max(1, min(100, args.quality))
    if not args.no_optimize and fmt in ("png", "jpeg"):
        options["optimize"] = True

    image.save(destination, format=fmt.upper(), **options)


def main():
    args = parse_args()

    if args.scale is None and args.height is None:
        print("Error: --width must be used with --height.", file=sys.stderr)
        return 2
    if args.height is not None and args.width is None:
        print("Error: --height must be used with --width.", file=sys.stderr)
        return 2

    root = args.input.expanduser().resolve()
    if not root.is_dir():
        print(f"Error: input folder does not exist: {root}", file=sys.stderr)
        return 2

    output_root = (
        args.output.expanduser().resolve()
        if args.output
        else root / "rescaled"
    )

    if output_root == root:
        print("Error: output folder cannot be the input folder itself.", file=sys.stderr)
        return 2

    iterator = root.rglob("*") if args.recursive else root.iterdir()
    files = sorted(
        p for p in iterator
        if p.is_file()
        and p.suffix.lower() in SUPPORTED_EXTENSIONS
        and output_root not in p.parents
    )

    if not files:
        print(f"No supported image files found in {root}")
        return 0

    print(f"Input:  {root}")
    print(f"Output: {output_root}")
    print(f"Images: {len(files)}")
    print(f"Filter: {args.method}")
    print()

    success = skipped = failed = 0

    for source in files:
        destination = output_path_for(source, root, output_root, args)
        if destination.exists() and not args.overwrite:
            print(f"SKIP (exists): {destination}")
            skipped += 1
            continue

        try:
            with Image.open(source) as original:
                original_format = original.format or source.suffix.lstrip(".")
                # Apply EXIF orientation for photographs.
                original = ImageOps.exif_transpose(original)
                target_size = get_target_size(original.size, args)

                # Animated GIF: resize all frames and preserve frame timing.
                if getattr(original, "is_animated", False) and args.format in ("keep", "gif"):
                    frames = []
                    durations = []
                    for frame in ImageSequence.Iterator(original):
                        frame = frame.convert("RGBA")
                        frames.append(resize_image(frame, target_size, args.method, args.fit))
                        durations.append(frame.info.get("duration", original.info.get("duration", 100)))
                    if args.dry_run:
                        print(f"DRY RUN: {source.name} {original.size} -> {target_size}")
                    else:
                        destination.parent.mkdir(parents=True, exist_ok=True)
                        frames[0].save(
                            destination, save_all=True, append_images=frames[1:],
                            duration=durations, loop=original.info.get("loop", 0),
                            optimize=not args.no_optimize, disposal=2
                        )
                        print(f"OK: {source.name} {original.size} -> {target_size}")
                else:
                    resized = resize_image(original, target_size, args.method, args.fit)
                    if args.dry_run:
                        print(f"DRY RUN: {source.name} {original.size} -> {target_size}")
                    else:
                        save_image(resized, destination, args, original_format)
                        print(f"OK: {source.name} {original.size} -> {target_size}")
                if not args.dry_run:
                    success += 1
        except (OSError, ValueError, UnidentifiedImageError) as exc:
            print(f"ERROR: {source}: {exc}")
            failed += 1

    print()
    print(f"Finished: {success} written, {skipped} skipped, {failed} failed.")
    return 1 if failed else 0


if __name__ == "__main__":
    # Import here to keep the non-animated path simple.
    from PIL import ImageSequence
    raise SystemExit(main())
