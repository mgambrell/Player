# Fork & version selection — design analysis

This note answers the architecture question behind the whole effort: **does EasyRPG have
adequate machinery to select among many forks *and* many builds/versions of each, or must we
build it?** It evaluates the existing framework, the specific worry about *command-ID reuse
across forks*, and what (if anything) is the minimal conservative addition. Conclusions feed the
implementation work; nothing here is a speculative build.

TL;DR:
1. **Fork selection is essentially solved** by EasyRPG's existing detect → flag → dispatch
   pipeline; it selects Maniac Patch correctly for Beloved Rapture end-to-end.
2. **A command-ID remapping layer is *not* needed** for the known fork set — forks do not
   collide on command IDs (disjoint numeric ranges + Comment-sigil disambiguation + per-game
   mutual exclusivity). Where a remapper *would* slot in, if a future fork ever forces it, is
   identified below — but building it now would be solving a problem we do not have.
3. **The one real gap is per-build capability gating** *within* a fork (Maniac Patch builds
   diverge). EasyRPG already *detects* the build but stores it in an overloaded field and barely
   uses it. The minimal, conservative addition is a clean build accessor + a small capability
   table — proposed here, to be implemented only as concrete builds force specific gates.

---

## 1. The existing selection pipeline

Three layers, all already present (anchors into our tree):

**(a) Detection** — `src/player.cpp` `Player::Init`/engine-detect (~`:782`–`:887`) +
`src/exe_reader.cpp`:
- **Fork** by bundled DLL (`accord.dll`→Maniac, `dynloader.dll`→DynRPG, `harmony.dll`→KeyPatch,
  `Destiny.dll`→Destiny, `warp.dll`→PowerMode) and by `RPG_RT.exe` analysis (VERSIONINFO,
  PE-section sizes, logo CRC32, byte signatures).
- **Engine + version** by `EXEReader::FileInfo::GetEngineType` (`exe_reader.cpp:483`):
  VERSIONINFO product version, logo count, CODE/CHERRY/GEEP section sizes; for Maniac Patch it
  parses the `Maniacs, vNNNNNN` UTF-16 string into a build number (`exe_reader.cpp:391-397`).

**(b) Configuration** — `src/game_config_game.h` `Game_ConfigGame`: ~25 `patch_*` `ConfigParam`s.
Set from `EasyRPG.ini [Patch]`, `--patch-*` CLI flags, or detection. Any explicit setting sets
`patch_override`, disabling auto-detection (`game_config_game.cpp` + `player.cpp:821`). `RPG_RT.ini`
is **not** consulted for patch selection. Per-savegame overrides exist via
`EasyRpg_SetInterpreterFlag` (`game_interpreter.cpp:5675`) → `StateFlags::patch_maniac_on/off`.

**(c) Dispatch** — three `switch(com.code)` interpreters: `game_interpreter.cpp` (shared,
~`:611`), `game_interpreter_map.cpp`, `game_interpreter_battle.cpp`; each `default:`-returns, so
unknown codes are silently skipped. Handlers self-gate with `if (!Player::IsPatchManiac()) return
true;`. (`Player::IsPatchManiac()` = `patch_maniac.Get() > 0`, `player.h:528`.)

This pipeline is why Beloved Rapture "just works" at the selection level: detected
`2k3 / MajorUpdated / English`, `Patch configuration: Maniac Patch`, and the 3001–3032 handlers
activate.

## 2. Do forks collide on command IDs? (the remapper question)

The worry: different forks reuse the same command IDs for different meanings, so a single global
dispatch can't tell them apart without remapping. **In practice they do not collide**, for three
independent reasons:

**(a) Disjoint numeric ranges.** The fork command spaces are non-overlapping
(`lib/liblcf/src/generated/lcf/rpg/eventcommand.h`):

| Range | Owner |
|---|---|
| 10110–~22420 | Baseline RM2k/2k3 event commands |
| 5001–5005 | Official RM2k3 1.10+ (ExitGame/ATB/fullscreen/video) |
| **2002–2058** | **EasyRPG** own extensions (`EasyRpg_*`) |
| **3001–3032** | **Maniac Patch** (`Maniac_*`) |

EasyRPG-2xxx and Maniac-3xxx never overlap, and neither overlaps baseline. A game that is *both*
EasyRPG-extended and Maniac would still dispatch each command unambiguously.

**(b) Sigil disambiguation for the Comment-based forks.** DynRPG and Destiny add **no new command
codes**. They encode their scripting inside ordinary `Comment` commands and disambiguate by a
leading sigil — `@` (DynRPG / EasyRPG plugin calls) vs `$` (DestinyScript). EasyRPG already
routes on the sigil + the relevant patch flag (`game_interpreter.cpp` `HandleDynRpgScript`
~`:2057`). PowerMode and Key Patch likewise piggyback on existing commands/variable protocols,
not new codes.

**(c) Per-game mutual exclusivity.** A shipped game uses exactly one runtime fork (it embeds one
patched `RPG_RT.exe`). Detection picks that one; the others' flags stay off. There is no scenario
in the known corpus where two forks' overlapping IDs are simultaneously live.

**Conclusion:** a command-ID **remapping layer is unnecessary** for the current fork set. The
existing range-disjointness + sigil + flag-gating *is* the disambiguation mechanism.

### Where a remapper *would* go, if ever forced

If some future fork redefined a **baseline** command ID to mean something else (none known to),
or two simultaneously-active forks ever collided, the clean seam is a **per-fork code-normalize
step at the top of dispatch**: translate the incoming `com.code` through a small
fork-selected lookup to a canonical internal code *before* the `switch`, in the three
interpreters named in §1(c). This is a ~localized change (one indirection per interpreter) and
should be added **only** when a concrete colliding fork appears — documented here so the option
is on record, not built speculatively (keeps with the project's conservative-diff guidance).

## 3. The real gap: per-build capability gating within a fork

Forks evolve. Maniac Patch builds differ in ways that matter to faithful emulation — e.g.
(from the RE and the [build registry](maniac-patch-builds.md)):

- command **3029** (ControlTextProcessing) exists in `PE220325` but not `PE211010`;
- SetGameOption **op 7** (FaceSet cell size) is `PE220325`-only;
- the **64-bit `PE241028`** build changes integer/variable ranges and adds `js` scripting;
- the `im`/`pf` page-condition-refresh model and `RPG_RT.rs1` encryption are build-keyed.

### What EasyRPG already has — and its overloaded-field smell

Detection *does* parse the build number, but it is stored into the **same** `int patch_maniac`
field that also encodes the manual on/off/mode:

- `patch_maniac == 0` → off; `> 0` → on (`IsPatchManiac`).
- `--patch-maniac 2` → `2` = "on, but do **not** widen variable ranges to 32-bit"
  (`game_constants.cpp:28/37`, help text `player.cpp:1526`).
- **Auto-detect** stores the **build number**: `patch_maniac.Set(maniac_patch_version)`
  (`player.cpp:822`) — so for Beloved Rapture `patch_maniac.Get()` is literally **220325**.

So the field means *off / mode-2 / a six-digit build* depending on how it was set. The build
number is therefore *available* in the common (auto-detected) case but is **not used for gating**:
the one "since 240423" remark (`game_interpreter.cpp:2843`) is only a comment — the actual
behavior gates on a parameter bit, not the build. Manual `--patch-maniac 1|2` discards build
information entirely.

### Proposed minimal, conservative addition

Implement *only as concrete builds force specific gates*; design it now so each gate is one line:

1. **A dedicated accessor** `Player::ManiacBuild()` → the detected build number (`YYMMDD`, e.g.
   `220325`), or `0` when unknown / manually forced. Internally it reads the existing detected
   value; it does **not** change `patch_maniac`'s storage, so nothing regresses. (If desired
   later, split detection into `patch_maniac` = mode {0,1,2} + a separate `maniac_build` int to
   remove the overload — a clean but larger change; not required for BR.)
2. **A tiny capability predicate**, e.g. `Player::ManiacHasFaceSizeOption()` ≡
   `ManiacBuild() == 0 || ManiacBuild() >= 220325` (unknown ⇒ permissive, matching today's
   "always-on" stance and avoiding regressions on manually-forced games). Gates read as
   `if (Player::ManiacHasFaceSizeOption()) …`.
3. **Drive the gate values from the spec** — the per-command "Since" column in
   [maniac-patch-commands.md](maniac-patch-commands.md) and the
   [build registry](maniac-patch-builds.md) are the authoritative source for the thresholds.

This keeps run-time configuration (no build-time `#define`s), reuses the existing framework, and
adds one well-named accessor rather than a subsystem.

## 4. Build-time vs run-time configuration

The task allowed build-time configuration as a fallback. It is **not needed**: every selection
input (fork DLL, VERSIONINFO build, engine version, logo CRC) is available at run time from the
game's own files, and EasyRPG already consumes them. Build-time `#define`s would also defeat the
goal of *one* binary that runs *all* forks/versions. Recommendation: **run-time detection only**,
with manual `--patch-*` / `EasyRPG.ini` overrides for the rare misdetection — exactly the present
model. The corpus's definitive per-binary identity (PE timestamp + hash, see
[build registry](maniac-patch-builds.md)) is the fallback when detection is ambiguous.

## 5. Status & recommendations

| Concern | Verdict |
|---|---|
| Select among forks | **Adequate today** (detect → flag → dispatch); verified on BR + Yume Nikki 16:9. |
| Command-ID collision across forks | **Not a real problem** (disjoint ranges + sigils + mutual exclusivity). No remapper needed; seam documented if ever forced. |
| Select among **builds** of one fork | **Latent gap** — build is detected but overloaded into `patch_maniac` and unused for gating. Add `Player::ManiacBuild()` + per-feature predicates, gate-by-gate, sourced from the specs. |
| Build-time config | **Unnecessary**; run-time detection covers it and preserves the single-binary goal. |

Net: keep the existing framework; do **not** build a command-ID remapper now; add a thin,
well-named **per-build capability layer** for Maniac Patch incrementally as specific build
divergences are implemented. This is the conservative path and it satisfies "support all forks
and all versions" without new subsystems.
