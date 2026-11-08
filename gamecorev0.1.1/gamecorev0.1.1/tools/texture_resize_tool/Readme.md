# Texture Rescaler

A Python batch image rescaling tool for game textures and other image assets. Resize an entire folder of textures using scaling factors or target dimensions, with support for multiple image formats and resampling filters.

## Features

- Batch-resize images in a directory.
- Optional recursive processing of subdirectories.
- Scale images by a factor, such as 50% or 25%.
- Resize images to exact pixel dimensions.
- Choose from Lanczos, nearest-neighbor, bilinear, bicubic, and box resampling.
- Preserve directory structure when processing recursively.
- Keep original files untouched by default.
- Preserve PNG transparency when saving as PNG.
- Support PNG, JPEG, BMP, TGA, TIFF, WebP, and GIF image formats.
- Preview planned changes with dry-run mode.
- Skip existing output files or overwrite them when requested.
- Select an output format and adjust JPEG/WebP quality.

## Requirements

- Python 3
- Pillow (Python Imaging Library)

## Installation

### Kubuntu, Ubuntu, and Debian

Install Pillow through APT:

```bash
sudo apt update
sudo apt install python3-pil
```

Verify the installation:

```bash
python3 -c "from PIL import Image; print(Image.__version__)"
```

No additional Python packages are required for the standard rescaling workflow.

## Usage

Run the script from a terminal:

```bash
python3 texture_rescaler.py INPUT_FOLDER OPTIONS
```

For example:

```bash
python3 texture_rescaler.py ./textures --scale 0.5
```

By default, resized images are written to a `rescaled` subdirectory inside the input folder.

## Command-Line Options

| Option | Description |
|---|---|
| `input` | Source folder containing images |
| `-o`, `--output` | Specify the output folder |
| `-s`, `--scale` | Scale factor, such as `0.5` or `0.25` |
| `--width` | Target width in pixels |
| `--height` | Target height in pixels |
| `--method` | Resampling filter |
| `--fit` | Dimension handling: `stretch`, `contain`, or `cover` |
| `--recursive` | Include subdirectories |
| `--overwrite` | Overwrite existing output files |
| `--suffix` | Add a suffix to output filenames |
| `--format` | Output format: `keep`, `png`, `jpg`, `webp`, `tiff`, or `bmp` |
| `--quality` | JPEG/WebP quality from 1 to 100 |
| `--no-optimize` | Disable supported output optimization |
| `--dry-run` | Show planned changes without writing files |

Use `--help` to see the available arguments:

```bash
python3 texture_rescaler.py --help
```

## Examples

### Reduce textures to 50%

```bash
python3 texture_rescaler.py ./textures --scale 0.5
```

### Reduce textures to 25%

```bash
python3 texture_rescaler.py ./textures --scale 0.25
```

### Double texture resolution

```bash
python3 texture_rescaler.py ./textures --scale 2
```

### Resize to 512 × 512 pixels

```bash
python3 texture_rescaler.py ./textures --width 512 --height 512
```

When using `--width` and `--height`, both dimensions must be supplied.

### Process a complete directory tree

```bash
python3 texture_rescaler.py ./assets/textures \
    --scale 0.5 \
    --recursive \
    --output ./assets/textures_rescaled
```

The tool processes supported images in nested directories and preserves their relative folder structure.

### Use nearest-neighbor filtering

```bash
python3 texture_rescaler.py ./textures \
    --scale 0.5 \
    --method nearest
```

Useful for pixel art and textures where sharp pixel boundaries should be retained.

### Use Lanczos filtering

```bash
python3 texture_rescaler.py ./textures \
    --scale 0.5 \
    --method lanczos
```

Lanczos is the default and is a good general-purpose choice for detailed game textures.

### Change output format

Convert output textures to PNG:

```bash
python3 texture_rescaler.py ./textures \
    --scale 0.5 \
    --format png
```

### Add a filename suffix

```bash
python3 texture_rescaler.py ./textures \
    --scale 0.5 \
    --suffix _half
```

For example, `wall.png` becomes `wall_half.png`.

### Preview changes before processing

```bash
python3 texture_rescaler.py ./textures \
    --scale 0.5 \
    --recursive \
    --dry-run
```

This displays the planned resizing operations without writing output images.

### Overwrite existing output files

```bash
python3 texture_rescaler.py ./textures \

