# RPG_RT fork & version support — specification series

RPG Maker 2000 and 2003 ship a runtime interpreter, `RPG_RT.exe`. Over two decades the
community produced a large family of **binary forks and patches** of that runtime — some
adding whole scripting systems and event commands, some changing low-level behavior, some
merely re-skinning or translating the engine. EasyRPG Player reimplements the runtime, and
to run the games that depend on these forks it has to reproduce the forks' behavior.

This directory is the **source of truth** for that effort. The goal is deliberately ambitious:

> Each spec should be complete enough that a competent developer could reconstruct support
> for the fork in an EasyRPG-class engine from the document alone — turning a
> "supports no forks" player into a "supports this fork" player without reading the original
> binary. The reverse-engineering captured here matters more than any one code change.

These documents live in **our fork** and are written from verifiable evidence (binary RE,
file-format inspection, primary docs). They are not upstream EasyRPG positions; see
[Upstreaming](#upstreaming) below.

## How to read a spec

Every fork spec follows [`TEMPLATE.md`](TEMPLATE.md): identity → target builds → version
lineage → detection → event commands → modified baseline commands → file formats → runtime
behavior → EasyRPG support matrix → test assets → open questions → references. Claims carry a
source: an external link, a `file:line` anchor into this repo (`src/…`) or bundled liblcf
(`lib/liblcf/…`), or a binary handler address from the RE notes. Unverified claims are marked
`(UNVERIFIED)`, `(RE-pending)`, or `(hypothesis)`.

## Registry

Priority order is driven by the immediate goal — running **Beloved Rapture** (RPG Maker 2003
v1.12a + Maniac Patch build 220325). See [`../games/beloved-rapture.md`](../games/beloved-rapture.md).

### Major runtime forks (add event commands / scripting)

| Fork | Engine | Adds | Detection | EasyRPG | Spec |
|---|---|---|---|---|---|
| **Maniac Patch** (マニアクスパッチ) | RM2k3 v1.12a (Steam) | ~30 event commands (3001–3033), string variables, expression VM, global save, custom resolution, font loading, battle hooks | `accord.dll`; VERSIONINFO `Maniacs, vNNNNNN` | Deep (partial) | [maniac-patch.md](maniac-patch.md) · [commands](maniac-patch-commands.md) · [formats](maniac-patch-fileformats.md) |
| **DynRPG** | RM2k3 v1.08 | Native C++ DLL plugin SDK; comment-command API; QuickPatches | `dynloader.dll` | HLE-only (2 plugins) | [dynrpg.md](dynrpg.md) |
| **Destiny Patch** | RM2k 1.05/1.07 | `DestinyScript` in `$`-comment commands; `Destiny.dll` | `Destiny.dll` | Stub | [destiny.md](destiny.md) |
| **PowerMode 2003** (Mega Patch 2003) | RM2k3 v1.09 | `V[1..8]` control registers; audio extensions | `warp.dll` | Detect + partial (master) | [powermode2003.md](powermode2003.md) |
| **Ineluki's Key Patch** | RM2k/2k3 | Key input + script hooks via `*.script.wav`/`*.link.wav`; replaces `harmony.dll` | `harmony.dll` | Most ini-script cmds | [ineluki-key-patch.md](ineluki-key-patch.md) |

### Behavioral binary patches (no new command codes)

Small patches — many catalogued with per-build byte offsets in the
[Makerpendium Patch DB](https://dev.makerpendium.de/docs/patch_db/main-en.htm) — that change a
variable/switch protocol or a fixed engine behavior. EasyRPG implements several as
`Game_ConfigGame` flags. See [runtime-micro-patches.md](runtime-micro-patches.md):
AntiLagSwitch, DirectMenu, EncounterRandomnessAlert, MonSca(Plus), EXPlus, GuardRevamp,
CommonThisEvent, PicUnlock, PicPointer, BetterAEP, StatDelimiter, the JP "RPG2k3 commands in
RPG2k games"/in-battle-Call-Event LDB hacks, and more.

### Engine identification (not features)

- [official-versions.md](official-versions.md) — the official RM2k (1.00–1.62) and RM2k3
  (1.00–1.12a) version matrix and the runtime-behavior differences EasyRPG keys off of.
- [bootlegs-translations.md](bootlegs-translations.md) — Don Miguel / RPG Advocate / Hellsoft
  / national translations, logo CRC identification, and games special-cased in `exe_reader.cpp`.

### EasyRPG's own extensions

- [easyrpg-extensions.md](easyrpg-extensions.md) — EasyRPG's own command codes (2002–2058),
  `0xC8+` chunk IDs, the per-savegame `EasyRpg_SetInterpreterFlag` mechanism, the `--patch-*`
  CLI surface, `EasyRPG.ini [Patch]`, and how patch selection / auto-detection / override work.

## Architecture notes for adding a fork (orientation)

- **Patch configuration**: `src/game_config_game.h` (`Game_ConfigGame`, ~25 `patch_*`
  `ConfigParam`s). Read from `EasyRPG.ini` section `[Patch]` and `--patch-*` CLI flags. Setting
  any patch explicitly sets `patch_override`, which disables auto-detection. `RPG_RT.ini` is
  **not** consulted for patch selection. (Inventory: `/home/john/research/easyrpg_patch_inventory.md`.)
- **Auto-detection**: `src/player.cpp` (~`:740`–`:887`) combines `RPG_RT.exe` analysis
  (`src/exe_reader.cpp`: VERSIONINFO, PE section sizes, logo CRCs, byte signatures) with DLL
  sniffing and LDB fallbacks.
- **Command dispatch**: three nested switches — `src/game_interpreter.cpp` (shared),
  `src/game_interpreter_map.cpp`, `src/game_interpreter_battle.cpp` — each `default:`-returns,
  so unknown command codes are silently skipped. This is the natural seam for a fork
  command-remapping layer (see [easyrpg-extensions.md](easyrpg-extensions.md) and the design
  note when implemented).
- **liblcf** (`lib/liblcf`): CSV-driven codegen (`src/generated/`). Event-command codes are
  preserved losslessly even when unknown; unknown *chunks* are dropped on rewrite. Maniac and
  EasyRpg fields are real, named entries in the generated model.

## Upstreaming

EasyRPG maintainers verify fork behavior against the real runtimes, consult the original patch
authors, require tester reports, and **reject AI-generated contributions**. Everything here is
RE-backed and lives in our fork first. Any upstream PR must be human-reviewed, evidence-cited,
and accompanied by tests / tester reports.

## Provenance

These specs are distilled from a research corpus kept outside the repo:
`/home/john/research/` (web research + EasyRPG inventory + synthesis) and `/home/john/re/`
(Ghidra RE of `BelovedRapture.exe`). The test-game corpus lives at `c:\rg\easyrpg_library`.
