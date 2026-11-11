#!/bin/bash
set -euo pipefail

SOURCE="/Users/michael/actions-runner/irrlicht/irrlicht-1.8.5/source/Irrlicht/MacOSX/CIrrDeviceMacOSX.mm"
BACKUP="${SOURCE}.bak"

if [[ ! -f "$SOURCE" ]]; then
    echo "ERROR: Irrlicht source not found: $SOURCE" >&2
    exit 1
fi

if grep -q 'setActivationPolicy:NSApplicationActivationPolicyRegular' "$SOURCE"; then
    echo "Activation policy fix already appears to be applied."
    exit 0
fi

python3 - "$SOURCE" "$BACKUP" <<'PY'
import pathlib
import shutil
import sys

source = pathlib.Path(sys.argv[1])
backup = pathlib.Path(sys.argv[2])
text = source.read_text()

old = '''\t\tif(!CreationParams.WindowId) //load menus if standalone application
\t\t{
\t\t\t[[NSAutoreleasePool alloc] init];
\t\t\t[[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
\t\t\t[NSApp setDelegate:(id<NSApplicationDelegate>)[[[AppDelegate alloc] initWithDevice:this] autorelease]];
\t\t\t[NSBundle loadNibNamed:@"MainMenu" owner:[NSApp delegate]];
\t\t\t[NSApp finishLaunching];
\t\t}'''

new = '''\t\tif(!CreationParams.WindowId) //load menus if standalone application
\t\t{
\t\t\t[[NSAutoreleasePool alloc] init];

\t\t\tNSApplication *app = [NSApplication sharedApplication];

\t\t\t// Register this process as a regular macOS GUI application.
\t\t\t[app setActivationPolicy:NSApplicationActivationPolicyRegular];

\t\t\t[app setDelegate:(id<NSApplicationDelegate>)[[[AppDelegate alloc] initWithDevice:this] autorelease]];
\t\t\t[NSBundle loadNibNamed:@"MainMenu" owner:[NSApp delegate]];
\t\t\t[app finishLaunching];

\t\t\t// Activate after launch initialization has completed.
\t\t\t[app activateIgnoringOtherApps:YES];
\t\t}'''

if text.count(old) != 1:
    print("ERROR: Expected initialization block was not found exactly once.", file=sys.stderr)
    print("No changes made.", file=sys.stderr)
    sys.exit(1)

shutil.copy2(source, backup)
source.write_text(text.replace(old, new, 1))

print(f"Updated: {source}")
print(f"Backup:  {backup}")
PY

echo
echo "Edit complete. Review the changes with:"
echo "diff -u \"$BACKUP\" \"$SOURCE\""
