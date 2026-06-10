<!--
Per-GAME compatibility dossier (adapted from docs/forks/TEMPLATE.md).
This is the priority target of the fork-support effort: getting *Beloved Rapture*
running under EasyRPG Player. It is a consumer of the Maniac Patch fork spec, not a
fork spec itself, so several TEMPLATE sections are narrowed to "what THIS game does".
Cite every nontrivial claim. Mark guesses (UNVERIFIED)/(RE-pending)/(hypothesis).
-->

# Beloved Rapture — game compatibility dossier

> *Beloved Rapture* is a commercial RPG by **Rapturous Studio** (lead **Aaron T. Crawford**,
> RMN handle "Blind"), released **2024-10-07** on Steam (appid **2017620**) and GOG. It is
> built on **RPG Maker 2003 v1.12a (Steam)** patched with **BingShan's Maniac Patch, build
> 220325** (version resource: `RPG Maker 2003 Runtime (Maniacs, v220325, en, im)`). Despite
> being a heavily-customised widescreen (426×240) game with an entirely string-picture-driven
> UI and a Maniacs-hooked battle system, it uses only **11 distinct Maniac Patch event
> commands** and **zero command codes unknown to liblcf** — the gap to running it on EasyRPG
> is faithful semantics for those 11 commands plus a handful of resolution/font/menu quirks,
> not a parsing problem. This dossier is the engine-facing reconstruction target for the
> whole fork-support effort.

**Status at a glance:** EasyRPG support = **PARTIAL** (0.8.1 "Stun": demo playable; full game
has a still-unidentified missing event command + combat-system issues). Detection = Maniac
Patch via `BelovedRapture.exe` VERSIONINFO `Maniacs, v220325` (see [§4](#4-detection)). Spec
confidence = **HIGH** for identity/census/engine surface (binary RE complete), **MEDIUM** for
the exact runtime semantics of individual Maniac commands vs. EasyRPG's implementations.

Fork references used throughout: [maniac-patch.md](../forks/maniac-patch.md) (fork overview),
[maniac-patch-commands.md](../forks/maniac-patch-commands.md) (per-command reference),
[maniac-patch-fileformats.md](../forks/maniac-patch-fileformats.md) (chunks/files).
**Note:** `maniac-patch-commands.md` is a *planned* companion file that does not yet exist —
all links to it here are forward references, consistent with `maniac-patch.md` §11 (which owns
the same forward reference). Per-index parameter layouts live there once written.

---

## 1. Identity

| Item | Value | Source |
|---|---|---|
| Title | **Beloved Rapture** | — |
| Steam App ID | **2017620** | <https://store.steampowered.com/app/2017620/Beloved_Rapture/> |
| Steam release | **2024-10-07** | Steam appdetails API (`release_date: Oct 7, 2024`), <https://store.steampowered.com/api/appdetails?appids=2017620> |
| GOG | DRM-free, Win 10/11, same release date, "~20 h"; "developers actively patching" | <https://www.gog.com/en/game/beloved_rapture> |
| Price | $19.99 USD | store page (above) |
| Developer | **Rapturous Studio** | Steam appdetails API |
| Publisher | **2 Left Thumbs** | Steam appdetails API |
| Lead dev | **Aaron T. Crawford** — RMN handle **"Blind"** (formerly "Blind\|Mind"), Boston MA | <https://rpgmaker.net/games/8/>, <https://x.com/BelovedRapture> |
| Programmer | **Giznads** (joined 2022; widescreen recalibration, Maniacs animated-enemy battles, custom menu) | RMN blogs <https://rpgmaker.net/games/8/blog/24503/>, <https://rpgmaker.net/games/8/blog/24825/> |
| Widescreen/battle help | **Cherry** (credited alongside Giznads for recalibration) | <https://rpgmaker.net/games/8/blog/24825/> |
| Platforms (official) | **Windows only** (`mac:false, linux:false`) | Steam appdetails API |
| Soundtrack DLC | appid **3278470** (type `music`, 2024-10-12) — *not* a content DLC | <https://store.steampowered.com/api/appdetails?appids=3278470> |
| Free expansion | **"Lost Songs of Archon"** (2025-11-09) — integrates into existing overworld; not a separate map tree | <https://store.steampowered.com/news/app/2017620/view/597416631491101553>, <https://rpgmaker.net/games/8/blog/25981/> |
| Steam reviews | "Very Positive", 83% of 227 positive (mid-2026) | store page |
| Length | ~24 h (20–27 reported by testers) | dev FAQ <https://steamcommunity.com/app/2017620/discussions/0/4849904427678903276/> |
| Content shape | 6 playable characters, **no random encounters**; Classic/Story difficulty; Active/Wait ATB toggle | store page; dev FAQ |
| RMN profiles | **game ID 8** (original freeware, demo on RMN since 2007) <https://rpgmaker.net/games/8/>; **game ID 13192** (commercial, Jan 2026) <https://rpgmaker.net/games/13192/> | — |
| IA archive (old free build) | <https://archive.org/details/rmn-game-8-beloved-rapture> | — |

**License / redistribution.** Commercial, paid, all-rights-reserved. Do **not** ship game
assets, the LDB, the `.exe`, or extracted text tables with the engine or this spec — only
non-redistributable detection metadata (version strings, file layout, command census) may be
recorded here. The corpus copy at `/mnt/c/rg/easyrpg_library/beloved_rapture/` is for local RE
only.

**Liveness.** Actively maintained as of 2026-06: free DLC shipped 2025-11; a further DLC and a
"Director's Cut" are announced (<https://rpgmaker.net/games/8/blog/25981/>). The developer has
publicly stated they are **"exploring the EasyRPG engine"** for console ports / achievements
(<https://steamcommunity.com/app/2017620/discussions/0/4849903793440542032/>), which makes
faithful EasyRPG support directly relevant to the studio, not just to preservation.

---

## 2. Target engine build

**RPG Maker 2003 v1.12a (Steam) + Maniac Patch build 220325 (en, im variant).** Definitive —
three independent signatures agree:

| Evidence | Value | Source |
|---|---|---|
| `BelovedRapture.exe` PE VERSIONINFO (UTF-16) | `RPG Maker 2003 Runtime (Maniacs, v220325, en, im)` | `strings -el` on the exe; ANALYSIS.md |
| `Beloved_Rapture.r3proj` (16 bytes, text) | `RPG2003 v1.12a` | ANALYSIS.md |
| ASCII string in exe | `maniac patch` (×2), `ultimate_rt_eb` | br_dispatch_findings.md @ 0x0054ccb4/0x0054ccc8 |
| `RPG_RT.ini [RPG_RT]` | Maniacs window keys `WinW=426`, `WinH=240` | ANALYSIS.md |

The shipped runtime is a **whole replacement** `RPG_RT.exe` (shipped as `Game.exe`, renamed
`BelovedRapture.exe`), not a binary patch applied over a stock exe. It is a **MSVC C++ rewrite
of the interpreter grafted onto the RM2k3 binary** — the event-command dispatcher is pure MSVC
switch/jump-table code, not the original Delphi cascaded compares (br_dispatch_findings.md
§Dispatcher).

| Binary | Size | SHA-256 | Notes |
|---|---:|---|---|
| `BelovedRapture.exe` | 1,522,176 | `5f3eddd55aa55d9f7bbe0a7ae8c4a3be82db01b4786d954fa27cd17a555dbe1f` | PE32 GUI x86, 5 sections, renamed Maniacs RPG_RT; not packed (no UPX) | 
| `Parser.exe` (root + `Text/`) | 32,768 | `c8f8aaf10cc6a58b0f2cdc8b8c3bc815ce464d0ac69d170e0f763e9df082909f` | .NET 4.7.2 dev tool; PDB `D:\rm2k3\CBS\Sources\Parser\obj\Debug\Parser.pdb` | 

Static-RE risk negligible: both hashes return no scanner listings and no evidence of malice
(public-index lookups only, nothing uploaded/executed; `/home/john/research/vt_hash_checks.md`).

### 2.1 What `en` / `im` mean (from BingShan's official Maniacs site, JP)

Source: **Steam2003 Maniacs** <https://bingshan1024.github.io/steam2003_maniacs/>, section
「im版とpf版」 ("im edition and pf edition").

- **`im`** = 「出現条件の変更直後（標準）/ Immediately (Default)」 — map-event **page
  appearance conditions are re-checked immediately after a change**. The compatibility-focused
  edition (「互換性を重視したバージョン」); BingShan recommends it when unsure.
- **`pf`** = 「毎フレーム（一部の挙動に互換性なし）/ Per frame (Incompatible)」 — conditions
  re-checked every frame.
- From Maniacs **v241028** the two executables were **merged**, selected by `RPG_RT.ini`
  `[RPG_RT] pf=1`. BR (220325) predates the merge, hence the literal `im` tag.
- **`en`** = English-language build of the Maniacs runtime (Maniacs ships 日本語版/英語版
  *nihongo-ban / eigo-ban*, "Japanese edition / English edition" variants; v210414 unified
  usable font types across editions).

EasyRPG implication: EasyRPG already re-checks page conditions on change, so the `im`/`pf`
distinction is effectively a no-op for the engine. (UNVERIFIED that any BR event depends on the
*timing* of the recheck; the census shows no construct that obviously would.)

---

## 3. Version lineage (of the engine build, not the game)

BR's Maniac build **220325 was never publicly released**: it is an interim dev build
distributed via BingShan's Discord; the public Maniacs site repo has no commits in the
2021-10 → 2024-10 window (ANALYSIS.md resolved Q5). Its feature set therefore sits between two
documented public builds. The authoritative record of *what 220325 actually dispatches* is the
binary enumeration in [§5](#5-event-commands), not the public changelog.

| Build | Date | Relevance to BR | Published |
|---|---|---|---|
| v210414 | 2021-04-14 | EasyRPG's primary reference target ("we almost support everything of the legacy version v210414") | Yes |
| **220325** | 2022-03-25 (build id) | **BR's exact build.** Feature set = ≤211010 docs + a subset of 241028 changes | **No — Discord-only dev build** |
| v241028 | 2024-10-28 | 64-bit, im/pf merged, asset encryption (`RPG_RT.rs1`), provisional movie support — **none used by BR** | Yes |

Key 220325-specific facts pinned by RE (br_dispatch_findings.md), versus what liblcf models:

- **Has** all 26 liblcf Maniac codes (3001–3021, 3025–3029) in the dispatcher
  (br_dispatch_findings.md "26 KNOWN-MANIAC"), plus the two non-liblcf codes 3022 and 3024
  (counted among the "3 UNKNOWN-TO-LIBLCF" alongside the code-0 terminator; see below).
- **Does NOT have** `Maniac_Zoom` (liblcf 3032) nor codes 3023/3030/3031 — Zoom postdates
  220325. An engine emulating *this build* must treat Zoom as absent.
- Contains an **undocumented command 3022** = "execute inline expression bytecode" (expression
  statement), handler `FUN_00448a50`, which drives the Maniacs recursive expression VM at
  `FUN_00445140`; both are **unknown to liblcf** (the liblcf enum jumps 3021→3025, so 3022/3023/
  3024 have no enum). **BR's own data never emits 3022** (census shows zero), so it does not
  block BR but is a forward-compat note for the Maniac fork spec.
- Also recognized as deliberate no-ops by 220325: **3024** (returns true; not the unknown-code
  default) and **3027 `AddMoveRoute`** (consumed by lookahead inside the MoveEvent/11330 handler
  `FUN_004140c0`, not a standalone dispatch) — br_dispatch_findings.md §"Notable dispatch facts".

---

## 4. Detection

How to recognise a BR install (and, generally, a Maniac-220325 game) without running it:

| Signal | Where | Meaning |
|---|---|---|
| VERSIONINFO `Maniacs, v220325` | `BelovedRapture.exe` (UTF-16) | Maniac Patch present + version. **Primary detector.** |
| `WinW=426` / `WinH=240` | `RPG_RT.ini [RPG_RT]` | Maniacs custom resolution → 426×240 widescreen |
| `Font/*.fon` + `EXFONT.bmp` | `Font/` dir | Maniacs font auto-load (see [§8](#8-runtime-behavior-changes)) |
| `Save.lgs` ("LcfGlobalSave") string | exe | Maniacs global-save (though BR uses only the save/load-info subset) |
| `accord.dll` | game dir | Generic Maniac DLL sniff (fallback; not BR-specific) |

**No `accord.dll` reliance needed for BR** — BR ships a self-contained Maniacs exe and is
detected from the exe VERSIONINFO.

### EasyRPG mapping

- **Config flag:** `Game_ConfigGame::patch_maniac` — `ConfigParam<int> patch_maniac{ "Maniac Patch", "", "Patch", "Maniac", 0 }` (`src/game_config_game.h:45`). `Player::IsPatchManiac()` returns `patch_maniac.Get() > 0` (`src/player.h:519`–`:525`). Value `2` means "enable but do **not** widen variable ranges to 32-bit" (`src/player.cpp:1505`; `src/game_constants.cpp:28`,`:37`); BR wants the default `1`.
- **Autodetect:** `src/player.cpp:798`–`:802` reads the exe VERSIONINFO via `EXEReader::FileInfo::GetEngineType(maniac_patch_version)` (`src/exe_reader.cpp:483`) and, unless `patch_override` is set, calls `game_config.patch_maniac.Set(maniac_patch_version)`. The version is parsed from the literal `Maniacs, v` UTF-16 string at `src/exe_reader.cpp:391`–`:397` (`atoi` of the 6 digits → `220325`). A non-zero parse enables the patch. As a fallback, `accord.dll` presence also sets `patch_maniac = 1` (`src/player.cpp:853`–`:855`).
- **CLI:** `--patch-maniac [N]` (`src/player.cpp:1503`); `--no-patch` / `--no-patch-maniac` disable. Setting any patch flag sets `patch_override`, which disables autodetection (see [../forks/easyrpg-extensions.md](../forks/easyrpg-extensions.md)).
- **Note (RE-pending):** EasyRPG currently collapses the parsed Maniac version to the *enable* boolean plus the `==2` variable-range switch; it does **not** gate behavior by the literal `220325` vs `210414`. For BR this is fine (BR uses no command absent from 210414 except none — its 11 commands all exist in both), but the `Zoom`-absent / `3022`-present distinctions of 220325 are not modeled. (hypothesis: irrelevant to BR because BR emits neither.)

---

## 5. Event commands

### 5.1 Empirical Maniac-command census (the load-bearing table)

Scanned with liblcf (`LDB_Reader`/`LMU_Reader`) across **all 545 maps, 400 common events,
1,830 troop pages — 533,201 event commands** (command_usage.md). Result: **11 distinct Maniac
commands, 4,450 instances, zero codes unknown to liblcf, zero parse failures.** Detected
encoding `ibm-5348_P100-1997`.

Anchor convention: dispatch anchors point at the `case Cmd::Maniac_*:` label in EasyRPG's
`ExecuteCommand` switch; func anchors point at the handler definition. The `N` in
`CmdSetup<…, N>` is EasyRPG's *minimum* parameter count for the command (asserted at dispatch).
Binary handler is the 220325 `BelovedRapture.exe` address (br_dispatch_findings.md).

| Code | Command (liblcf enum) | Count | Heaviest user(s) | BR handler | EasyRPG handler (`N` min-params) |
|---:|---|---:|---|---|---|
| 3007 | `Maniac_ShowStringPicture` | **2,273** | CE19 'LongRange1xHandler', CE46 'Enemy Scan TEST' | `FUN_00443b50` | `CommandManiacShowStringPicture` `src/game_interpreter.cpp:4568` (dispatch `:796`, `N=23`) |
| 3020 | `Maniac_ControlStrings` | **1,518** | CE12 'Auto-Save', CE20 'Store&RemoveItems/Gold' | `FUN_00426720` | `CommandManiacControlStrings` `src/game_interpreter.cpp:5052` (dispatch `:812`, `N=8`) |
| 3012 | `Maniac_GetBattleInfo` | 274 | CE1 'BattEndHandler', CE4 'Pre-Battle Call 2' | `FUN_00412860` | `CommandManiacGetBattleInfo` `src/game_interpreter_battle.cpp:846` (dispatch `:258`, `N=5`) |
| 3011 | `Maniac_ChangeBattleCommandEx` | 238 | Troops 2/5/6 page 1 | `FUN_00412b90` | `CommandManiacChangeBattleCommandEx` `src/game_interpreter_battle.cpp:810` (dispatch `:256`, `N=2`) |
| 3010 | `Maniac_ControlAtbGauge` | 110 | CE19 'LongRange1xHandler' | `FUN_00412bf0` | `CommandManiacControlAtbGauge` `src/game_interpreter_battle.cpp:723` (dispatch `:254`, `N=7`) |
| 3018 | `Maniac_SetGameOption` | 16 | CE9 '+GPConfiguration+', CE16 'GameSpeed=====' | `FUN_0044ecd0` | `CommandManiacSetGameOption` `src/game_interpreter.cpp:5034` (dispatch `:810`, `N=4`) |
| 3001 | `Maniac_GetSaveInfo` | 7 | CE77 'CMSGetSaveInfo' | `FUN_00436ce0` | `CommandManiacGetSaveInfo` `src/game_interpreter.cpp:4398` (dispatch `:784`, `N=12`) |
| 3003 | `Maniac_Load` | 7 | CE81 'CMSConfirmLoad' | `FUN_00437510` | `CommandManiacLoad` `src/game_interpreter.cpp:4498` (dispatch `:786`, `N=3`) |
| 3009 | `Maniac_ControlBattle` | 4 | CE39 'BeginBattle=====' | `FUN_00412f00` | `CommandManiacControlBattle` `src/game_interpreter_battle.cpp:706` (dispatch `:252`, `N=4`) |
| 3002 | `Maniac_Save` | 2 | CE12 'Auto-Save', CE78 'CMSConfirmSave' | `FUN_00437390` | `CommandManiacSave` `src/game_interpreter.cpp:4478` (dispatch `:788`, `N=3`) |
| 3008 | `Maniac_GetPictureInfo` | 1 | Map0157 event 237 'clouds 2' page 2 | `FUN_0044a850` | `CommandManiacGetPictureInfo` `src/game_interpreter.cpp:4731` (dispatch `:798`, `N=8`) |

(CE = common event; counts from command_usage.md; BR handler addresses from br_dispatch_findings.md.)
Implementation priority for BR therefore
follows weight: **string pictures (3007) and string variables (3020) first**, then the
**battle trio (3010/3011/3012)**, then SetGameOption and save/load-info. Each command's full
parameter layout is owned by [maniac-patch-commands.md](../forks/maniac-patch-commands.md);
this dossier records only BR-specific usage and verification notes (§5.3).

**Maniac commands BR does NOT use** (so EasyRPG need not be correct on them to run BR, but they
must not crash if liblcf preserves an unexpected one): `ControlVarArray` (3013),
`KeyInputProcEx` (3014), `RewriteMap` (3015), `ControlGlobalSave` (3016), `ChangePictureId`
(3017), `CallCommand` (3019), `GetGameInfo` (3021), `EditPicture`/`WritePicture` (3025/3026),
`AddMoveRoute` (3027), `EditTile` (3028), `ControlTextProcessing` (3029),
`GetMousePosition`/`SetMousePosition` (3005/3006), `Zoom` (absent from the build entirely).

### 5.2 Official RM2k3 v1.10+ commands BR uses (BASELINE, not Maniacs)

These are Steam-era v1.10+ codes, classified BASELINE by liblcf; their presence is consistent
with the v1.12a runtime, **not** a Maniacs addition (command_usage.md analyst notes):

| Code | Command | Count | EasyRPG |
|---:|---|---:|---|
| 5002 | `ExitGame` | 5 | implemented |
| 5003 | `ToggleAtbMode` | 1 | implemented |
| 5004 | `ToggleFullscreen` | 2 | implemented |
| 5005 | `OpenVideoOptions` | 6 | implemented |

The full baseline histogram (105 distinct codes, heaviest: `Wait` 69,484; `ShowPicture`
33,944; `MovePicture` 42,235; `ControlVars` 48,100; `MoveEvent` 33,046) is in
command_usage.md and is unremarkable stock RM2k3 — every code carries a liblcf enum, so liblcf
fully covers BR's command vocabulary.

### 5.3 BR-specific verification notes per command (RE-pending semantics)

- **3007 `Maniac_ShowStringPicture`** — BR's entire custom UI (menus, battle HUD, enemy-scan
  overlays) is string-picture-driven; this is the single most important command for BR.
  Binary handler `FUN_00443b50` (br_dispatch_findings.md). EasyRPG layout uses 23 params
  (`CmdSetup<…, 23>`, `src/game_interpreter.cpp:797`), with position/transform bitmasks decoded
  via `ManiacBitmask` (`src/game_interpreter.cpp:2782`,`:2804`). **(RE-pending)** confirm BR's
  sub-op set (text source modes, font selection by name `BRTinyFont`) matches EasyRPG's
  decoder; font metrics are a known gap ([§9](#9-easyrpg-support-matrix)).
- **3020 `Maniac_ControlStrings`** — string-variable engine (assign/concat/toNum, actor-name
  /-desc reads, ins/rep/subs/join, extract/hex). Handler `FUN_00426720`. EasyRPG decodes the
  per-arg evaluation-mode bitfields documented inline at `src/game_interpreter.cpp:5056`–`:5087`.
  EasyRPG 0.8.1 string-variable work is what made the **demo playable**
  (<https://blog.easyrpg.org/2025/04/easyrpg-player-0-8-1-stun/>). **(RE-pending)** BR's exact
  op mix and any file-I/O string ops (see [§7](#7-file-format-and-on-disk-changes) re `Text/`).
- **3010/3011/3012 (battle trio)** — `ControlAtbGauge` (`FUN_00412bf0`),
  `ChangeBattleCommandEx` (`FUN_00412b90`), `GetBattleInfo` (`FUN_00412860`). These drive BR's
  animated-enemy, custom-HUD battle system. EasyRPG handlers live in
  `src/game_interpreter_battle.cpp` (anchors in §5.1) and were written against **Maniacs
  210414** behavior (PR #3295). **(RE-pending)** invocation order and ATB interaction at
  battle start/parallel — the open question behind the still-"missing event command" and combat
  issues ([§9](#9-easyrpg-support-matrix), [§11](#11-open-questions)).
- **3009 `Maniac_ControlBattle`** — only 4 uses (CE39 'BeginBattle'), but central to combat
  flow. Handler `FUN_00412f00`; EasyRPG `CommandManiacControlBattle`
  (`src/game_interpreter_battle.cpp:706`) and the `ManiacBattleHook`/`ProcessManiacSubEvents`
  machinery (`src/game_battle.cpp:236`,`:245`) implement it (PR #3295, motivated explicitly by
  BR).
- **3018 `Maniac_SetGameOption`** — 16 uses. EasyRPG implements only **operation 2 (Change
  Picture Limit) as a no-op** (EasyRPG supports unlimited pictures); all other operations log
  `Maniac SetGameOption: Operation {} not supported` (`src/game_interpreter.cpp:5042`–`:5046`).
  **3018 is the prime suspect for the 0.8.1 "missing event command"** (see §9): BR calls it 16×
  from `+GPConfiguration+`/`GameSpeed` config events, and any operation other than `2` is
  currently unhandled. **(RE-pending)** enumerate which `operation` values BR actually passes
  (decode the 16 call sites; handler `FUN_0044ecd0`).
- **3001/3002/3003 (save/load-info)** — `GetSaveInfo` (`FUN_00436ce0`), `Save`
  (`FUN_00437390`), `Load` (`FUN_00437510`). Used by BR's custom save/load menu (CE77/78/81).
  EasyRPG: dispatch at `src/game_interpreter.cpp:784`–`:788`. Known BR-specific bug:
  GetSaveInfo faceset transparency (issue #3128, fixed in PR #3293).
- **3008 `Maniac_GetPictureInfo`** — a single stray use on a map (cloud parallax). Low risk.

---

## 6. Modified baseline commands

BR does not appear to use Maniacs *extensions* of baseline commands beyond what the census
captures: all move routes are 100% standard opcodes 0–41 (no Maniacs move-route opcodes;
command_usage.md move-route histogram), and there are zero UNKNOWN codes. The one
baseline-behavior subtlety relevant to BR is **lenient handling of intentionally-invalid IDs**:
BR deliberately passes invalid Item/Actor/Event IDs to mean "blank" and **relies on RPG_RT
silently falling back** rather than showing the alert dialog. EasyRPG had to add conditional-
branch indirection for Item/Actor/Event IDs and suppress those alerts (issue #3128; fixed in
0.8.1, <https://blog.easyrpg.org/2025/04/easyrpg-player-0-8-1-stun/>). This is a *faithful-bug*
requirement: emulate RPG_RT's silent fallback, not its alert.

Everything else BR uses is stock RM2k3 v1.12a. (Section kept per TEMPLATE; BR exercises no
other modified baseline command.)

---

## 7. File-format and on-disk changes

BR introduces **no new LCF chunks** that break liblcf: the entire game parses cleanly
(command_usage.md "Parse failures: None"). Two cosmetic liblcf-model gaps were observed and are
harmless to BR:

- **LDB State chunk `0x28`** skipped 22× (1-byte payload each) — a small unmodeled State flag
  (liblcf's `ChunkState` enum gaps between `0x27 battler_animation_id` and `0x29 restrict_skill`).
- **Map0169.lmu** has one corrupted `EventPage` chunk `0x22` (`layer`, declared size 1, actual
  2 bytes); liblcf recovers with a warning.

Neither affects the command census or runtime command execution.

### Non-standard files vs stock RM2k3

| File / dir | Status | Resolution |
|---|---|---|
| `RPG_RT.edb` (52 MB XML) | **Dev-time artifact, runtime-inert** | XML dump of `RPG_RT.ldb` produced by EasyRPG's own **`lcf2xml.exe`** (liblcf utility; `.edb` is liblcf's XML-DB convention). The Launcher runs it on every boot via `extract.bat`. **No `.edb` reference in the engine binary** (br_dispatch_findings.md "No references to `.edb`"). Engine never reads it. |
| `Text/*.txt` (~73 tables, UTF-8 BOM) | **Dev-time artifact, runtime-inert** | Output of `Parser.exe` (consumes `RPG_RT.edb`, writes tables: `actors_*.txt`, `enemies_atk.txt`, `items_description.txt`, …). Direction is **LDB→XML→TXT extraction**, not TXT→LDB compile. Binary grep over LDB + all 545 LMUs finds **zero** occurrences of `txt`/`Text/`/`edb` in event data. The engine *does* reference a `\Text\` output dir (br_dispatch_findings.md @ 0x0054d938) but only as the target of Maniacs file-output commands, which BR's data never invokes. |
| `RPG_RT2.lmt` (6,076 B) | **Vestigial** | 100% zero bytes; no engine reference; no public mention. Safe to ignore. |
| `Beloved_Rapture.r3proj` (16 B) | Editor marker | Stock Steam RM2k3 project marker; contents `RPG2003 v1.12a`. Not read by engine (no `.r3proj` reference). |
| `Save.lgs` ("LcfGlobalSave") | Maniacs global save | Engine references it; BR uses only save/load-info commands, so likely minimal/unused at runtime. (UNVERIFIED whether BR writes it.) |
| `Movie/` | Empty | No FMV shipped; movie playback is a non-issue for BR. |
| `Font/` | Custom fonts | `BelovedRapture.fon`, `BRTinyFont.fon` (Windows bitmap-font containers), `EXFONT.bmp` (extended glyph sheet). See [§8](#8-runtime-behavior-changes). |
| `Sources/` | Dev C# tooling | `Launcher.sln`, `Parser.sln`, `Parser.exe`, exported tables. Ships with the game but is build-time/data tooling. |

### Dev toolchain (confirmed dev-time, runtime-inert)

The retail boot path is: **`Launcher.exe` → `extract.bat` → `lcf2xml.exe RPG_RT.ldb >
RPG_RT.edb` → `Text\Parser.exe` (writes `Text/*.txt`) → `Game.exe`** (the Maniacs runtime).
`Launcher/Program.cs` shows a splash form, synchronously runs `extract.bat`, then
`Process.Start("Game.exe", args)` (web dossier §3.1). `extract.bat`:

```bat
@echo off
.\lcf2xml.exe .\RPG_RT.ldb > nul 2> nul
.\Text\Parser.exe > nul 2> nul
```

**Consequence for EasyRPG:** launching the game directory **directly** (bypassing
`Launcher.exe`) loses nothing — `RPG_RT.ldb` is the authoritative data source; `RPG_RT.edb`
and `Text/*.txt` are regenerated dev artifacts, not runtime inputs. EasyRPG should run BR from
the game folder and **ignore `RPG_RT.edb`, `Text/`, `RPG_RT2.lmt`, and `.r3proj` entirely**.
No Steamworks usage was found in `Launcher/Program.cs`.

### `RPG_RT.ini` surface

The Maniacs build's complete `[RPG_RT]` ini surface (single reader `FUN_004a3820`,
br_dispatch_findings.md):

| Key | Default | Clamp | BR value | EasyRPG |
|---|---|---|---|---|
| `GameTitle` | "Untitled" | — | `Beloved Rapture` | read `src/player.cpp:752` |
| `WinW`/`Winw` | 320 | 64–1920 | **426** | read `src/player.cpp:756` |
| `WinH`/`Winh` | 240 | 32–1440 | **240** | read `src/player.cpp:757` |
| `Encoding` | 0 | — | (auto-detected `ibm-5348`) | — |
| `RuntimePackageKey` | `KADOKAWA\rpg2003` | — | (default) | n/a |

Note: this build **does not read `FullPackageFlag`** (replaced by the
RuntimePackageKey/RuntimePackagePath registry mechanism), even though BR's ini sets
`FullPackageFlag=1`. EasyRPG *does* read `FullPackageFlag` (`src/player.cpp:754`) to suppress
the RTP warning — harmless divergence (it just means "no RTP needed").

---

## 8. Runtime behavior changes

### 8.1 Custom resolution — 426×240

BR runs at **426×240** (16:9 at 240p). `426 % 16 ≠ 0`, which specifically broke EasyRPG map
scrolling for maps whose width/height isn't a multiple of 16 (fixed in 0.8.1,
<https://blog.easyrpg.org/2025/04/easyrpg-player-0-8-1-stun/>). EasyRPG reads `WinW`/`WinH` into
`Player::screen_width`/`screen_height` and sets `Player::has_custom_resolution = true`
(`src/player.cpp:755`–`:758`), which downstream affects window sizing (`src/baseui.cpp:109`) and
title layout (`src/scene_title.cpp:56`). A related regression — "Wide/Ultra Wide screen
shaking" from `std::clamp` with negative `map_width - screen_width` — was caught and fixed
(#3333 → #3334); maps **smaller than** the 426-wide screen are entangled with BR's resolution
support and must be handled (clamp must tolerate negative slack).

### 8.2 Fonts

- **Loading:** the engine registers every file in `Font/` via `AddFontResourceExA` /
  `AddFontMemResourceEx` (br_dispatch_findings.md). BR ships **`.fon`** bitmap fonts
  (`BelovedRapture.fon`, `BRTinyFont.fon`) rather than the `.ttf`/`.otf` Maniacs documents; the
  LDB references face name **`BRTinyFont`** and a string `1.1.0 Font Fix` (web dossier §4.2).
  String pictures can select a font by name.
- **EasyRPG gap (KNOWN, open):** PR #3293 fixed BR menus but states *"What this doesn't fix is
  the wrong font size, guess they replaced the font in the exe with a different one. So the
  metrics are different."* (<https://github.com/EasyRPG/Player/pull/3293>). So **font-metrics
  parity** with BR's shipped/exe-embedded font is unresolved. EasyRPG 0.8.1 separately improved
  external-font handling (embedded bitmap glyphs at correct sizes, TTF/OTF support, settings
  font browser). To match BR exactly an engine needs `.fon` rasterization or a metrics-matched
  substitute. See discussion threaded off **upstream PR #3293**.
- **EXFONT:** `EXFONT.bmp` is BR's extended gaiji sheet; Maniacs removes the gaiji count limit.
  EasyRPG loads a custom ExFont from `Font/ExFont` (`src/player.cpp:773`) and from the exe
  (`Cache::exfont_custom`, `src/player.cpp:793`).

### 8.3 Variable ranges

Maniacs widens variable ranges to 32-bit. EasyRPG ties this to the patch value: with
`patch_maniac == 1` (BR's case) ranges are widened to `int32` min/max; `== 2` keeps the stock
2k3 range (`src/game_constants.cpp:28`–`:41`). BR should run with `1`. (UNVERIFIED that BR's
data relies on the widened range, but the default is correct.)

### 8.4 Audio / input / misc

- BR (and stock RM2k3) uses **Microsoft DirectSound**, which errors if no output device is
  present at launch (dev statement,
  <https://steamcommunity.com/app/2017620/discussions/0/695372304943705205/>). Under
  EasyRPG/SDL this is a non-issue (different audio stack), but it's relevant to Wine/Proton
  runs of the *original* exe.
- Devs deliberately did **not** implement action-key text fast-forward ("it broke all
  cutscenes with movements"); an in-game text-speed setting exists
  (<https://steamcommunity.com/app/2017620/discussions/0/595136999981126834/>). EasyRPG's own
  fast-forward could therefore desync BR cutscenes — (hypothesis) worth checking during
  verification.
- BR has **no random encounters** (store page); encounter steps are not a factor. The 573
  `EnemyEncounter` (10710) commands in the census (command_usage.md) are *event-scripted*
  battles, not encounter-step-driven randoms — consistent with "no random encounters".

### 8.5 Save semantics

BR uses `Save%02d.lsd` (stock) plus the Maniacs save/load-info commands for its custom
save/load UI. No evidence BR depends on `Save.lgs` global-save content (it does not use
`ControlGlobalSave` 3016). (UNVERIFIED.)

---

## 9. EasyRPG support matrix

Baseline release reference: **0.8.1 "Stun" (2025-04)** and **0.8.1.1 (2025-06-02)**; no newer
Player release as of 2026-06-09 (<https://blog.easyrpg.org/category/release/>). The 0.8.1
release post states plainly: *"While the demo of **Beloved Rapture** works, the full game still
has **a missing event command** and **issues with the combat system**."*
(<https://blog.easyrpg.org/2025/04/easyrpg-player-0-8-1-stun/>).

| Feature | State | Detail / anchor | PR / issue |
|---|---|---|---|
| LCF parsing (LDB + 545 LMU) | **FULL** | parses with zero fatal errors | command_usage.md |
| Maniac detection & enable | **FULL** | exe VERSIONINFO → `patch_maniac`; `src/player.cpp:798` | — |
| Custom resolution 426×240 | **FULL (0.8.1)** | `WinW/WinH` read `src/player.cpp:755`; scroll-rounding fixed | 0.8.1; #3333→#3334 |
| String variables (3020) | **PARTIAL→FULL for demo** | `src/game_interpreter.cpp:5052`; made demo playable | 0.8.1 |
| String pictures (3007) | **PARTIAL** | `src/game_interpreter.cpp:4568` | #3128, #3293 |
| Esc-menu suite (GetSaveInfo transparency, invalid-ID fallback, item/equip lists) | **FULL (0.8.1)** | menu fixes | #3128 → **PR #3293** (merged 2024-11-18) |
| SetGameOption — Picture Limit (op 2) | **FULL** | no-op (unlimited pics) `src/game_interpreter.cpp:5043` | 0.8.1 |
| SetGameOption — other operations | **MISSING** | logs "Operation {} not supported" `src/game_interpreter.cpp:5046`; **prime "missing event command" suspect** | (RE-pending) |
| Control Variables Maniac expr/indirection ops (#3128 "AssignInplace") | **FULL (0.8.1)** | Maniac-gated paths in `src/game_interpreter_control_variables.cpp` (`Player::IsPatchManiac()` branches `:131`,`:258`–`:280`) | #3128 |
| Battle hooks `ControlBattle` (3009) | **PARTIAL** | `src/game_interpreter_battle.cpp:706`; `ManiacBattleHook` `src/game_battle.cpp:236` | **PR #3295** (merged 2024-12-21) |
| Battle trio 3010/3011/3012 | **PARTIAL** | `src/game_interpreter_battle.cpp:723/810/846`; written vs 210414 | #3295 |
| Battle-start / battle-parallel common-event triggers | **BLOCKER (open)** | invocation order/ATB interaction unresolved; tracked in **upstream PR #3545** (not merged as of this writing) | PR #3545 (RE-pending) |
| Font metrics (`.fon` / exe-embedded) | **MISSING (WONTFIX in #3293)** | wrong glyph size; metrics differ | **PR #3293** note |
| CallCommand variants | n/a for BR | BR does not use 3019 | #3319 |
| Maniac_Zoom | n/a | absent from the 220325 build | — |

Meta-tracking: "Support Maniac Patch (Metabug)" <https://github.com/EasyRPG/Player/issues/1818>.
Third-party BR effort: **`MackValentine/BelovedRapture`** is a full EasyRPG Player source tree
(default branch `Maniacs-Battle`, created 2025-01-26) by a known Maniacs-feature author —
evidence of active work toward running BR (<https://github.com/MackValentine/BelovedRapture>).

### The "missing event command" — prime suspects (ordered)

1. **3018 `Maniac_SetGameOption`** with a non-`2` operation (config/GameSpeed events; 16 uses;
   only op 2 implemented). **Strongest suspect** — it is a Maniac command BR genuinely emits
   and EasyRPG genuinely no-ops/warns. *(hypothesis; confirm by decoding BR's 16 call-site
   operation values.)*
2. A battle-context use of the **3010/3011/3012 trio** whose 220325 semantics diverge from the
   210414 reference EasyRPG implemented — consistent with the co-reported "combat-system
   issues." *(RE-pending.)*

Note the 0.8.1 blog does **not** name the command; identifying it definitively is a goal of the
verification pass ([§11](#11-open-questions)).

---

## 10. Test assets

The game itself is the asset (commercial, non-redistributable): corpus copy at
`/mnt/c/rg/easyrpg_library/beloved_rapture/` (full game) — exercises string pictures, string
variables, the Maniacs battle trio, custom resolution, and the custom save/menu system. The
**free demo** (Steam/GOG) is the lighter-weight target that 0.8.1 already runs and is the right
first smoke test. For isolated Maniac-command tests unrelated to BR's copyrighted content, use
the synthetic projects referenced by [maniac-patch-commands.md](../forks/maniac-patch-commands.md).

### Concrete TEST PLAN (run BR under our built Player, ordered by census weight)

Run from the game directory (bypass `Launcher.exe`), `--patch-maniac 1`, forced resolution
426×240. Verify feature-by-feature, heaviest census first:

1. **String pictures (3007, 2,273×)** — boot to title and main menu; confirm all custom HUD/menu
   string-picture text renders, positioned correctly, with the `BRTinyFont` face. *Expect a
   known font-metrics mismatch (sizes off) per PR #3293 — record it, don't treat as new.*
2. **String variables (3020, 1,518×)** — open Esc menu → quests/guides, inventory, store/remove
   gold flows (CE20); confirm strings build correctly (no garbled/empty labels). Re-test the
   #3128 "Guides → Quests" crash path specifically.
3. **Battle trio + ControlBattle (3012/3011/3010/3009)** — enter combat; verify ATB gauge
   manipulation, battle-command swaps, animated-enemy hooks, HP-change popups, target selection.
   **Watch battle start and any battle-parallel common events** (CE1 'BattEndHandler',
   CE4 'Pre-Battle Call 2', CE39 'BeginBattle') — this is the suspected combat-issues / PR #3545
   region. Note invocation order vs RPG_RT.
4. **SetGameOption (3018, 16×)** — open the config/GameSpeed menus (CE9 '+GPConfiguration+',
   CE16 'GameSpeed'); **capture every `operation` value passed**; confirm none hit the
   "Operation {} not supported" warning. This directly tests the "missing event command"
   hypothesis.
5. **Save/Load (3001/3002/3003)** — exercise the custom save/load menu (CE77/78/81); verify
   save-slot info (faceset transparency per #3128), save, reload.
6. **GetPictureInfo (3008, 1×)** — visit Map0157 (cloud parallax) and confirm no warning.
7. **Resolution/scrolling** — traverse a map narrower/shorter than 426×240 and a large map;
   confirm no scroll-rounding artifacts and no screen-shake clamp regression (#3333).
8. **Invalid-ID fallback** — trigger the menu paths that intentionally pass invalid Item/Actor/
   Event IDs; confirm silent fallback (no alert dialog) per #3128.

Capture any `Output::Warning` lines (especially "Maniac … not supported") as the actionable
gap list.

---

## 11. Open questions

1. **Identity of the 0.8.1 "missing event command."** Prime suspect **3018 SetGameOption**
   (non-`2` operation); decode the 16 BR call sites' `operation` values (handler `FUN_0044ecd0`)
   and cross-check against EasyRPG's `CommandManiacSetGameOption`. (RE-pending.)
2. **Battle-start / battle-parallel common-event semantics** (CE1/CE4/CE39 and troop pages):
   exact invocation order and ATB interaction in 220325 — the data EasyRPG PR #3545 needs to be
   correct for BR. (RE-pending; handlers `FUN_00412860/00412b90/00412bf0/00412f00`.)
3. **3007/3020 sub-op parity:** confirm BR's actual `ShowStringPicture` sub-ops and
   `ControlStrings` op mix match EasyRPG's decoders (drive from Ghidra handlers `FUN_00443b50`/
   `FUN_00426720` + EasyRPG source diff). (RE-pending.)
4. **Font metrics:** `.fon` bitmap-font load semantics and the metrics of BR's exe-replaced
   default font, to fix the PR #3293 size mismatch. (Open.)
5. **`Save.lgs` use:** does BR write/read the Maniacs global save at runtime despite not using
   `ControlGlobalSave`? (UNVERIFIED.)
6. **Version gating:** should EasyRPG model the 220325-vs-210414 distinction (Zoom absent, 3022
   present)? Irrelevant to BR (emits neither) but matters for "support all Maniac builds".
   (hypothesis.)

---

## 12. References

Primary RE / corpus (this repo's research tree):
- `/mnt/c/rg/easyrpg_library/beloved_rapture/ANALYSIS.md` — identification, census summary, file census.
- `/mnt/c/rg/easyrpg_library/beloved_rapture/command_usage.md` — full command census (533,201 commands).
- `/home/john/re/br_dispatch_findings.md` — `BelovedRapture.exe` dispatcher (`FUN_004500d0`), 147 dispatched IDs, ini/registry/font surface; Ghidra project `/home/john/re/ghidra-projects/BR`.
- `/home/john/research/beloved_rapture_web.md` — web identity, dev, tooling, compatibility reports.
- `/home/john/research/vt_hash_checks.md` — binary safety lookups.

EasyRPG source anchors (this tree): `src/player.cpp:798` (Maniac autodetect), `src/exe_reader.cpp:391`/`:483` (version parse), `src/game_config_game.h:45` (`patch_maniac`), `src/player.h:519` (`IsPatchManiac`), `src/game_interpreter.cpp:4568`/`:5034`/`:5052` (ShowStringPicture/SetGameOption/ControlStrings), `src/game_interpreter_battle.cpp:706`–`:861` (battle commands), `src/game_battle.cpp:236` (`ManiacBattleHook`).

Fork specs (siblings): [maniac-patch.md](../forks/maniac-patch.md), [maniac-patch-commands.md](../forks/maniac-patch-commands.md), [maniac-patch-fileformats.md](../forks/maniac-patch-fileformats.md), [easyrpg-extensions.md](../forks/easyrpg-extensions.md).

External (archive links where rot-prone):
- Steam: <https://store.steampowered.com/app/2017620/Beloved_Rapture/>; appdetails API <https://store.steampowered.com/api/appdetails?appids=2017620>.
- GOG: <https://www.gog.com/en/game/beloved_rapture>.
- EasyRPG 0.8.1 release: <https://blog.easyrpg.org/2025/04/easyrpg-player-0-8-1-stun/>.
- EasyRPG issues/PRs: [#3128](https://github.com/EasyRPG/Player/issues/3128), [#3293](https://github.com/EasyRPG/Player/pull/3293), [#3295](https://github.com/EasyRPG/Player/pull/3295), [#3319](https://github.com/EasyRPG/Player/pull/3319), [#3333](https://github.com/EasyRPG/Player/issues/3333), [#1818 metabug](https://github.com/EasyRPG/Player/issues/1818).
- Maniac Patch (BingShan, JP): <https://bingshan1024.github.io/steam2003_maniacs/>; EasyRPG "Known patches": <https://wiki.easyrpg.org/development/technical-details/known-patches>.
- RMN dev blogs: <https://rpgmaker.net/games/8/blog/24503/>, <https://rpgmaker.net/games/8/blog/24825/>, <https://rpgmaker.net/games/8/blog/25981/>.
- Third-party EasyRPG/BR tree: <https://github.com/MackValentine/BelovedRapture>.
