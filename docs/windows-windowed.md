# Running Fallout 2 CE in a Window on Windows

These commands configure the GOG Fallout 2 installation for windowed mode and run the locally built Fallout 2 CE executable.

The examples use:

```text
C:\dev\fallout2-ce
C:\Program Files (x86)\GOG Galaxy\Games\Fallout 2
```

The configuration file is named `f2_res.ini`, not `fres.ini`.

## Configure windowed mode

Open PowerShell and run:

```powershell
$gameDir = 'C:\Program Files (x86)\GOG Galaxy\Games\Fallout 2'

# Create backups without overwriting an existing backup.
if (-not (Test-Path "$gameDir\ddraw.ini.bk")) {
    Copy-Item "$gameDir\ddraw.ini" "$gameDir\ddraw.ini.bk" -ErrorAction Stop
}

if (-not (Test-Path "$gameDir\f2_res.ini.bk")) {
    Copy-Item "$gameDir\f2_res.ini" "$gameDir\f2_res.ini.bk" -ErrorAction Stop
}
```

Edit `f2_res.ini` so its `[MAIN]` section contains:

```ini
[MAIN]
SCR_WIDTH=1024
SCR_HEIGHT=768
WINDOWED=1
```

`ddraw.ini` does not need to be changed for Fallout 2 CE. Leave its Sfall graphics mode unchanged; CE creates its window through SDL2.

## Build Fallout 2 CE

From the repository root:

```powershell
cd C:\dev\fallout2-ce
cmake --preset windows-x64
cmake --build --preset windows-x64-debug --parallel 4
```

The resulting executable is:

```text
C:\dev\fallout2-ce\out\build\windows-x64\Debug\fallout2-ce.exe
```

## Copy the executable to the game directory

```powershell
$gameDir = 'C:\Program Files (x86)\GOG Galaxy\Games\Fallout 2'

Copy-Item `
    'C:\dev\fallout2-ce\out\build\windows-x64\Debug\fallout2-ce.exe' `
    "$gameDir\fallout2-ce.exe" `
    -Force
```

## Run the game

Run it with the Fallout 2 directory as the working directory:

```powershell
cd 'C:\Program Files (x86)\GOG Galaxy\Games\Fallout 2'
.\fallout2-ce.exe
```

This lets the executable find:

```text
ddraw.ini
f2_res.ini
```

## Restore the original configuration

If you need to undo the windowed-mode change:

```powershell
$gameDir = 'C:\Program Files (x86)\GOG Galaxy\Games\Fallout 2'

Copy-Item "$gameDir\ddraw.ini.bk" "$gameDir\ddraw.ini" -Force
Copy-Item "$gameDir\f2_res.ini.bk" "$gameDir\f2_res.ini" -Force
```

The CE executable reads `ddraw.ini` from the executable directory and `f2_res.ini` from the current working directory, so copying the executable into the Fallout 2 directory and launching it from there is the simplest setup.
