<!--
Spec: EasyRPG engine extensions & the patch-selection framework.
This is the "hub" document of the fork-support series: every other spec in docs/forks/
references the patch-selection machinery (Game_ConfigGame patch_* params, autodetection,
override, runtime flags) defined here, and the three command-dispatch switches that a future
fork command-remapping layer would extend.
All file:line anchors are into /home/john/Player (branch RISKY) and its bundled liblcf at
lib/liblcf. Equivalent upstream files: github.com/EasyRPG/Player and github.com/EasyRPG/liblcf
(line numbers may drift upstream).
-->

# EasyRPG engine extensions & patch-selection framework

> Unlike the other documents in this directory, this one describes **EasyRPG's own additions**
> rather than a third-party RPG_RT fork. EasyRPG Player invented its own block of event commands
> (codes **2002–2058**), its own LCF chunk space (**`0xC8`+ / "200+"**), and — most importantly
> for this whole effort — the **patch-selection framework**: the `Game_ConfigGame` `patch_*`
> configuration parameters, the `--patch-*` / `EasyRPG.ini [Patch]` surface, the auto-detection
> pipeline, the `patch_override` precedence rule, and the per-savegame `EasyRpg_SetInterpreterFlag`
> runtime toggle. Every fork spec in this series keys off the machinery documented here. This is
> also the document a future "fork selection mechanism / command-ID remapping" design will extend;
> see [§12 Extension seams](#12-extension-seams).

**Status at a glance:** EasyRPG support = N/A — this *is* EasyRPG. The patch framework itself is
FULL and is the canonical mechanism. The EasyRPG-invented event commands are PARTIAL (7 of 10
codes have handlers; 3 are enum-only). Detection of the EasyRPG extension set = **manual only**
(`--patch-easyrpg` / `[Patch] EasyRPG`; never auto-detected). Spec confidence = **HIGH** (entirely
source-verified in our tree).

## 1. Identity

- **Names / aliases:** "EasyRPG Engine Extensions" (the literal description string of the
  `patch_easyrpg` config param, `src/game_config_game.h:42`). Activated as `Player::HasEasyRpgExtensions()`
  (`src/player.h:564`). No Japanese name — this is an EasyRPG-native concept, not a JP community patch.
- **Author(s):** The EasyRPG team (Ghabry et al.). These are first-party features of EasyRPG Player
  and bundled liblcf.
- **License:** GPLv3 (Player) / MIT-like (liblcf). The extension command codes, chunk IDs and CSV
  field names are all defined in the open-source liblcf generator (`lib/liblcf/generator/csv/*_easyrpg.csv`),
  so there is no redistribution concern for shipping detection data.
- **Distribution:** <https://github.com/EasyRPG/Player> and <https://github.com/EasyRPG/liblcf>
  (both alive).
- **Liveness:** Actively maintained; this is the engine itself.

Two adjacent but **separate** concepts must not be confused:
1. **The `patch_easyrpg` flag** (this document) — opts a game into the EasyRPG-invented event
   commands and a handful of relaxed behaviors. Off by default.
2. **EasyRPG-native projects** (`is_easyrpg_project`) — projects authored in EasyRPG's own XML-ish
   `.edb`/`.emt` format (via `lcf2xml`), detected structurally at `src/player.cpp:1024`
   (`is_easyrpg_project = !edb.empty() && !emt.empty();`, where `edb`/`emt` are `DATABASE_NAME_EASYRPG`
   / `TREEMAP_NAME_EASYRPG`). This is
   an authoring/storage concept, *not* the patch flag, and is out of scope here except to disambiguate.

## 2. Target engine builds

Not applicable in the usual sense — these extensions are not derived from any RPG_RT build. They
ride on top of whichever engine EasyRPG has detected (RM2k or RM2k3, any version). A handful of
EasyRPG commands gate on `Player::IsPatchManiac()` or `Player::IsRPG2k3()` internally (noted per
command in §5), but the extension set has no target-version requirement of its own.

## 3. Version lineage

No published dated-build lineage exists for these extensions; they accreted commit-by-commit in
EasyRPG Player / liblcf master. The state captured here is the **RISKY** branch of this fork
(merge of upstream master at the time of the inventory; CMake project version **0.8.1**,
`CMakeLists.txt:3`). Individual command/feature ages are not separately published — treat the table
below as "present as of this tree" and consult `git log` / upstream PRs for finer dating
(RE-pending precise per-feature dating).

| Build | Date | Headline changes | Published |
|---|---|---|---|
| EasyRPG Player master ≈ 0.8.1 | this tree (RISKY) | Full `patch_*` framework; EasyRpg commands 2002/2003/2051/2053/2055/2056/2057 implemented; `0xC8`+ chunk space; ~104 `easyrpg_*` LCF fields | Yes (master) |

(Codes 2050, 2052, 2058 are enum-only placeholders — declared in liblcf, no Player handler — and
appear to be reserved for future work; see §5.)

## 4. Detection

The `patch_easyrpg` extension set is deliberately **never auto-detected** — there is no DLL, PE
signature, or data-file marker that means "this game wants EasyRPG extensions". It must be turned on
explicitly. This is by design: the extensions change semantics, so they are opt-in.

| Aspect | Value | Anchor |
|---|---|---|
| `Game_ConfigGame` field | `patch_easyrpg` (`BoolConfigParam`, default `false`) | `src/game_config_game.h:42` |
| `EasyRPG.ini [Patch]` key | `EasyRPG` | `src/game_config_game.h:42` |
| CLI flag | `--patch-easyrpg` / `--no-patch-easyrpg` | `src/game_config_game.cpp:106-110` |
| Predicate | `Player::HasEasyRpgExtensions()` → `game_config.patch_easyrpg.Get()` | `src/player.h:564` |
| Auto-detected? | **No.** Not present in the DLL-sniffing (`src/player.cpp:843-865`) or EXE-analysis (`:785-818`) paths inside `Player::CreateGameObjects` (`:697`) | — |
| Override behavior | Setting it (ini or CLI) sets `patch_override = true`, which disables *all* auto-detection of other patches | `src/game_config_game.cpp:108,219-221` |

For how detection works for *other* forks (the auto-detection pipeline this flag participates in by
setting `patch_override`), see [§10 Patch-selection framework](#10-patch-selection-framework-the-hub).

## 5. Event commands

EasyRPG-invented command codes occupy **2002–2058**, chosen to sit below the official RPG_RT ranges
(`1xxxx`/`2xxxx` plus `10`, `1005-1009`, `5001-5005`) and below the Maniacs range (3001–3032). These
codes are EasyRPG's own invention (they are *not* emitted by any commercial editor); they appear only
in EasyRPG-native projects or in projects hand-authored with EasyRPG in mind. Definitions live in
`lib/liblcf/generator/csv/enums_easyrpg.csv:4-13` and the generated
`lib/liblcf/src/generated/lcf/rpg/eventcommand.h:160-169`.

All handlers begin with `if (!Player::HasEasyRpgExtensions()) return true;` where applicable — the
commands are silently ignored when the extension flag is off (mirroring how Maniac/unknown commands
fall through `default: return true;`).

| Code | Name (EN) | Dispatch layer | Status | String field | Parameters (per index) | EasyRPG status + anchor |
|---:|---|---|---|---|---|---|
| 2002 | `EasyRpg_TriggerEventAt` | Map | Implemented | — | `[0]`/`[1]` X (val-or-var), `[2]`/`[3]` Y (val-or-var); optional `[4]` flags (bit 0 = `face_player`); calls `game_player->TriggerEventAt(x,y,triggered_by_decision_key,face_player)` | `src/game_interpreter_map.cpp:245,876` (min size 4) |
| 2003 | `EasyRpg_Pathfinder` | Map | Implemented | — | `[0]`/`[1]` source event id, `[2]`/`[3]` target X, `[4]`/`[5]` target Y, `[6]`/`[7]` search iteration limit, `[8]`/`[9]` route length (tiles), `[10]` flags (1 wait-when-moving, 2 allow-diagonal, 4 debug-log, 8 skip-when-moving, 16 route-skippable), `[11]`/`[12]` count of ignore-event-ids, `[13..13+N]` those ids, then move-frequency (val-or-var); computes an obstacle-avoiding move route and applies it | `src/game_interpreter_map.cpp:249,897` (min size 13; param doc at :897-915) |
| 2050 | `EasyRpg_CallMovementAction` | — | **Enum only — no handler** | — | (reserved) | enum `eventcommand.h:162`; no dispatch case |
| 2051 | `EasyRpg_WaitForSingleMovement` | Map | Implemented | — | `[0]`/`[1]` event_id (val-or-var), `[2]`/`[3]` failure_limit (val-or-var), `[4]`/`[5]` output_var (val-or-var); blocks the interpreter until that character's move route finishes (or failure_limit failed moves), writing a result code to output_var; resume state held in `_state.easyrpg_parameters`/`easyrpg_active` | `src/game_interpreter_map.cpp:247,975` (min size 6) |
| 2052 | `EasyRpg_AnimateVariable` | — | **Enum only — no handler** | — | (reserved) | enum `eventcommand.h:164`; no dispatch case |
| 2053 | `EasyRpg_SetInterpreterFlag` | Shared | Implemented | flag name (string) | see §5.1 op table | `src/game_interpreter.cpp:820,5546` (min size 2) |
| 2055 | `EasyRpg_ProcessJson` | Shared | Implemented (build-gated) | JSON pointer path | get/set JSON values to/from switch/var/string; built only when `HAVE_NLOHMANN_JSON`; **string ops require Maniac** (warning at `:5742`) | `src/game_interpreter.cpp:822,5651` (min size 8) |
| 2056 | `EasyRpg_CloneMapEvent` | Shared | Implemented (async) | target name (via `[10]`/`[11]` string-or-var) | `[0]`/`[1]` src_map (0 = current; non-current triggers a `RequestMap` async yield), `[2]`/`[3]` src_event, `[4]`/`[5]` target_x, `[6]`/`[7]` target_y, `[8]`/`[9]` target_event, `[10]`/`[11]` target_name; queues `AsyncOp::MakeCloneMapEvent`; records source in save fields `easyrpg_clone_map_id`/`easyrpg_clone_event_id`. Internal guard rejects `parameters.size() < 8` | `src/game_interpreter.cpp:824,5880` (CmdSetup min size 10) |
| 2057 | `EasyRpg_DestroyMapEvent` | Shared | Implemented | — | 2 params; removes a (cloned) map event | `src/game_interpreter.cpp:826,5915` (dispatch case min size 2; handler `CommandEasyRpgDestroyMapEvent`) |
| 2058 | `EasyRpg_StringPictureMenu` | — | **Enum only — no handler** | — | (reserved) | enum `eventcommand.h:169`; no dispatch case |

### 5.1 `EasyRpg_SetInterpreterFlag` (2053) — per-savegame patch toggling

This is the **runtime counterpart** to the static `patch_*` config: it lets a game turn an individual
patch ON or OFF for the duration of a savegame (the state is serialized into the LSD save, so it
survives save/load). Handler: `Game_Interpreter::CommandEasyRpgSetInterpreterFlag`
(`src/game_interpreter.cpp:5546-5649`). Requires `Player::HasEasyRpgExtensions()` and the build-time
macro `ENABLE_DYNAMIC_INTERPRETER_CONFIG` (defined unconditionally at `src/game_interpreter_shared.h:30`).

**Command shape:**
- `com.string` — the flag name (case-insensitive), e.g. `"maniac"`, `"common-this"`.
- `com.parameters[0]`/`[1]` — value mode + value (`ValueOrVariable`); non-zero ⇒ turn the patch **on**,
  zero ⇒ **off** (`:5568`).
- If `com.string` is empty and `parameters.size() > 2`, `com.parameters[2]` is used as the numeric flag id
  directly (`:5571-5572`).

**Flag-name → id table** (`src/game_interpreter.cpp:5555-5565`):

| id | Accepted string(s) | Toggles |
|---:|---|---|
| 1 | `destiny` | Destiny patch |
| 2 | `dynrpg` | DynRPG |
| 3 | `maniac` | Maniac Patch |
| 4 | `common-this` | Common This Event |
| 5 | `pic-unlock` | PicUnlock (pictures while message shown) |
| 6 | `key-patch` | Ineluki Key Patch |
| 7 | `rpg2k3-cmds`, `rpg2k3-commands` | RPG2k3-commands-in-2k |
| 8 | `rpg2k-battle` | force RM2k battle system |

**Semantics:** the flags live in the savegame as a paired on/off bitset
`lcf::rpg::SaveEventExecState::EasyRpgStateRuntime_Flags` (one `*_on` and one `*_off` bit per patch,
plus a master `conf_override_active` bit; defined in `lib/liblcf/generator/csv/flags_easyrpg.csv`).
On first use the handler clears any stale flags unless an override is already active (`:5581-5583`),
sets the requested on/off bit (`:5585-5644`), then sets `conf_override_active = true` (`:5645`). Every
patch predicate in `Player::` consults these flags first via
`Player::GetRuntimeFlag(&Flags::patch_X_on, &Flags::patch_X_off)`; the override only takes effect
while `conf_override_active` is set (`src/player.h:568-576`, e.g. `IsPatchManiac()` at `player.h:519`).
The currently active flag block is published through a global pointer
`Player::active_interpreter_flags` (declared `= &Player::interpreter_default_flags`, `src/player.h:441`),
re-pointed to the running interpreter's `_state.easyrpg_runtime_flags` at `src/game_interpreter.cpp:398`
with a `makeScopeGuard` that restores the default pointer on scope exit (`:399-401`). Separately, when the
base interpreter frame pops, the per-savegame `conf_override_active` bit is cleared at
`src/game_interpreter.cpp:867`. The in-game debug window can display the resolved flags
(`src/window_interpreter.cpp:51-60`; display names in array order `rpg2k3_cmds, dynrpg, maniac, keypatch,
destiny, common_this_event, unlock_pics, rpg2k_battle_system`).

## 6. Modified baseline commands

EasyRPG extensions widen a few existing behaviors rather than adding command IDs (all gated on
`Player::HasEasyRpgExtensions()`):

- **`Comment` / `Comment_2` (12410 / 22410):** DynRPG-style `@…` comment commands are accepted
  *without* DynRPG being active, but **only** functions whose name starts with `@easyrpg_`
  (`Game_Interpreter::HandleDynRpgScript`, `src/game_interpreter.cpp:2057-2069`). The registered
  EasyRpgPlugin then provides `@call`, `@easyrpg_output`, `@easyrpg_add` (see [dynrpg.md](dynrpg.md)).
- **System-function range 200–209 → custom main-menu subscreens:** when extensions are active, the
  `OpenSaveMenu` system-function dispatcher (`CommandOpenSaveMenu`) accepts ids `200..209` in its
  `default:` branch and calls `RequestMainMenuScene(current_system_function - 200, …)`
  (`src/game_interpreter_map.cpp:814-822`; `RequestMainMenuScene` at `:104`). This is an EasyRPG-only
  extension of the standard subscreen jump that DirectMenu also drives.
- **`Maniac_ControlStrings` `FromFile`:** wildcard file matching `t[..] FromFile("name*")` is enabled
  under EasyRPG extensions (`src/game_strings.cpp:236`) — a relaxation of Maniacs' exact-name lookup.
- **Maniacs expression in-place assignment is *disabled* under EasyRPG extensions** to avoid a
  semantic clash (`src/maniac_patch.cpp:150-154`). This is an intentional interaction: enabling the
  EasyRPG flag *removes* a Maniacs capability.

## 7. File-format changes

EasyRPG reserves the chunk space **`0xC8`+ (decimal 200+)** for its own LCF additions, by convention
to avoid colliding with official RPG_RT chunks and with the real Maniacs chunk IDs (which use their
genuine on-disk values). All additions are generated from
`lib/liblcf/generator/csv/fields_easyrpg.csv` (104 `easyrpg_*` field rows total) into
`lib/liblcf/src/generated/lcf/{ldb,lsd}/chunks.h`.

### 7.1 LDB (database) additions — `easyrpg_*`, chunk IDs `0xC8`–`0xDB`

Counts and chunk IDs below are the rows of `fields_easyrpg.csv` (line ranges given); the generated
enums land in `lib/liblcf/src/generated/lcf/ldb/chunks.h`. (`Actor`/`Enemy`/`Skill` each show 13 csv
rows but a few are size-tag `t` rows paired with their `f` data row, so the distinct chunk-IDs span is
`0xC9`–`0xD5`.)

| Struct | Count (csv rows) | Example chunks (id → meaning) | Anchor (`generator/csv/fields_easyrpg.csv`) |
|---|---:|---|---|
| `System` | 20 | `easyrpg_alternative_exp=0xC8` (EXP formula 0/1/2), battle options, stat/variable caps, `easyrpg_default_enemyai=0xDB`, RM2k-battle switch `easyrpg_use_rpg2k_battle_system`; IDs `0xC8`–`0xDB` | csv 137–156 |
| `Terms` | 20 | extra UI / battle message strings, e.g. `easyrpg_item_number_separator=0xC8` … `easyrpg_battle2k3_item=0xDB` | csv 117–136 |
| `Actor` | 13 | `easyrpg_actorai=0xC9`, `easyrpg_prevent_critical=0xCA`, `easyrpg_raise_evasion=0xCB`, … `easyrpg_dual_attack=0xD4`, `easyrpg_attack_all=0xD5` | csv 65–77 |
| `Enemy` | 13 | `easyrpg_enemyai=0xC9`, `easyrpg_prevent_critical=0xCA` … `easyrpg_super_guard=0xD4`, `easyrpg_attack_all=0xD5` | csv 79–91 |
| `Skill` | 13 | `easyrpg_battle2k3_message=0xC9`, `easyrpg_ignore_reflect=0xCA` … `easyrpg_hp_percent=0xD4`, `easyrpg_hp_cost=0xD5` | csv 92–104 |
| `BattleCommands` | 6 | `easyrpg_default_atb_mode=0xC8`, `easyrpg_enable_battle_row_command=0xC9`, `easyrpg_sequential_order=0xCA`, `easyrpg_disable_row_feature=0xCB`, `easyrpg_fixed_actor_facing_direction=0xCC`, `easyrpg_fixed_enemy_facing_direction=0xCD` | csv 59–64 |
| `Item` | 2 | `easyrpg_using_message=0xC9`, `easyrpg_max_count=0xCA` | csv 105–106 |
| `Terrain` | 2 | `easyrpg_damage_in_percent=0xC8`, `easyrpg_damage_can_kill=0xC9` | csv 109–110 |
| `State` | 2 | `easyrpg_immune_states` — size-tag `0xC8` + `DBBitArray` data `0xC9` (one logical field, two chunk IDs) | csv 107–108 |

These DB fields are how a game opts into EasyRPG's extended behaviors *per-record* (e.g. custom AI,
stat caps) without a global flag. The `System.easyrpg_*` stat/variable caps overlap conceptually with
the EXE-derived `Game_Constants` overrides (see [runtime-micro-patches.md](runtime-micro-patches.md)
§ EXE-derived constants).

### 7.2 LSD (save) additions — `easyrpg_*`

| Struct | Chunk(s) | Meaning | Anchor |
|---|---|---|---|
| `Save` | `easyrpg_data = 0xC8` (struct `SaveEasyRpgData`) | container: `version` (0x01), `codepage` (0x02), `windows` (0x64, array of `SaveEasyRpgWindow` from ShowStringPicture) | `fields_easyrpg.csv:2-5`; `lsd/chunks.h:1062` (ChunkSave), `:1065` (ChunkSaveEasyRpgData) |
| `SavePicture` | `easyrpg_flip=0xC8` / `easyrpg_blend_mode=0xC9` / `easyrpg_type=0xCA` | EasyRPG picture flip/blend/type; field types `Enum<EasyRpgFlip>` (none/x/y/both) and `Enum<EasyRpgPictureType>` (the enum block is named `EasyRpgType`: default/window/canvas) | `fields_easyrpg.csv:16-18`; enums `enums_easyrpg.csv:52-58` |
| `SaveMapEventBase` | `easyrpg_move_failure_count=0xC9`, `easyrpg_clone_map_id=0xCA`, `easyrpg_clone_event_id=0xCB`, `easyrpg_runtime_flags=0xCC` | clone bookkeeping for `EasyRpg_CloneMapEvent`; per-event runtime flag block | `lsd/chunks.h:795` (ChunkSaveMapEventBase, fields :876-882); `fields_easyrpg.csv:36-39` |
| `SaveEventExecState` | `easyrpg_active=0xC9`, `easyrpg_string=0xCA`, `easyrpg_parameters=0xCB`, `easyrpg_runtime_flags=0xCC` | `easyrpg_runtime_flags` = the per-savegame patch toggle bitset (§5.1) | `lsd/chunks.h:792` (ChunkSaveEventExecState at :739); `fields_easyrpg.csv:12-15` |
| `SaveEventExecFrame` | `easyrpg_runtime_flags=0xCC` | per-frame runtime flags | `lsd/chunks.h:736` (ChunkSaveEventExecFrame at :709); `fields_easyrpg.csv:11` |

### 7.3 liblcf round-trip caveat

Unknown **chunks** are dropped on rewrite (`LcfReader::Skip`, `lib/liblcf/src/reader_lcf.cpp:281-296`
— "Skipped Chunk %02X … in lcf at %X" debug log + hex dump, no retention), so a game relying on chunks
liblcf doesn't model survives *loading* but not save round-tripping. Unknown **event-command codes**, by
contrast, survive losslessly because `EventCommand` is read raw (`code`/`indent`/`string`/`parameters`,
no enum validation: `lib/liblcf/src/ldb_eventcommand.cpp:40-52`) — this is the key extension point for
adding new forks. See [§12 Extension seams](#12-extension-seams).

## 8. Runtime behavior changes

Behaviors enabled by `HasEasyRpgExtensions()` that are not commands or chunks:

- **>8 bpp system/cache images allowed** (`src/cache.cpp:282`), same relaxation Maniac grants.
- **`@easyrpg_*` comment commands without DynRPG** (§6).
- **Custom main-menu subscreens** via system-function ids 200–209 (§6).
- **Per-savegame patch toggling** via `EasyRpg_SetInterpreterFlag` (§5.1) — the only mechanism in the
  engine to change patch state mid-game.
- **JSON processing** (`EasyRpg_ProcessJson`, 2055) when built with nlohmann-json.

## 9. EasyRPG support matrix

Because this document *is* EasyRPG's own feature set, "support" reduces to "is the feature
implemented in this tree":

| Feature | Status | Anchor / note |
|---|---|---|
| Patch-selection framework (`patch_*`, override, autodetect, `--no-patch`) | FULL | `src/game_config_game.{h,cpp}`, `src/player.cpp:785-865` (detection in `CreateGameObjects` :697) |
| `EasyRpg_SetInterpreterFlag` (2053) | FULL | `src/game_interpreter.cpp:5546` |
| `EasyRpg_TriggerEventAt` (2002) | FULL | `src/game_interpreter_map.cpp:876` |
| `EasyRpg_Pathfinder` (2003) | FULL | `src/game_interpreter_map.cpp:897` |
| `EasyRpg_WaitForSingleMovement` (2051) | FULL | `src/game_interpreter_map.cpp:975` |
| `EasyRpg_ProcessJson` (2055) | PARTIAL | build-gated (`HAVE_NLOHMANN_JSON`); string ops need Maniac (`:5742`) |
| `EasyRpg_CloneMapEvent` (2056) | FULL | `src/game_interpreter.cpp:5880` |
| `EasyRpg_DestroyMapEvent` (2057) | FULL | `src/game_interpreter.cpp:5915` |
| `EasyRpg_CallMovementAction` (2050) | MISSING | enum only; no handler |
| `EasyRpg_AnimateVariable` (2052) | MISSING | enum only; no handler |
| `EasyRpg_StringPictureMenu` (2058) | MISSING | enum only; no handler |
| `easyrpg_*` LDB/LSD chunks (~104 fields) | FULL (modeled in liblcf) | `lib/liblcf/generator/csv/fields_easyrpg.csv` |

## 10. Patch-selection framework (the hub)

This section is the load-bearing one for the rest of the series. Every fork spec references the
machinery below for its "Detection → EasyRPG mapping" row.

### 10.1 Configuration plumbing

`Game_ConfigGame` (`src/game_config_game.h`) holds ~25 `patch_*` `ConfigParam`s. They are loaded by
`Game_ConfigGame::Initialize()` (`src/game_config_game.cpp:28`) in two phases:

1. `LoadFromStream()` reads `EasyRPG.ini` from the game directory (`EASYRPG_INI_NAME = "EasyRPG.ini"`,
   `src/options.h:57`), section **`[Patch]`** (`game_config_game.cpp:206-262`).
2. `LoadFromArgs()` applies CLI flags, which **override** ini values (`game_config_game.cpp:62-204`).

Each `ConfigParam` carries its own ini section/key in its constructor; e.g.
`BoolConfigParam patch_dynrpg{ "DynRPG", "", "Patch", "DynRPG", false }` ⇒ ini `[Patch] DynRPG`
(`game_config_game.h:44`). **`RPG_RT.ini` is not consulted for patch selection** — it is read only for
`GameTitle`, `FullPackageFlag`, `WinW`/`WinH` (`src/player.cpp:746-762`).

### 10.2 Full `patch_*` parameter table

| Param (C++ field) | Type / default | `[Patch]` ini key | CLI flag(s) | Auto-detected by | Anchor(s) |
|---|---|---|---|---|---|
| `patch_easyrpg` | bool / false | `EasyRPG` | `--patch-easyrpg` / `--no-patch-easyrpg` | — (manual only) | h:42; cpp:106 |
| `patch_destiny` | bool / false | `Destiny` — **declared but NOT read in `LoadFromStream`** (omission) | **none** (no CLI flag) | `Destiny.dll` present | h:43; player.cpp:857 |
| `patch_dynrpg` | bool / false | `DynRPG` | `--patch-dynrpg` / `--no-…` | `dynloader.dll` present | h:44; cpp:111; player.cpp:848 |
| `patch_maniac` | **int** / 0 | `Maniac` | `--patch-maniac [N]` / `--no-…` (`N=2` = on but keep RPG_RT variable limits) | EXE VERSIONINFO `Maniacs, v…` or `accord.dll` | h:45; cpp:116; player.cpp:798-802,853 |
| `patch_common_this_event` | bool / false | `CommonThisEvent` | `--patch-common-this` / `--no-…` | — | h:46; cpp:126 |
| `patch_unlock_pics` | bool / false | `PicUnlock` | `--patch-pic-unlock` / `--no-…` | — | h:47; cpp:131 |
| `patch_key_patch` | bool / false | `KeyPatch` | `--patch-key-patch` / `--no-…` | `harmony.dll` present | h:48; cpp:136; player.cpp:844 |
| `patch_rpg2k3_commands` | bool / false | `RPG2k3Commands` | `--patch-rpg2k3-cmds`, `--patch-rpg2k3-commands` / `--no-…` | — | h:49; cpp:141 |
| `patch_anti_lag_switch` | int (switch id) / 0 | `AntiLagSwitch` | `--patch-antilag-switch SWITCH` / `--no-…` | — | h:50; cpp:146 |
| `patch_direct_menu` | int (var id) / 0 | `DirectMenu` | `--patch-direct-menu VAR` / `--no-…` | — | h:51; cpp:158 |
| `patch_encounter_random_alert_sw` | int (switch) / 0 | `EncounterAlert.Switch` | `--patch-encounter-alert [-sw S] [-var V]` | — | h:53; game_runtime_patches |
| `patch_encounter_random_alert_var` | int (var) / 0 | `EncounterAlert.Var` | (same flag, `-var`) | — | h:54 |
| `patch_monsca_maxhp`…`patch_monsca_droprate` (10) | int (var id) / 0 | `MonSca.MaxHP`, `MonSca.MaxSP`, `MonSca.Attack`, `MonSca.Defense`, `MonSca.Spirit`, `MonSca.Agility`, `MonSca.Experience`, `MonSca.Money`, `MonSca.ItemId`, `MonSca.ItemDropRate` | `--patch-monsca [-maxhp N] [-maxsp N] [-atk N] [-def N] [-spi N] [-agi N] [-exp N] [-gold N] [-item N] [-droprate N] [-lvlscale N] [-plus N]` | — | h:56-65 |
| `patch_monsca_levelscaling` | int (switch id) / 0 | `MonSca.LevelScaling` | (same flag, `-lvlscale`) | — | h:66 |
| `patch_monsca_plus` | int / 0 | `MonSca.Plus` | (same flag, `-plus`) | — | h:67 |
| `patch_explus_var` | int (var) / 0 | `EXPlus.VarExpBoost` | `--patch-explus [-var N]` | — | h:69 |
| `patch_explusplus_var` | int (var) / 0 | `EXPlus.VarActorInParty` | (same flag) | — | h:70 |
| `patch_guardrevamp_normal` | int (%) / 0 | `GuardRevamp.NormalDefense` | `--patch-guardrevamp [-normal N] [-strong N]` | — | h:72 |
| `patch_guardrevamp_strong` | int (%) / 0 | `GuardRevamp.StrongDefense` | (same flag) | — | h:73 |
| `patch_powermode` | int / 0 | `PowerMode2003` | `--patch-powermode` / `--no-patch-powermode` | `warp.dll` present | h:75; cpp:170; player.cpp:861 |
| `patch_support` | bool / true (**CLI only**, no ini key) | — | `--no-patch` (disables ALL patch support and `Lock()`s every patch off) | n/a | h:78; cpp:90-105 |
| `patch_override` | plain bool (not a `ConfigParam`) | — | set by any explicit patch option (ini or CLI); **disables all auto-detection** | n/a | h:81; cpp:103,108,220; player.cpp:800,843 |

Notes:
- The `EncounterAlert`/`MonSca`/`EXPlus`/`GuardRevamp` params are *behavioral* micro-patches parsed
  through `RuntimePatches::ParseFromCommandLine`/`ParseFromIni` (`game_config_game.cpp:175,259`); their
  semantics are documented in [runtime-micro-patches.md](runtime-micro-patches.md). The default switch/var
  ids (e.g. ERA switch 1018 / var 3355, MonSca V1001–V1010, EXPlus V3333/V3332) are defaults applied only
  when the patch is enabled, defined in `game_runtime_patches.h`.
- **`patch_destiny` anomaly:** it has *no* CLI flag and its `[Patch] Destiny` ini key is declared but
  `patch_destiny.FromIni(ini)` is **never called** in `LoadFromStream` (there is no read between the
  `patch_dynrpg` block at `cpp:223` and the `patch_maniac` block at `cpp:227`). Destiny is therefore
  reachable only via `Destiny.dll` auto-detection (`player.cpp:857`) or the runtime flag (§5.1). Likely
  an oversight (RE-pending / UNVERIFIED whether intentional).
- Legacy multi-value form `--patch dynrpg maniac common-this pic-unlock key-patch rpg2k3-cmds` is kept
  for backwards compatibility (`game_config_game.cpp:179-200`).
- `PrintActivePatches()` logs the consolidated state at startup (`game_config_game.cpp:264`). Note it
  *does* print `patch_destiny` even though that flag can't be set from ini/CLI (`cpp:275`).

### 10.3 `patch_override` precedence

The single most important rule for the whole series: **any explicit patch setting (ini or CLI) sets
`patch_override = true`, which suppresses *all* auto-detection.** Concretely:

- Every `FromIni` block that finds a value sets `patch_override` (`game_config_game.cpp:219-261`).
- Every `--patch-*` CLI handler sets it (`game_config_game.cpp:108,113,123,128,…`).
- `--no-patch` sets it after locking every patch off (`game_config_game.cpp:90-104`).
- The auto-detection block in `player.cpp` is wholly guarded: `if (!game_config.patch_override) { …DLL
  sniffing… }` (`src/player.cpp:843-865`), and the Maniac-version write from the EXE is guarded too
  (`player.cpp:800-802`).

Consequence to document in sibling specs: *manually setting any one patch turns off detection for
all of them.* A user who sets `[Patch] Maniac=1` for a Maniac game that also ships `harmony.dll`
will **not** get Ineluki auto-detected, because `patch_override` is now set. (This is the precedence
note referenced from each fork spec's §4.)

### 10.4 Auto-detection pipeline (`Player::CreateGameObjects`, `src/player.cpp:697`; detection `:785-886`)

Order of evidence when `engine == EngineNone` and `!patch_override`:

1. **`RPG_RT.exe` analysis** via `EXEReader` (`src/exe_reader.cpp`; uses `engine_path` if set, not on
   Emscripten): VERSIONINFO product version, logo count, CODE/CHERRY/GEEP section sizes, the
   `is_easyrpg_player` marker, and the Maniacs build number parsed from a `"Maniacs, v…"` string
   (`player.cpp:795-805`). The detected Maniac build is written to `patch_maniac` unless overridden
   (`player.cpp:800-802`). Stat-patch constant overrides come from
   `EXEReader::GetOverriddenGameConstants()` (`player.cpp:805`). Engine-detection details (logos, section
   sizes, the 1.1.2.1 Maniacs rules) are documented in [official-versions.md](official-versions.md) and
   [maniac-patch.md](maniac-patch.md).
2. **LDB fallback** when no EXE verdict (`player.cpp:820-837`): `lcf::Data::system.ldb_id == 2003` ⇒
   2k3 (+`ultimate_rt_eb.dll` ⇒ English|MajorUpdated); else 2k, with `Data::data.version >= 1` ⇒
   English|MajorUpdated; else the `FileFinder::IsMajorUpdatedTree()` heuristic.
3. **DLL sniffing** (skipped entirely if `patch_override`; `player.cpp:843-865`):

   | File present | Sets | Anchor |
   |---|---|---|
   | `harmony.dll` | `patch_key_patch = true` | `player.cpp:844` |
   | `dynloader.dll` | `patch_dynrpg = true` (+ "will not run properly" warning) | `player.cpp:848` |
   | `accord.dll` | `patch_maniac = 1` (only if not already set) | `player.cpp:853` |
   | `Destiny.dll` (`DESTINY_DLL`) | `patch_destiny = true` | `player.cpp:857` |
   | `warp.dll` | `patch_powermode = true` | `player.cpp:861` |

4. **Post-detection hooks:** Ineluki `autorun.script` (`player.cpp:880-882`); Destiny `Load()`
   (`player.cpp:884-886`).

## 11. Test assets

No game catalogued in `docs/games/` is keyed specifically to the `patch_easyrpg` flag — the one game
documented there, **Beloved Rapture** ([../games/beloved-rapture.md](../games/beloved-rapture.md),
Maniac 220325), is a Maniac-Patch consumer whose detection path runs through this framework but which
does not enable EasyRPG extensions. The `EasyRpg_*` commands and `easyrpg_*` chunks are primarily
exercised by EasyRPG's own test suite (`lib/liblcf/tests/`) and by EasyRPG-native demo projects.
General corpus lives at `c:\rg\easyrpg_library`. (Test-asset gap: RE-pending a dedicated `patch_easyrpg`
repro project.)

## 12. Extension seams

This document is the one a future **fork-selection / command-ID-remapping** design will extend. The
three relevant dispatch switches — each a `switch (static_cast<Cmd>(com.code))` that ends with a
`default:` falling through (so unknown codes are silently skipped) — are:

| Layer | Function | Switch start | `default:` behavior | Anchor |
|---|---|---|---|---|
| Shared / base | `Game_Interpreter::ExecuteCommand` | `src/game_interpreter.cpp:612` | `return true;` (skip) | cases to :827, default :828 |
| Map | `Game_Interpreter_Map::ExecuteCommand` | `src/game_interpreter_map.cpp:186` | falls through to base | default :251-252 |
| Battle | `Game_Interpreter_Battle::ExecuteCommand` | `src/game_interpreter_battle.cpp:225` (fn :224) | falls through to base | default :260-261 |

ID-collision landscape a remap layer must respect: baseline RPG_RT uses `10`/`1005-1009`/`5001-5005`
and the `1xxxx`/`2xxxx` blocks; **EasyRPG occupies 2002–2058**; **Maniacs occupies 3001–3032**. Any new
fork that adds *numeric* command codes (rather than comment-scripting like DynRPG `@` / Destiny `$`,
which hook `CommandComment` (`game_interpreter.cpp:2106`) via `HandleDynRpgScript` (`:2057`) /
`HandleDestinyScript` (`:2093`)) must be checked against these
three ranges; a collision needs a per-`game_config` remapping pass.

Natural insertion points for a command-remapping layer (in increasing invasiveness):
1. **Before each of the three `ExecuteCommand` switches** — translate `com.code` based on the active
   fork config immediately on entry. Local, requires no liblcf change.
2. **In liblcf as alternate enum CSVs** — `lib/liblcf/generator/csv/enums_easyrpg.csv` already
   demonstrates how a new command block is declared and regenerated; a fork could ship its own enum
   block.
3. **A load-time LMU/LDB command-stream rewriter** — rewrite `com.code` values as files load. Heaviest,
   but keeps the interpreter unaware of forks.

EasyRPG already demonstrates per-patch *semantic* remapping of shared codes via `Player::IsPatchManiac()`
branches inside handlers and `ManiacBitmask()` masking (`src/game_interpreter.cpp:5941`); and per-savegame
*behavioral* switching via `EasyRpg_SetInterpreterFlag` (§5.1). The remaining gap a future design fills
is *code-level* remapping (one fork's 3xxx command meaning another's), for which seam (1) is the cleanest
hook. See the [README architecture notes](README.md#architecture-notes-for-adding-a-fork-orientation)
and the design note when implemented.

## 13. Open questions

- **`patch_destiny` ini read:** is the missing `patch_destiny.FromIni(ini)` in `LoadFromStream`
  (`game_config_game.cpp:206-262`) intentional or a bug? (RE-pending; appears to be an oversight since
  `PrintActivePatches` still reports it.)
- **Reserved EasyRpg commands:** what are the intended semantics of `EasyRpg_CallMovementAction` (2050),
  `EasyRpg_AnimateVariable` (2052), `EasyRpg_StringPictureMenu` (2058)? (Enum-only; design not yet in
  this tree.)
- **Per-feature dating:** exact upstream PR/commit history for each `EasyRpg_*` command and `easyrpg_*`
  chunk (release-vs-master annotations) is not captured here. (RE-pending `git log` / upstream PR survey.)
- **Codepage field in `SaveEasyRpgData`:** interaction between `SaveEasyRpgData.codepage` and the
  Maniac `save.system.maniac_strings` encoding is not fully traced. (UNVERIFIED.)

## 14. References

Primary sources (all in this tree; line numbers from branch RISKY):
- `src/game_config_game.{h,cpp}` — the `patch_*` framework, ini/CLI loading, `patch_override`.
- `src/player.cpp:697` (`CreateGameObjects`), EXE analysis `:785-818`, LDB fallback `:819-837`, DLL
  sniffing `:843-865`, post-detection hooks `:880-886` — engine + patch auto-detection pipeline.
- `src/player.h:439-576` — patch predicates and `GetRuntimeFlag` plumbing (`HasEasyRpgExtensions` :564).
- `src/game_interpreter.cpp:611-831` (shared dispatch), `5546-5649` (SetInterpreterFlag), `5651-5878`
  (ProcessJson), `5880-5914` (CloneMapEvent), `5915-` (DestroyMapEvent) — EasyRpg command handlers.
- `src/game_interpreter_map.cpp:185-254,876,897,975` — map dispatch + EasyRpg map commands.
- `src/game_interpreter_battle.cpp:224-262` — battle dispatch.
- `src/game_interpreter_shared.h:30,37-156` — `ENABLE_DYNAMIC_INTERPRETER_CONFIG`, dispatch helpers.
- `src/window_interpreter.cpp:33-61` — debug display of runtime flags.
- `lib/liblcf/generator/csv/enums_easyrpg.csv`, `flags_easyrpg.csv`, `fields_easyrpg.csv` — generator
  inputs for the command codes, runtime flags, and `easyrpg_*` fields.
- `lib/liblcf/src/generated/lcf/rpg/eventcommand.h:160-196`, `lcf/ldb/chunks.h`, `lcf/lsd/chunks.h` —
  generated definitions.
- Research inventory (off-repo): `/home/john/research/easyrpg_patch_inventory.md`.

Cross-links: [README.md](README.md) · [maniac-patch.md](maniac-patch.md) · [dynrpg.md](dynrpg.md) ·
[destiny.md](destiny.md) · [ineluki-key-patch.md](ineluki-key-patch.md) ·
[powermode2003.md](powermode2003.md) · [runtime-micro-patches.md](runtime-micro-patches.md) ·
[official-versions.md](official-versions.md) · [../games/beloved-rapture.md](../games/beloved-rapture.md).
