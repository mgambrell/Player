<!--
Spec: Runtime micro-patches (Makerpendium families). Part of the docs/forks/ series.
See TEMPLATE.md for section order and README.md for the registry / conventions.
-->

# Runtime micro-patches (Makerpendium families)

> A large family of tiny **behavioral binary patches** of `RPG_RT.exe` for RPG Maker 2000 and
> 2003 — collected and documented (with per-build x86-ASM offsets and raw bytes) in the
> [Makerpendium RPG_RT.exe Patch Database](https://dev.makerpendium.de/docs/patch_db/main-en.htm)
> (currently 117 entries). Unlike the major scripting forks (Maniac Patch, DynRPG, Destiny,
> PowerMode — covered in their own specs), these patches **add no new event-command codes**. Each
> one either (a) repurposes a fixed range of in-game **variables/switches as a control protocol**
> that an unmodified game data file can already write/read, or (b) silently changes a **fixed engine
> behavior** (stat caps, defend math, refresh suppression). A game "uses" such a patch simply by
> shipping a patched `RPG_RT.exe` and writing the agreed variables/switches; the `.ldb`/`.lmu` data
> is ordinary. EasyRPG Player reimplements about a dozen of these as `Game_ConfigGame` flags so the
> same games run unpatched. This spec is the master reference for that family.

**Status at a glance:** EasyRPG support = **PARTIAL** (≈11 of 117 patches modelled). Detection =
mostly **manual** (`EasyRPG.ini [Patch]` / `--patch-*`), with two byte-signature autodetections
(StatDelimiter, WhiteDragon) in `exe_reader.cpp` and DLL-sniffing for the DLL-bearing forks. Spec
confidence = **HIGH** for the implemented patches (cross-checked against source), **MEDIUM** for the
~90 catalogued-but-unimplemented patches (documented from the Patch-DB only). **Release split:** only
AntiLagSwitch, DirectMenu, CommonThisEvent, PicUnlock and RPG2k3Commands shipped in the **0.8.1**
release; the variable/switch protocol patches (ERA, MonSca, EXPlus, GuardRevamp) and the EXE-cap
autodetects (StatDelimiter, WhiteDragon) are **master-only** (post-0.8.1 — the whole
`game_runtime_patches.*` module and `EXEReader::GetOverriddenGameConstants` postdate the 0.8.1 tag).
See §9.

Author voice note: throughout, "the original patch" means the IPS/HPA distributed via Makerpendium;
"EasyRPG" means this repo's reimplementation. Variable/switch defaults quoted are the original
patches' hardcoded IDs unless stated otherwise — EasyRPG makes them configurable.

## 1. Identity

- **Family name:** RPG_RT micro-patches / behavioral patches. JP scene context: the classic Japanese
  community avoided binary patching (it breached ツクールの規約 / *the Tkool EULA*) and hacked **game
  data** instead (see §6, §7); the binary-patch culture is predominantly German/Western plus the
  modern JP author **炬燵あきら / KotatsuAkira**.
- **Recurring authors:** **bugmenot** (battle/menu/stat protocol patches: MonSca, EXPlus,
  GuardRevamp, AntiLag, Direct Menu, …), **Cherry** (David Trapp — UnlockPics, PicPointer,
  CommonThisEvent, BetterAEP, many bugfixes), **KotatsuAkira / 炬燵あきら** (Encounter Randomness
  Alert, EXtraFONT, GameWindowSuperScale, PicPointer Restruct, …), **elvissteinjr**, **Miroku**
  (Auto Enter Patch), **Seena** (RM2k3++ / RM Limit Changer).
- **License / redistribution:** patches are distributed as **IPS** (some as **HPA** = *Hyper Patch
  Archive*, Cherry's multi-version container, applied with Hyper Patcher 2). They patch a copyrighted
  Enterbrain binary, so the patch *bytes* (not the binary) are what gets shared. EasyRPG ships only
  reimplemented behavior + detection signatures, not patch bytes or RPG_RT.
- **Distribution:** primarily the Makerpendium Patch-DB index
  (<https://dev.makerpendium.de/docs/patch_db/main-en.htm>); many KotatsuAkira IPS mirrored on
  archive.org as `RPGMakerPatch_<name>` items; Cherry's at cherrytree.at; wiki categories at
  [RM2k (84)](https://www.makerpendium.de/wiki/Kategorie:Patch_(RPG_Maker_2000)) and
  [RM2k3 (127)](https://www.makerpendium.de/index.php/Kategorie:Patch_(RPG_Maker_2003)).
- **Liveness:** the Patch-DB is actively maintained and announced an offline package "soon"; new
  patches still appear (KotatsuAkira itch.io: <https://kotatsuakira.itch.io/>). Many original
  download URLs are dead — prefer archive.org / Patch-DB mirrors.

## 2. Target engine builds

These patches are per-build: each Patch-DB page carries a **compatibility matrix** of the exact
`RPG_RT.exe` builds it has been ported to. The two western modding baselines are **RM2k 1.07**
(`2000-12-27`) and **RM2k3 1.08** (`1.0.8.0`, `2003-09-22`); ports to **Steam RM2k 1.6x** and **RM2k3
1.10/1.11/1.12a** exist for some. Because a patch is byte-offset-specific, the same logical patch has
**different offsets per build** — see the per-page matrices, and the `code_size`/`code_ofs`
discriminators EasyRPG uses (§4). Most of these patch an **existing** `RPG_RT.exe` in place (IPS);
the cap-raising ones (RM2k3++ / 100-Pics) ship a **replacement** `RPG_RT.exe`.

EasyRPG does not need the byte offsets to *run* a patched game — it reimplements the behavior and
keys it off the configured flag — but it does use the offsets for the two **autodetected** stat-cap
patches (§4, §7).

## 3. Version lineage

This is a *family*, not one product, so the table tracks the patches themselves rather than dated
builds of a single binary. "Status" = EasyRPG support level. (Build dates of the underlying RPG_RT
are in [official-versions.md](official-versions.md).)

| Patch (Makerpendium id) | Author | Target builds | EasyRPG status |
|---|---|---|---|
| AntiLag (Fast/Slow/Switch/Switch+) | bugmenot | RM2k, RM2k3 | Switch variant: **FULL** (flag) |
| Direct Menu Patch | bugmenot | RM2k, RM2k3 | **FULL** (flag, PR #3192) |
| Encounter Randomness Alert (+MEPR) | KotatsuAkira | RM2k, RM2k3 | **FULL** (flag, PR #3378) |
| MonSca (+Plus) | bugmenot | RM2k, RM2k3 | **FULL** (flag, PR #3378) |
| EXPlus (+[+]) | bugmenot | RM2k, RM2k3 | **FULL** (flag) |
| GuardRevamp | bugmenot | RM2k, RM2k3 | **FULL** (flag) |
| CommonThisEventPatch | Cherry | RM2k, RM2k3 (official in 2k3 1.12) | **FULL** (flag) |
| UnlockPics-Patch | Cherry | RM2k/RM2k3 (official in 2k ≥1.60) | **FULL** (flag) |
| StatDelimiter | bugmenot | RM2k 1.62, RM2k3 1.0.8.0/1.0.9.1 | **FULL** + **autodetect** (byte sig) |
| WhiteDragon (Italian 1.08 cap patch) | (Italian scene) | RM2k3 1.0.8.0 | **FULL** + **autodetect** (string `NoTitolo`) |
| "RPG2k3 commands in 2k" / in-battle Call-Event LDB hack | (data-level, JP scene) | data hack, not a binary patch | **FULL** (flag) |
| BetterAEP / CustomSaveLoad (V[3350]/V[3351] protocol) | Cherry | RM2k 1.07, RM2k3 1.08 | **NONE** (catalogued only) |
| PicPointerPatch / PicsInBattle | Cherry / KotatsuAkira | RM2k 1.07, RM2k3 1.08 (+Steam ports) | **NONE** |
| ~90 further micro-patches | various | RM2k/RM2k3 various | **NONE** (see §6.3) |

## 4. Detection

There is **no single detection mechanism** for this family. EasyRPG covers it three ways, in
priority order (auto-detection in `Player::CreateGameObjects`,
[`src/player.cpp:697`](../../src/player.cpp); the EXE/DLL detection block runs ≈`:740`–`:887`; the pipeline is documented in
[`/home/john/research/easyrpg_patch_inventory.md`](../../../research/easyrpg_patch_inventory.md) §1.2):

1. **Manual flag (the normal path for this family).** Almost all micro-patches are invisible in the
   data files (they only change behavior or repurpose generic variables), so the game's curator must
   declare them in `EasyRPG.ini [Patch]` or on the CLI. Setting **any** patch option (ini or CLI)
   sets `Game_ConfigGame::patch_override`
   ([`src/game_config_game.h:81`](../../src/game_config_game.h)), which **disables all
   auto-detection** — see [easyrpg-extensions.md](easyrpg-extensions.md). `RPG_RT.ini` is **not**
   consulted for patch selection.

2. **EXE byte-signature autodetect (two cap patches only).** `EXEReader::GetOverriddenGameConstants`
   ([`src/exe_reader.cpp:569`](../../src/exe_reader.cpp)–`:615`) switches on the PE `CODE`-section
   size and probes a build-specific offset for a marker:

   | Patch | `code_size` (build) | Probe offset (`code_ofs +`) | Marker bytes | Applied config |
   |---|---|---|---|---|
   | **StatDelimiter** | `0x9CC00` (RM2k 1.62) | `0x07DAA6` | `"XXX"` = 3× `POP EAX` (`0x58 0x58 0x58`) | `KnownPatchConfigurations::StatDelimiter` |
   | **StatDelimiter** | `0xC8E00` (RM2k3 1.0.8.0) | `0x09D279` | `"XXX"` | StatDelimiter |
   | **StatDelimiter** | `0xC9000` (RM2k3 1.0.9.1) | `0x09C5AD` | `"XXX"` | StatDelimiter |
   | **WhiteDragon** (Italian 1.08 caps) | `0xC8E00` (RM2k3 1.0.8.0) | `0x08EBE0` | string `"NoTitolo"` | `Rm2k3_Italian_WD_108` |

   These do not toggle a `patch_*` flag — they override `Game_Constants` directly (see §7,
   `src/game_constants.h:139`). The StatDelimiter marker is literally the three `POP EAX` opcodes the
   patch inserts (the source ASM uses the placeholder string `"XXX"` in EasyRPG to spell `0x58 0x58
   0x58`). Makerpendium: <https://dev.makerpendium.de/docs/patch_db/patches/stat_delimiter.htm>.

3. **DLL sniffing (for the DLL-bearing forks only — not this family).** `harmony.dll`,
   `dynloader.dll`, `accord.dll`, `Destiny.dll`, `warp.dll` set their respective flags
   (`src/player.cpp:843`–`865`). None of the §5/§6 micro-patches ship a DLL, so this never fires for
   them. (PowerMode 2003's `warp.dll` is the exception and is covered in
   [powermode2003.md](powermode2003.md).)

**Open detection gap:** ini-less byte patches (BetterAEP family, PicPointer, UnlockPics, AntiLag,
MonSca, …) cannot currently be auto-detected without a signature DB. EasyRPG issue
[#1182](https://github.com/EasyRPG/Player/issues/1182) proposes generating exe signatures from the
Patch-DB offset tables. (UNVERIFIED that this will be implemented.)

### 4.1 EasyRPG flag map (the master table)

Every implemented micro-patch, its `Game_ConfigGame` field, ini key, CLI flag, default
variable/switch IDs, the handler `file:line`, and the Makerpendium reference. All anchors verified
against the tree at branch `RISKY`.

| Patch (EN / Makerpendium id) | What it changes | Game signal/protocol | `Game_ConfigGame` field + anchor | ini `[Patch]` key / CLI | Default IDs | Handler anchor | Makerpendium |
|---|---|---|---|---|---|---|---|
| **AntiLagSwitch** (`antilag_switch`) | Suppresses per-frame event-page refresh while a switch is ON (kills "lag" from many parallel page-condition checks) | Game turns the switch ON/OFF | `patch_anti_lag_switch` (int = switch id) [`game_config_game.h:50`](../../src/game_config_game.h) | `AntiLagSwitch` / `--patch-antilag-switch SWITCH` | original: S[1000]; EasyRPG: explicit id required | `Game_Map::GetNeedRefresh` returns `false` when switch ON, [`game_map.cpp:1781`](../../src/game_map.cpp) | [antilag_switch.htm](https://dev.makerpendium.de/docs/patch_db/patches/antilag_switch.htm) |
| **DirectMenu** (`direct_menu`) | Calling the menu jumps straight to a subscreen | Before opening menu, game writes V[N]=subscreen (1 item/2 skill/3 equip/4 status/5 order; 2k: 1–3 only) and V[N+1]=actor (party slot 1–4; ≤−1 = DB actor id) | `patch_direct_menu` (int = base var id) [`game_config_game.h:51`](../../src/game_config_game.h) | `DirectMenu` / `--patch-direct-menu VAR` | original V[3326]/V[3327]; EasyRPG: explicit base var | `Game_Interpreter_Map::RequestMainMenuScene` reads V[N], V[N+1], [`game_interpreter_map.cpp:105`](../../src/game_interpreter_map.cpp) | [direct_menu.htm](https://dev.makerpendium.de/docs/patch_db/patches/direct_menu.htm) (PR [#3192](https://github.com/EasyRPG/Player/pull/3192)) |
| **EncounterRandomnessAlert (ERA / MEPR)** (`encounter_alert`) | Suppresses the random encounter; reports the troop instead so the game can run its own battle/escape logic | On a would-be random encounter: troop id → V[var], switch → ON, battle skipped, map refreshed | `patch_encounter_random_alert_sw`, `patch_encounter_random_alert_var` [`game_config_game.h:53`](../../src/game_config_game.h)–`:54` | `EncounterAlert.Switch`/`EncounterAlert.Var` / `--patch-encounter-alert [-sw S] [-var V]` | S[1018], V[3355] ([`game_runtime_patches.h:101`](../../src/game_runtime_patches.h)–`104`) | `EncounterRandomnessAlert::HandleEncounter` [`game_runtime_patches.cpp:236`](../../src/game_runtime_patches.cpp); called from [`game_map.cpp:1645`](../../src/game_map.cpp) | [encounter_alert.htm](https://dev.makerpendium.de/docs/patch_db/patches/encounter_alert.htm) (PR [#3378](https://github.com/EasyRPG/Player/pull/3378)) |
| **MonSca / MonScaPlus** (`mon_sca`) | Scales 10 enemy battle outputs by in-game variables | Game presets the scaling vars; optional level-formula switch | 12 params `patch_monsca_*` [`game_config_game.h:56`](../../src/game_config_game.h)–`:67` | `MonSca.MaxHP`…`MonSca.Plus` / `--patch-monsca [-maxhp N …]` | V[1001]–V[1010]; level switch S[1001]; Plus offset 0 ([`game_runtime_patches.h:131`](../../src/game_runtime_patches.h)–`145`) | `MonSca::Modify*` [`game_runtime_patches.cpp:282`](../../src/game_runtime_patches.cpp)–`340`; stat hooks in [`game_enemy.h:332`](../../src/game_enemy.h)–`362`; exp/gold/item in [`game_enemyparty.cpp:107`](../../src/game_enemyparty.cpp)–`137` | [mon_sca.htm](https://dev.makerpendium.de/docs/patch_db/patches/mon_sca.htm) |
| **EXPlus / EXPlus[+]** (`explus`) | Per-party-slot EXP multiplier; `[+]` also reports an actor's party slot | Game presets boost vars; `[+]` side-effect on actor-in-party branch | `patch_explus_var`, `patch_explusplus_var` [`game_config_game.h:69`](../../src/game_config_game.h)–`:70` | `EXPlus.VarExpBoost`/`EXPlus.VarActorInParty` / `--patch-explus [-var N]` | V[3333] (boost base), V[3332] (slot out) ([`game_runtime_patches.h:188`](../../src/game_runtime_patches.h)–`189`) | `EXPlus::ModifyExpGain`/`StoreActorPosition` [`game_runtime_patches.cpp:342`](../../src/game_runtime_patches.cpp)–`353`; called from [`scene_battle_rpg2k3.cpp:1874`](../../src/scene_battle_rpg2k3.cpp), [`scene_battle_rpg2k.cpp:888`](../../src/scene_battle_rpg2k.cpp), [`game_interpreter.cpp:3593`](../../src/game_interpreter.cpp) | [explus.htm](https://dev.makerpendium.de/docs/patch_db/patches/explus.htm) |
| **GuardRevamp** (`guard_revamp`) | Defend damage = configurable % instead of ÷2 / ÷4 | None (fixed behavior change) | `patch_guardrevamp_normal`, `patch_guardrevamp_strong` [`game_config_game.h:72`](../../src/game_config_game.h)–`:73` | `GuardRevamp.NormalDefense`/`GuardRevamp.StrongDefense` / `--patch-guardrevamp [-normal N] [-strong N]` | 50% / 25% ([`game_runtime_patches.h:222`](../../src/game_runtime_patches.h)–`223`) | `GuardRevamp::OverrideDamageAdjustment` [`game_runtime_patches.cpp:355`](../../src/game_runtime_patches.cpp); called from `algo.cpp:174` | [guard_revamp.htm](https://dev.makerpendium.de/docs/patch_db/patches/guard_revamp.htm) |
| **CommonThisEvent** (`common_this_event`) | "This Event" (id 10005) resolves inside common events | None (fixed behavior) | `patch_common_this_event` (bool) [`game_config_game.h:46`](../../src/game_config_game.h) | `CommonThisEvent` / `--patch-common-this` | n/a | `Game_Interpreter::GetThisEventId` walks the call stack when `IsPatchCommonThisEvent()`, [`game_interpreter.cpp:329`](../../src/game_interpreter.cpp) | [common_this_event.htm](https://dev.makerpendium.de/docs/patch_db/patches/common_this_event.htm) |
| **PicUnlock** (`UnlockPics`) | Picture Show/Move/Erase no longer blocked while a message box is open | None (fixed behavior) | `patch_unlock_pics` (bool) [`game_config_game.h:47`](../../src/game_config_game.h) | `PicUnlock` / `--patch-pic-unlock` | n/a | gated by `IsPatchUnlockPics()` in ShowPicture/MovePicture/ErasePicture, [`game_interpreter.cpp:2773`](../../src/game_interpreter.cpp), `:2920`, `:3075` | [issue #588](https://github.com/EasyRPG/Player/issues/588) (also official RM2k ≥1.60) |
| **StatDelimiter** | Raises HP/SP cap to 9,999,999 and base/battle stat caps to 999,999 | None (cap change) | overrides `Game_Constants` (not a flag) [`game_constants.h:151`](../../src/game_constants.h)–`156` | autodetect (byte sig) | n/a | `EXEReader::GetOverriddenGameConstants` [`exe_reader.cpp:592`](../../src/exe_reader.cpp)/`603`/`608` | [stat_delimiter.htm](https://dev.makerpendium.de/docs/patch_db/patches/stat_delimiter.htm) |
| **WhiteDragon** (Italian 1.08 caps) | Raises var range ±999,999,999; HP 99,999; SP/base-stat 9,999; damage 99,999; gold 9,999,999 | None (cap change) | overrides `Game_Constants` [`game_constants.h:141`](../../src/game_constants.h)–`149` | autodetect (string `NoTitolo`) | n/a | `EXEReader::GetOverriddenGameConstants` [`exe_reader.cpp:600`](../../src/exe_reader.cpp) | (Italian scene; see [bootlegs-translations.md](bootlegs-translations.md)) |
| **RPG2k3Commands** (data hack, not a binary patch) | Enables 2k3-only event commands & page conditions in 2k projects | The game's `.ldb`/`.lmu` were edited (external tools) to contain 2k3 commands / battle-event Call-Event | `patch_rpg2k3_commands` (bool) [`game_config_game.h:49`](../../src/game_config_game.h) | `RPG2k3Commands` / `--patch-rpg2k3-cmds`, `--patch-rpg2k3-commands` | n/a | `Player::IsRPG2k3Commands()` gates ~20 call sites (see §6.5) | [YADOT 改造データ](http://yado.tk/2k/01_shoshin/022_kaizou/) (data-level) |

## 5. Event commands

**Not applicable by definition.** These patches add **zero** new event-command codes — that is the
membership criterion for this spec (forks that add command codes live in
[maniac-patch.md](maniac-patch.md), [dynrpg.md](dynrpg.md), etc.). They operate entirely through (a)
the standard ControlVariables/ControlSwitches
commands writing reserved IDs, or (b) hooks in the engine's fixed code paths. The heading is kept per
template; see §6 for the baseline commands whose *behavior* changes.

## 6. Modified baseline commands

These patches change how **existing** RM2k/2k3 commands and engine paths behave. One subsection per
affected area; EasyRPG anchors verified against the tree.

### 6.1 ControlVariables / ControlSwitches as control protocols

Several patches do nothing until the game writes a reserved variable or switch with an ordinary
`ControlVariables` (10220) / `ControlSwitches` (10210) command, then a hook fires:

- **MonSca** — the game writes V[1001..1010] (scaling factors, parts-per-1000). On enemy stat read
  the engine multiplies: `val = val * V[id] / 1000`. When the level-scaling switch (S[1001]) is ON,
  the formula becomes `val = val * avg_party_level * V[id] / 1000`. The **Plus** variant offsets each
  variable id by the enemy's **troop member index** so each slot scales independently
  (`V[base + troop_index]`). EasyRPG: `ApplyScaling` / `GetVariableId`,
  [`game_runtime_patches.cpp:255`](../../src/game_runtime_patches.cpp)–`280`. Item-drop *id* is the
  one exception — it is **added**, not scaled: `item_id += V[1009]`
  (`ModifyItemGained`, [`game_runtime_patches.cpp:330`](../../src/game_runtime_patches.cpp)–`333`).
  Faithful replication detail:
  a scaling factor of **0 is treated as "no change"**, not "multiply by zero"
  ([`game_runtime_patches.cpp:271`](../../src/game_runtime_patches.cpp)).
- **EXPlus** — V[3333+slot−1] is read per party member as a percentage delta:
  `exp_gain = exp_gain * (100 + V[3333 + party_index]) / 100`
  ([`game_runtime_patches.cpp:342`](../../src/game_runtime_patches.cpp)). The **[+]** variant adds a
  *side effect to ConditionalBranch* (see §6.2).
- **AntiLagSwitch** — the game toggles S[1000] (or the configured switch). While ON the engine
  returns "no refresh needed" so parallel/automatic page-condition re-evaluation is skipped, which is
  what removes lag. EasyRPG: `Game_Map::GetNeedRefresh`
  ([`game_map.cpp:1781`](../../src/game_map.cpp)). The Patch-DB **Fast** variant suppresses refresh
  far more aggressively and is "absolutely incompatible to most already existing games"
  ([antilag_fast.htm](https://dev.makerpendium.de/docs/patch_db/patches/antilag_fast.htm)); EasyRPG
  only implements the **Switch** behavior.
- **DirectMenu** — the game writes V[3326]=subscreen and V[3327]=actor, then triggers an Open-Menu;
  the engine routes straight to that subscreen (see §6.3).

### 6.2 ConditionalBranch (12010) — EXPlus[+] side effect

The **[+]** variant of EXPlus piggybacks on the "is actor in party" sub-condition: whenever a
ConditionalBranch checks actor-in-party (parameter[2] == 0), the actor's current 1-based party slot
is written to V[3332]. EasyRPG: `RuntimePatches::EXPlus::StoreActorPosition(actor_id)` is invoked
exactly there, [`game_interpreter.cpp:3593`](../../src/game_interpreter.cpp); the value stored is
`party_position + 1` ([`game_runtime_patches.cpp:351`](../../src/game_runtime_patches.cpp)).

### 6.3 OpenMainMenu (11950) — DirectMenu

`Game_Interpreter_Map::RequestMainMenuScene` is the EasyRPG entry point. When `patch_direct_menu` is
set and the call's subscreen is unspecified (`subscreen_id == -1`), it reads the configured base var
V[N] for the subscreen id and V[N+1] for the actor selector
([`game_interpreter_map.cpp:105`](../../src/game_interpreter_map.cpp)–`111`). Subscreen ids (the
patch protocol): **1** inventory, **2** skills, **3** equipment, **4** status, **5** order (RM2k
supports only 1–3). Per-index detail in EasyRPG's `switch (subscreen_id)`
([`game_interpreter_map.cpp:115`](../../src/game_interpreter_map.cpp)–`174`): case **5** (order) is
gated on `Feature::HasRow()` and a party size > 1 (else falls through);
[`game_interpreter_map.cpp:155`](../../src/game_interpreter_map.cpp). EasyRPG additionally routes ids
**6** (Settings) and **7** (Language) to its own scenes — these are **not** part of the original
patch protocol, [`game_interpreter_map.cpp:168`](../../src/game_interpreter_map.cpp)–`173`. Actor
selector V[N+1]: positive = party slot (the engine maps 0/1/`>4` → first actor, then `--` to 0-based);
a **negative** value selects a database actor id (the engine takes `abs()` and sets `is_db_actor`).
Makerpendium
[direct_menu.htm](https://dev.makerpendium.de/docs/patch_db/patches/direct_menu.htm); EasyRPG PR
[#3192](https://github.com/EasyRPG/Player/pull/3192) ("as described in Makerpendiums patch
database").

### 6.4 Defend damage adjustment — GuardRevamp

Stock RPG_RT halves damage to a defending target (quarters it with the "strong defense" attribute).
GuardRevamp replaces the fixed ÷2/÷4 with `dmg = dmg * rate% / 100` for normal vs. strong defense.
EasyRPG `GuardRevamp::OverrideDamageAdjustment` returns `true` to skip the stock calc when at least
one rate is configured and the target is defending; a rate of **0 falls through** to stock behavior
for that defense class ([`game_runtime_patches.cpp:355`](../../src/game_runtime_patches.cpp)–`375`),
called from the damage algorithm (`algo.cpp:174`). Defaults 50/25 reproduce stock results.

### 6.5 2k3-only commands in 2k projects, and the in-battle Call-Event LDB hack

This is **not a binary RPG_RT patch** but a **data-level hack**, included here because EasyRPG models
it as a patch flag (`RPG2k3Commands`). The classic Japanese scene avoided patching `RPG_RT.exe`
(EULA) and instead edited the **game data** (`RPG_RT.ldb` / `.lmu`) with external editors to do
things the official editor forbids. YADOT's page 「改造した作品データで公開している一例」 (*examples of
publishing modified work-data*) documents the canonical artifacts — damage ≥1000, EXP-curve values
< 10, and **バトルイベントに「イベントの呼び出し」** (*"Call Event" placed inside battle events* — normally
impossible in the 2k editor) — under the warning 「改造は全て自己責任です！」 (*all modding is at your own
risk*) (<http://yado.tk/2k/01_shoshin/022_kaizou/>, HTTP-only Shift-JIS).

EasyRPG's `patch_rpg2k3_commands` makes the engine **dispatch 2k3-only event commands and honor
2k3-only page conditions even when the loaded project is a 2k project** — exactly what those LDB
hacks rely on. `Player::IsRPG2k3Commands()` is `true` for any real 2k3 engine **or** when the flag is
set ([`player.h:492`](../../src/player.h)). It gates ~20 sites, including:

- battle-event Conditional Branch turn/fatigue/command conditions
  ([`game_interpreter_battle.cpp:113`](../../src/game_interpreter_battle.cpp), `:126`, `:139`, `:169`);
- battle command handlers ([`game_interpreter_battle.cpp:268`](../../src/game_interpreter_battle.cpp),
  `:286`, `:334`);
- the `timer2` page condition ([`game_event.cpp:287`](../../src/game_event.cpp));
- extended parameter parsing on several map commands
  ([`game_interpreter.cpp:1575`](../../src/game_interpreter.cpp), `:2341`, `:2603`, `:2634`, `:2667`,
  `:3670`, `:4088`, `:4133`; [`game_interpreter_map.cpp:602`](../../src/game_interpreter_map.cpp)).

The flag is toggleable mid-game via EasyRPG's own `EasyRpg_SetInterpreterFlag` command (`@raw 2053`,
PR [#3123](https://github.com/EasyRPG/Player/pull/3123)); see [easyrpg-extensions.md](easyrpg-extensions.md).
liblcf loads/saves the 2k3 command codes losslessly inside a 2k data file regardless (unknown command
codes are preserved verbatim;
[`/home/john/research/easyrpg_patch_inventory.md`](../../../research/easyrpg_patch_inventory.md)
§9.3), so the hack survives round-tripping even without the flag — only **dispatch** needs the flag.

### 6.6 Catalogued-but-unimplemented protocols (reference)

The following families are documented in the Patch-DB and appear in corpus games but are **NOT**
implemented in EasyRPG. They are included so a developer can recognize the variable/switch usage and
extend support. (All MEDIUM confidence — sourced from the Patch-DB pages, not cross-checked against a
running binary.)

| Patch | Protocol | Makerpendium |
|---|---|---|
| **BetterAEP** | Title-skip; the "process-cancel" (EndEventProcessing) command, gated on V[3350], opens the file menu in load mode (1) or quits (2) | [better_aep.htm](https://dev.makerpendium.de/docs/patch_db/patches/better_aep.htm) |
| **CustomSaveLoadPatch** (BetterAEP addon) | Direct save/load to slot V[3351]; V[3352..3354] = savefile existence / leader level / HP | [custom_save_load.htm](https://dev.makerpendium.de/docs/patch_db/patches/custom_save_load.htm) |
| **MenuManipulator** | V[3341] = main-menu command count; V[3342..3349] = command ids | [menu_manipulator.htm](https://dev.makerpendium.de/docs/patch_db/patches/menu_manipulator.htm) |
| **Menu-switch family** (SROA/SRA/…) | Custom-menu commands close the menu and set a switch: status S[1009], row S[1007], order S[1010], ATB S[1008] | `menuswitches_*.htm` |
| **AssignTurnBattle** | Removes ATB; V[3350] picks next battler (1..4 party, −1..−8 troop) | [assign_turn_battle.htm](https://dev.makerpendium.de/docs/patch_db/patches/assign_turn_battle.htm) |
| **BattleMessenger** | Battle message window geometry from V[3322..3325] (w,h,x,y) | [battle_messenger.htm](https://dev.makerpendium.de/docs/patch_db/patches/battle_messenger.htm) |
| **SetBattler** | "Change sprite" command swaps battler animation to V[3331] | [set_battler.htm](https://dev.makerpendium.de/docs/patch_db/patches/set_battler.htm) |
| **SelfVar** | Page conditions read 2 pseudo-self-switches + a self-variable mapped over S/V ≥ 5000 by `S[5001 + (Map−1)*2000 + (Ev−1)]` | [self_var.htm](https://dev.makerpendium.de/docs/patch_db/patches/self_var.htm) |
| **MoveEventPointer** | Move-event target id from V[3330] (10001 player … 10005 this event) | [move_event_pointer.htm](https://dev.makerpendium.de/docs/patch_db/patches/move_event_pointer.htm) |
| **BattleAnimationPointer** | Battle-anim target from V[3328], anim id from V[3329] | [anim_pointer.htm](https://dev.makerpendium.de/docs/patch_db/patches/anim_pointer.htm) |
| **SwitchPointer / BGM&SE Pointer** | Branch on S[1] redirected via V[3397]; BGM/SE params V[3389..3392]/V[3394..3396] ≥1000 act as pointers | [switch_pointer.htm](https://dev.makerpendium.de/docs/patch_db/patches/switch_pointer.htm), [bgm_se_pointer.htm](https://dev.makerpendium.de/docs/patch_db/patches/bgm_se_pointer.htm) |
| **ExtendedKeyInput** | Key Input Proc reads raw VK when V[3340]>0; result −1 when pressed | [ext_key_input.htm](https://dev.makerpendium.de/docs/patch_db/patches/ext_key_input.htm) |
| **Keyboard Observator** | V[4001..4256] = per-key state each frame (init via V[4256]) | [keyboard_observator.htm](https://dev.makerpendium.de/docs/patch_db/patches/keyboard_observator.htm) |
| **ShopEconomy / ShoppingShortcut** | Global buy/sell rates 100+V[3338] / 50+V[3339]; shop command → S[1015]/S[1016], V[3409..3410] | [shop_economy.htm](https://dev.makerpendium.de/docs/patch_db/patches/shop_economy.htm), [shopping_shortcut.htm](https://dev.makerpendium.de/docs/patch_db/patches/shopping_shortcut.htm) |
| **PicPointerPatch 2.5b** | Picture id > 10000 → id from V[id−10000]; > 50000 → also filename digit substitution | [pic_pointer_25b.htm](https://dev.makerpendium.de/docs/patch_db/patches/pic_pointer_25b.htm) |
| **PicsInBattle** | Map pictures drawn in battle; V[5000] = layer priority (0/1/2) | [pics_in_battle.htm](https://dev.makerpendium.de/docs/patch_db/patches/pics_in_battle.htm) |

**ID-clustering note (design guidance).** The German/Patch-DB ecosystem clusters its reserved
variables around **V[3326–3410]** and switches around **S[999–1018]**; battle/scaling protocols use
**V[1001–1010]** and **V[5000+]**. When implementing more of these, keep IDs configurable exactly as
`game_runtime_patches.h`'s `PatchArg` mechanism does (`-var`/`-sw`/`-maxhp` sub-arguments,
[`game_runtime_patches.h:34`](../../src/game_runtime_patches.h)–`42`). The clusters can collide
between patches, which is why per-game declaration (not blanket defaults) is the safe model.

## 7. File-format changes

**None.** This family's defining property is that it touches **only `RPG_RT.exe`** (or, for the
RPG2k3-commands case, ordinary data edited with external tools) — there are no new LDB/LMU/LMT/LSD
chunks, no new standalone files, and no new ini keys *in the game's own files*. The games are plain
RM2k/2k3 projects; that is precisely why detection is hard (§4) and why EasyRPG must be told which
patch is active.

The only "format" surface EasyRPG adds is its own `EasyRPG.ini [Patch]` keys (§4.1) and the
`Game_Constants` overrides that the two cap patches imply:

| Patch | Constant overrides (`game_constants.h:139`) |
|---|---|
| **StatDelimiter** | MaxActorHP 9,999,999; MaxActorSP 9,999,999; MaxStatBaseValue 999,999; MaxStatBattleValue 999,999 |
| **WhiteDragon** (`Rm2k3_Italian_WD_108`) | Var range ±999,999,999; MaxActorHP 99,999; MaxActorSP 9,999; MaxStatBaseValue 9,999; MaxDamageValue 99,999; MaxGoldValue 9,999,999 |

The cap-raising **replacement-RPG_RT** patches (RM2k3++ / RM Limit Changer, 100-Pics) effectively
change the *valid value ranges* in LDB but not the chunk layout — liblcf reads the larger values
fine; EasyRPG just needs matching `Game_Constants`. These two presets are the only ones modelled
today (UNVERIFIED whether RM Limit Changer maps cleanly onto an existing preset).

## 8. Runtime behavior changes

Summarized; per-patch detail is in §4.1/§6.

- **Refresh model:** AntiLagSwitch gates `Game_Map::GetNeedRefresh` so parallel/auto page conditions
  stop re-evaluating while the switch is ON (the engine's per-frame `need_refresh` is forced false)
  ([`game_map.cpp:1781`](../../src/game_map.cpp)).
- **Encounter model:** ERA zeroes the encounter rate and cancels the pending encounter, sets the
  troop var + alert switch, and **always** refreshes the map (the original patch refreshes only in
  the MEPR variant — EasyRPG deliberately always refreshes,
  [`game_runtime_patches.cpp:248`](../../src/game_runtime_patches.cpp)).
- **Battle math:** MonSca scales six base stats + exp/gold/drop-rate (×var/1000), adds to the drop
  *item id*; EXPlus multiplies per-slot EXP; GuardRevamp re-bases defend damage. All are pure read-
  time hooks; no state is persisted.
- **Stat/value caps:** StatDelimiter / WhiteDragon widen `Game_Constants`. These affect clamping of
  HP/SP/stats/damage/gold/variables engine-wide.
- **Menu flow:** DirectMenu changes which scene opens; no new input or rendering.
- **PicUnlock:** removes the "message active" guard on picture commands — pictures animate while text
  is on screen (matches official RM2k ≥1.60 / RM2k3 English behavior, which is why
  `IsPatchUnlockPics()` is OR-ed with `IsEnglish()` at the guard sites,
  [`game_interpreter.cpp:2773`](../../src/game_interpreter.cpp)).
- **No** changes to resolution, audio, text escape codes, save format, or input from this family
  (input/keyboard extensions like ExtendedKeyInput/Keyboard Observator are catalogued but
  unimplemented, §6.6). Resolution/audio/input extensions belong to the major forks
  ([powermode2003.md](powermode2003.md), [ineluki-key-patch.md](ineluki-key-patch.md),
  [maniac-patch.md](maniac-patch.md)).

## 9. EasyRPG support matrix

| Patch | Support | Where | Release vs. master |
|---|---|---|---|
| AntiLagSwitch | **FULL** (Switch variant only; Fast/Slow not modelled) | [`game_map.cpp:1781`](../../src/game_map.cpp) | in 0.8.1 |
| DirectMenu | **FULL** | [`game_interpreter_map.cpp:105`](../../src/game_interpreter_map.cpp), PR [#3192](https://github.com/EasyRPG/Player/pull/3192) | in 0.8.1 |
| EncounterRandomnessAlert (ERA/MEPR) | **FULL** | [`game_runtime_patches.cpp:236`](../../src/game_runtime_patches.cpp), PR [#3378](https://github.com/EasyRPG/Player/pull/3378) | **master-only** (post-0.8.1; `game_runtime_patches.*` absent from the 0.8.1 / 0.8.1.1 tags) |
| MonSca / MonScaPlus | **FULL** | [`game_runtime_patches.cpp:255`](../../src/game_runtime_patches.cpp)–`340`, PR [#3378](https://github.com/EasyRPG/Player/pull/3378) | **master-only** (post-0.8.1; first runtime-patches commit `81bb42e3d`) |
| EXPlus / EXPlus[+] | **FULL** | [`game_runtime_patches.cpp:342`](../../src/game_runtime_patches.cpp)–`353` | **master-only** (post-0.8.1) |
| GuardRevamp | **FULL** | [`game_runtime_patches.cpp:355`](../../src/game_runtime_patches.cpp) | **master-only** (post-0.8.1, commit `d7faf09f3`) |
| CommonThisEvent | **FULL** | [`game_interpreter.cpp:329`](../../src/game_interpreter.cpp) | in 0.8.1 |
| PicUnlock | **FULL** | [`game_interpreter.cpp:2773`](../../src/game_interpreter.cpp) (+ `:2920`, `:3075`) | in 0.8.1 |
| StatDelimiter | **FULL** + autodetect | [`exe_reader.cpp:592`](../../src/exe_reader.cpp)/`603`/`608`, [`game_constants.h:151`](../../src/game_constants.h) | **master-only** (post-0.8.1; `GetOverriddenGameConstants` / `KnownPatchConfigurations` absent from 0.8.1) |
| WhiteDragon caps | **FULL** + autodetect | [`exe_reader.cpp:600`](../../src/exe_reader.cpp), [`game_constants.h:141`](../../src/game_constants.h) | **master-only** (post-0.8.1) |
| RPG2k3Commands (data hack) | **FULL** (dispatch) | [`player.h:492`](../../src/player.h) + ~20 gates (§6.5) | in 0.8.1 |
| BetterAEP / CustomSaveLoad | **MISSING** | — | catalogued §6.6 |
| PicPointer / PicsInBattle | **MISSING** | — | catalogued §6.6 (PicPointer overlaps Maniacs native picture-var support) |
| MenuManipulator / menu-switch family | **MISSING** | — | catalogued §6.6 |
| SelfVar, pointer patches, ExtendedKeyInput, Keyboard Observator, ShopEconomy, … | **MISSING** | — | catalogued §6.6 |
| ~75 further patches (bugfixes, cosmetics, fonts, display) | **MISSING / mostly WONTFIX** | — | many are cosmetic or RPG_RT-bug-specific |

Caveats verified in source:

- **AntiLag**: only the Switch behavior; the original's Fast variant is intentionally not modelled
  (incompatible with most games).
- **ERA**: always refreshes the map, unlike the non-MEPR original (deliberate,
  [`game_runtime_patches.cpp:248`](../../src/game_runtime_patches.cpp)).
- **MonSca item drop**: drop *id* is **added** (`+= V[1009]`), not scaled — match this exactly.
- All `RuntimePatches::*` flags are force-disabled by `--no-patch` via
  `LockPatchesAsDiabled` [sic] ([`game_runtime_patches.cpp:118`](../../src/game_runtime_patches.cpp)).

## 10. Test assets

No single game exercises the whole family. Recommended stress targets (cross-ref
[`docs/games/`](../games/) and the corpus at `c:\rg\easyrpg_library`):

- Any RM2k3 game shipping a **MonSca/EXPlus/GuardRevamp** battle protocol (search the corpus for
  V[1001..1010] presets and S[1001] toggles).
- **Beloved Rapture** is *not* a primary asset for this family (it uses Maniac Patch), but Maniacs
  games sometimes also rely on `RPG2k3Commands`/`PicUnlock` semantics; see
  [../games/beloved-rapture.md](../games/beloved-rapture.md).
- **StatDelimiter / WhiteDragon** autodetect can be regression-tested with any RM2k 1.62 / RM2k3
  1.0.8.0 / 1.0.9.1 exe carrying the marker (do not redistribute the exe — only assert the constant
  overrides fire).
- A minimal repro for **RPG2k3Commands** is a 2k project whose LDB was edited to place a Call-Event
  command inside a battle event (the YADOT-style hack, §6.5).

(UNVERIFIED which specific corpus games are tagged for each patch — pending the corpus-analysis task.)

## 11. Open questions

- **Autodetect for the variable/switch-protocol patches.** Can the Patch-DB offset tables be turned
  into an `RPG_RT.exe` signature database (issue [#1182](https://github.com/EasyRPG/Player/issues/1182))
  so AntiLag/MonSca/DirectMenu/etc. don't require manual flags? (RE-pending.)
- **Default-ID collisions.** Multiple bugmenot patches reuse V[3350]/S[1000]; a game stacking patches
  could clash. Does any real corpus game stack two of these, and does EasyRPG's per-patch
  configurability cover it? (RE-pending.)
- **RM Limit Changer / RM2k3++** cap mapping — does it match an existing `Game_Constants` preset or
  need its own? (UNVERIFIED.)
- **MEPR vs. ERA** — EasyRPG collapses both into one always-refresh behavior; confirm no corpus game
  depends on the non-refreshing ERA variant. (RE-pending.)
- **PicPointer overlap with Maniacs.** Maniacs has native variable-picture-id support; is a separate
  PicPointer reimplementation still needed for non-Maniacs games? (hypothesis: yes, for 2k3 1.08
  games.)

## 12. References

Primary:

- Makerpendium RPG_RT.exe Patch Database (117 patches, per-build offsets & raw bytes) —
  <https://dev.makerpendium.de/docs/patch_db/main-en.htm>. Per-patch pages cited inline in §4.1/§6.6.
- EasyRPG Player source (branch `RISKY`, this repo): `src/game_runtime_patches.{h,cpp}`,
  `src/game_config_game.{h,cpp}`, `src/game_constants.h`, `src/exe_reader.cpp`, `src/game_map.cpp`,
  `src/game_enemy.h`, `src/game_enemyparty.cpp`, `src/game_interpreter.cpp`,
  `src/game_interpreter_map.cpp`, `src/game_interpreter_battle.cpp`, `src/player.{h,cpp}`.
- EasyRPG Player manual (`--patch-*` / `[Patch]` ini) — <https://easyrpg.org/player/manual/>.
- EasyRPG Wiki "Known patches" —
  <https://wiki.easyrpg.org/development/technical-details/known-patches>.

Research provenance (outside repo):

- [`/home/john/research/other_patches_web.md`](../../../research/other_patches_web.md) — web catalog
  of RM2k/2k3 forks & patches.
- [`/home/john/research/easyrpg_patch_inventory.md`](../../../research/easyrpg_patch_inventory.md) —
  EasyRPG/liblcf implemented-flag inventory (§1, §7, §9).

Secondary / JP data-hack culture:

- YADOT 「改造した作品データで公開している一例」 — <http://yado.tk/2k/01_shoshin/022_kaizou/> (HTTP-only
  Shift-JIS; relevant to §6.5).
- EasyRPG PRs/issues: [#3192](https://github.com/EasyRPG/Player/pull/3192) (DirectMenu),
  [#3378](https://github.com/EasyRPG/Player/pull/3378) (ERA/MonSca),
  [#3123](https://github.com/EasyRPG/Player/pull/3123) (SetInterpreterFlag),
  [#588](https://github.com/EasyRPG/Player/issues/588) (PicUnlock / version behavior),
  [#1182](https://github.com/EasyRPG/Player/issues/1182) (exe signature autodetect).

Cross-links: [README.md](README.md) · [easyrpg-extensions.md](easyrpg-extensions.md) ·
[official-versions.md](official-versions.md) · [bootlegs-translations.md](bootlegs-translations.md) ·
[powermode2003.md](powermode2003.md) · [maniac-patch.md](maniac-patch.md) ·
[../games/beloved-rapture.md](../games/beloved-rapture.md).
