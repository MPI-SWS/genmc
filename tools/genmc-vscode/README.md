# GenMC for VS Code

Run the `genmc` model checker on the currently active file and view the output inside VS Code.

## Commands

- `GenMC: Run on Active File` (`genmc.runOnActiveFile`): runs GenMC in a VS Code output channel named **GenMC**.
- `GenMC: Configure` (`genmc.configure`): opens this extension’s settings.

## Requirements

- GenMC installed and runnable as `genmc` on your `PATH`, **or** set an absolute path in `genmc.path`.

## Settings

This extension contributes the following settings:

- `genmc.path` (string): absolute path to the GenMC executable. Leave empty to use `genmc` from your `PATH`.
- `genmc.mode` (string): `verify` | `random` | `estimate`.
- `genmc.model` (string): `sc` | `tso` | `ra` | `rc11` | `imm` (translated to `--sc/--tso/--ra/--rc11/--imm`).
- `genmc.budget` (number): random mode only, passed as `--budget=<N>`.
- `genmc.scheduleSeed` (string): random mode only, passed as `--schedule-seed=<seed>`.
- `genmc.printScheduleSeed` (boolean): random mode only, adds `--print-schedule-seed`.

## What it runs

The extension prints the exact command line it is about to execute in the **GenMC** output channel, then runs:

- `--<model>`
- `--mode=<verify|random|estimate>`
- `-- <active-file>`

On completion it shows a notification and appends `[done] exited with code=...` to the output.

## Troubleshooting

- If you see “GenMC is not configured…”, set `genmc.path` or install `genmc` on your `PATH`.
- If you set `genmc.path`, it must be an absolute path and must point to an existing file.
