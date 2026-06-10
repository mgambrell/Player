<!--
Spec type: engine IDENTIFICATION (not a feature patch). TEMPLATE sections 1, 3, 4, 8, 9, 12
carry the weight; "event commands" / "file formats" become "what changed at runtime between
versions". Sources: /home/john/research/other_patches_web.md §9; verified against src/player.{h,cpp},
src/exe_reader.{h,cpp}, src/game_config_game.cpp, src/filefinder.{h,cpp}, src/feature.cpp,
src/game_pictures.cpp, src/sprite_picture.cpp, src/game_battle.cpp, src/window_message.cpp,
src/game_interpreter.cpp, src/game_message.cpp, src/algo.cpp and lib/liblcf chunk tables
(reader_struct.h, ldb/chunks.h). All file:line anchors re-checked 2026-06-09.
Do not edit source; this is the only file written.
-->

# Official RPG Maker 2000 & 2003 version matrix (RPGツクール2000 / RPGツクール2003)

> This is not a fork — it is the **baseline** every fork derives from. `RPG_RT.exe` is the
> runtime interpreter shipped with RPG Maker 2000 (RPGツクール2000, *Tsukūru 2000*) and
> RPG Maker 2003 (RPGツクール2003). Across two decades it was revised many times, and a
> handful of those revisions changed *runtime behavior* in ways a game can depend on:
> picture limits, message placeholders, picture-during-message blocking, "This Event" in
> common events, 0%-effect damage, MP3 support, and more. EasyRPG Player reimplements the
> runtime, so it must pick the right behavior set per game. It does this by classifying the
> engine into four orthogonal bits — **2k vs 2k3**, **Major-Updated**, **English/Steam** —
> and gates behavior off those bits. This document is the authoritative map from marketing
> version → build date → runtime behavior → the EasyRPG bit that selects it.

**Status at a glance:** EasyRPG support = FULL (engine classification is a core, mature
subsystem; a few fine-grained sub-versions are deliberately collapsed — see §8/§9).
Detection = VERSIONINFO product-version + startup-logo count + PE `CODE` section size from
`RPG_RT.exe`, with `RPG_RT.ldb` `ldb_id`/version-chunk and `RPG_RT.exe` file-size fallbacks.
Spec confidence = HIGH for the classification logic (verified in-tree); MEDIUM for some exact
build dates (community-documented, see §3).

---

## 1. Identity

| | |
|---|---|
| Names | RPG Maker 2000 (EN) / RPGツクール2000 (JP, *RPG Tsukūru 2000*); RPG Maker 2003 (EN) / RPGツクール2003 (JP, *RPG Tsukūru 2003*) |
| Runtime file | `RPG_RT.exe` (the interpreter); editor is `RPG2000.exe` / `RPG2003.exe` (not run by players) |
| Author / vendor | ASCII / Enterbrain (later Kadokawa). Official English & Steam line developed by **David "Cherry" Trapp**, published by **Degica** ([RPG Advocate context](https://rpgmaker.fandom.com/wiki/RPG_Advocate)) |
| License | Proprietary, commercial. The classic JP EULA (ツクールの規約) forbade modifying `RPG_RT.exe`, which is why the early JP scene hacked *data* not the binary ([YADOT](http://yado.tk/2k/01_shoshin/022_kaizou/)). Steam releases are sold on Steam. |
| Distribution | JP updates: <https://rpgmakerofficial.com/product/support/update_rpg2000.html> (alive, `RPG2000UP52.exe`). Steam: RM2k [app 383730](https://store.steampowered.com/news/app/383730/), RM2k3 [app 362870](https://store.steampowered.com/news/app/362870/). The phylomortis.com RM2k3 version history is dead; mirror at [EasyRPG wiki](https://easyrpg.github.io/wiki/development/technical-details/rpg-maker-2003-version-history/). |
| Liveness | Steam builds still receive occasional updates. JP RM2k update page last revised 2012-09-27 (v1.52). The runtime spec below is effectively frozen. |

Why this matters for EasyRPG: a game records which engine made it only indirectly (in the
LDB and in the bundled `RPG_RT.exe`); the same `.lmu`/`.ldb` event commands behave differently
across versions, so getting the engine bits right is a prerequisite to running anything.

---

## 2. Target engine builds

This document *covers all official builds* rather than targeting one. EasyRPG collapses the
build space into four boolean facts (see §4). The relevant marketing versions are:

- **RPG Maker 2000:** 1.00 → 1.07 → 1.10 (classic JP line); 1.50/1.51/1.52 (**VALUE!** major
  update); 1.60/1.61/1.62 (official **English / Steam** line).
- **RPG Maker 2003:** 1.00 → 1.04 (early JP); 1.05 → 1.08/1.09/1.09a (JP, where 1.08 is the
  Western modding baseline); 1.10 → 1.12/1.12a (official **English / Steam** line).

EasyRPG never patches an `RPG_RT.exe`; it reads it (and the LDB) only to classify, then runs
its own interpreter with the matching behavior set.

---

## 3. Version lineage

Build dates below are from the community-maintained version histories: the official JP RM2k
update page (<https://rpgmakerofficial.com/product/support/update_rpg2000.html>), the
phylomortis RM2k3 history (mirror: [EasyRPG wiki](https://easyrpg.github.io/wiki/development/technical-details/rpg-maker-2003-version-history/)),
the [Makerpendium Patch-DB](https://dev.makerpendium.de/docs/patch_db/main-en.htm) per-build
compatibility matrices, and the Steam patch notes linked per row. Dates marked (community) are
not from a vendor changelog. Internal `a.b.c.d` strings are the PE VERSIONINFO product version.

### 3.1 RPG Maker 2000

| Marketing ver | Build date | Internal / notes | Headline runtime changes | Published |
|---|---|---|---|---|
| 1.00 + ~6 interim builds | 2000-05-07 … 2000-11-15 (community) | no VERSIONINFO; **3 startup logos** | original release line | Yes (JP) |
| **1.07** | 2000-12-27 | no VERSIONINFO; 3 logos | Don Miguel's EN translation is built on the 1.05/1.07 era ([Makerpendium](https://www.makerpendium.de/index.php/Don_Miguel)) | Yes (JP) |
| **1.10** | 2001-05-05 | no VERSIONINFO; 3 logos | last of the "legacy" 2k line; pictures still 20 | Yes (JP) |
| **1.50 (VALUE!)** | 2003-03-27 | no VERSIONINFO; **1 logo**; `CODE` ≤ 0xB0000 | major update — see §8.1 | Yes (JP) |
| **1.51 / 1.52** | 2003-06-25 | one shared `RPG_RT`; update file `RPG2000UP52.exe` | 1.51 = 30+ bugfixes; 1.52 minor | Yes (JP) |
| **1.60** | 2015-07-05 (community) | first official English; has VERSIONINFO `1.6.x` | pictures-during-message unblocked (§8.1) | Yes (Steam/EN) |
| **1.61** | 2015-09-15 (community) | VERSIONINFO `1.6.x`; **1 logo** | battle-message placeholders `%S %O %V %U`, battle word-wrap ([#588](https://github.com/EasyRPG/Player/issues/588)) | Yes (Steam/EN) |
| **1.62** | 2017-09-14 (community) | VERSIONINFO `1.6.x` | minor (editor import dialog) ([Steam news](https://store.steampowered.com/news/app/383730/view/5260723642047801731)) | Yes (Steam/EN) |

Two famous *fakes* in this line are pure logo re-skins of 1.50/1.51 and are detected as such
(see [bootlegs-translations.md](bootlegs-translations.md)): **Gnaf's Picture Patch** (rebranded VALUE! 1.50, logo CRC32
`0xC5E846A7`) and **Spezial-Patch** by Rikku2000 (1.51 rebrand, logo CRC32 `0x806B6877`)
([Gnaf](https://www.makerpendium.de/index.php/Gnaf), [Spezial-Patch](https://www.makerpendium.de/index.php/Spezial-Patch_(RPG_Maker_2000))).

### 3.2 RPG Maker 2003

| Marketing ver | Build date | Internal / notes | Headline runtime changes | Published |
|---|---|---|---|---|
| **1.00** | 2002-12-18 | no VERSIONINFO; 1 logo | original release | Yes (JP) |
| 1.01 / 1.01a | — | Patch-DB labels `UNKNOWN1/2` | — | Yes (JP) |
| 1.02 | 2003-01-15 | `1.0.2.1` (first with VERSIONINFO) | — | Yes (JP) |
| 1.03 | 2003-02-05 | `1.0.3.0` | — | Yes (JP) |
| **1.04** | 2003-03-01 | `1.0.4.0` | last "legacy 2k3" (≤1.04) | Yes (JP) |
| **1.05** | 2003-05-13 | `1.0.5.0`; **major update**; `CODE` ≥ 0xC7400 | pictures 40→50, MP3, nested `\n[\v[..]]`, Key-Input direction restrictions + Shift, decision-key message speed-up, battle Input Number, hero-targetable battle anims, MP shown in "traditional" battle | Yes (JP) |
| 1.06 | 2003-06-24 | `1.0.6.0` | — | Yes (JP) |
| 1.07 | 2003-07-15 | `1.0.7.0` | — | Yes (JP) |
| **1.08** | 2003-09-22 | `1.0.8.0`; crash fixes | **the Western modding baseline** (DynRPG, PicPointer, etc. target this) | Yes (JP) |
| **1.09 / 1.09a** | 2004-01-14 | `1.0.9.1`; "Ver1.09a" per [VIPRPG](https://wikiwiki.jp/viprpg-dev/2003) | final JP build | Yes (JP) |
| **1.10** | 2015-04-24 (community; first official EN) | `1.1.0.x`; ships `ultimate_rt_eb.dll` | DirectDraw removed from fullscreen; caps 5000→9999; pictures 50→1000; labels 100→1000; 12-char hero names ([changelog](https://steamcommunity.com/app/362870/discussions/0/618460171314070796/)) | Yes (Steam/EN) |
| 1.10a / 1.11 | — | minor editor fixes ([news](https://steamcommunity.com/games/362870/announcements/detail/144469005452358612)) | — | Yes (Steam/EN) |
| **1.12** | — | "This Event" usable in common events; restart F12→Alt+F12; resizable dialogs ([news](https://store.steampowered.com/news/app/362870/view/3928784047051996238)) | Yes (Steam/EN) |
| **1.12a** | — | fixes a 1.12 regression: Show-Picture from ≤1.11 editors was treated as if "Erase on Map Change"/"Affected by Tint"/"Affected by Shake" were off ([SteamDB](https://steamdb.info/patchnotes/2173406/), [news](https://store.steampowered.com/news/app/362870/view/3928784047051996141)) | Yes (Steam/EN) |

**1.12a is the base of Maniac Patch, and therefore of Beloved Rapture** — see
[maniac-patch.md](maniac-patch.md) and [../games/beloved-rapture.md](../games/beloved-rapture.md).
A separate Maniac build line is a *rewrite* reporting VERSIONINFO `1.1.2.1` with no logos and no
`CODE`/`CHERRY` section — EasyRPG classifies that as 2k3-English-MajorUpdated too (see §4.2).

Unofficial English builds in this lineage (Don Miguel for 2k, RPG Advocate for 2k3, Hellsoft
Spanish, etc.) report 1.08/1.09 (`1.0.9.1`) VERSIONINFO but are *not* the official English bit
holders; they are covered in [bootlegs-translations.md](bootlegs-translations.md).

---

## 4. Detection — how EasyRPG decides the engine + version

EasyRPG resolves the engine into a bitmask, `Player::EngineType`, defined in
`src/player.h:38`:

| Bit | Value | Meaning | `src/player.h` |
|---|---|---|---|
| `EngineNone` | 0 | undecided | `:39` |
| `EngineRpg2k` | 1 | any RPG Maker 2000 | `:41` |
| `EngineRpg2k3` | 2 | any RPG Maker 2003 | `:43` |
| `EngineMajorUpdated` | 4 | 2k ≥ 1.50 **or** 2k3 ≥ 1.05 | `:45` |
| `EngineEnglish` | 8 | official English: 2k ≥ 1.61 **or** 2k3 ≥ 1.10 | `:47` |

These are *orthogonal flags*, not a linear version number. The query helpers in `src/player.h`
(declared `:201`–`:237`, defined inline `:448`–`:508`) combine them:

| Helper | Definition | `player.h` |
|---|---|---|
| `IsRPG2k()` / `IsRPG2k3()` | `engine & EngineRpg2k` / `& EngineRpg2k3` | `:448`, `:452` |
| `IsRPG2kLegacy()` | `engine == EngineRpg2k` (no other bits) → 2k ≤ 1.10 | `:456` |
| `IsRPG2k3Legacy()` | `engine == EngineRpg2k3` → 2k3 ≤ 1.04 | `:460` |
| `IsMajorUpdatedVersion()` | `engine & EngineMajorUpdated` | `:468` |
| `IsRPG2kUpdated()` | `IsRPG2k() && IsMajorUpdatedVersion()` → 2k ≥ 1.50 | `:476` |
| `IsRPG2k3Updated()` | `IsRPG2k3() && IsMajorUpdatedVersion()` → 2k3 ≥ 1.05 | `:480` |
| `IsEnglish()` | `engine & EngineEnglish` | `:472` |
| `IsRPG2kE()` | `IsRPG2k() && IsEnglish()` → 2k ≥ 1.61 | `:484` |
| `IsRPG2k3E()` | `IsRPG2k3() && IsEnglish()` → 2k3 ≥ 1.10 | `:488` |

Note the deliberate gaps: there is **no separate bit for 1.60 vs 1.61** (1.60 is hard to detect
and treated as legacy-ish; only ≥1.61 gets `EngineEnglish` — see the doc comment at
`src/player.h:218`), and **no bit distinguishing 1.10/1.11/1.12/1.12a** within 2k3-English.
EasyRPG decided this isn't worth differentiating ([#588](https://github.com/EasyRPG/Player/issues/588)).

### 4.1 Decision order (`Player::CreateGameObjects`, `src/player.cpp`)

1. **Manual override.** If `--engine`/`[Engine]` set a value, it wins and autodetect is skipped.
   `src/game_config_game.cpp:38`–`:59` (in `Game_ConfigGame::Initialize`; the `engine_str` value
   itself is populated from `--engine` in `LoadFromArgs` and from the `.ini` `[Engine]` block via
   `engine_str.FromIni` in `LoadFromStream`, `src/game_config_game.cpp:215`) sets
   `cfg.engine = EngineNone` then maps the string token to a bitmask. Autodetect (steps 2–3) is
   gated entirely on `engine == EngineNone` (`src/player.cpp:795`, `:820`), so any non-`None`
   manual value disables it. (This is the *engine* override, distinct from the `--patch-*`
   `patch_override` flag described in [easyrpg-extensions.md](easyrpg-extensions.md).) Token map:

   | `--engine` token (aliases) | Resulting bits |
   |---|---|
   | `rpg2k` / `2000` | `EngineRpg2k` |
   | `rpg2kv150` / `2000v150` | `EngineRpg2k \| EngineMajorUpdated` |
   | `rpg2ke` / `2000e` | `EngineRpg2k \| EngineMajorUpdated \| EngineEnglish` |
   | `rpg2k3` / `2003` | `EngineRpg2k3` |
   | `rpg2k3v105` / `2003v105` | `EngineRpg2k3 \| EngineMajorUpdated` |
   | `rpg2k3e` / `2003e` | `EngineRpg2k3 \| EngineMajorUpdated \| EngineEnglish` |

   The same tokens appear in the CLI help at `src/player.cpp:1465`–`:1470`.

2. **`RPG_RT.exe` analysis** (`src/player.cpp:784`–`:809`; skipped on Emscripten). An
   `EXEReader` parses the PE; `FileInfo::GetEngineType()` (`src/exe_reader.cpp:483`) returns the
   bitmask. See §4.2.

3. **LDB / tree fallback** when the exe yields `EngineNone` (`src/player.cpp:820`–`:836`):
   - `lcf::Data::system.ldb_id == 2003` → `EngineRpg2k3`, else `EngineRpg2k`
     (`src/player.cpp:821`). `ldb_id` is the LDB "system" id field that distinguishes the two
     editors.
   - For 2k3: if `ultimate_rt_eb.dll` is present → add `EngineEnglish | EngineMajorUpdated`
     (`src/player.cpp:823`). That DLL ships only with official English 2k3 ≥ 1.10.
   - For 2k: if `lcf::Data::data.version >= 1` → add `EngineEnglish | EngineMajorUpdated`
     (`src/player.cpp:828`). This is the **Database version chunk** — see §4.3.
   - If still not Major-Updated, call `FileFinder::IsMajorUpdatedTree()`
     (`src/player.cpp:832`).

The final classification is logged: `"Engine configured as: 2k={} 2k3={} MajorUpdated={} Eng={}"`
(`src/player.cpp:839`).

### 4.2 `RPG_RT.exe` classification — `FileInfo::GetEngineType` (`src/exe_reader.cpp:483`)

The reader extracts a `FileInfo` (`src/exe_reader.h:58`) with: `version` (64-bit packed
VERSIONINFO product version, 16 bits per field), `version_str`, `logos` (count of startup logos
embedded in the exe), `code_size` (PE `CODE` section size), `cherry_size`/`geep_size` (sizes of
the special `CHERRY`/`GEEP` PE sections), `machine_type`, `is_easyrpg_player`, and
`maniac_patch_version`. The classifier:

| Condition (in order) | Verdict | `exe_reader.cpp` |
|---|---|---|
| `is_easyrpg_player` or unknown machine | `EngineNone` (we don't classify our own exe) | `:486` |
| **No VERSIONINFO** and `logos == 3` | `EngineRpg2k` (old 2k only ever has 3 logos) | `:492` |
| No VERSIONINFO, `logos == 1`, `CODE > 0xB0000` and `≥ 0xC7400` | `EngineRpg2k3 \| MajorUpdated` (≥ 1.0.5.0 by code size; covers Ahriman's Prophecy 1.0.8.0 *without* VERSIONINFO) | `:499` |
| No VERSIONINFO, `logos == 1`, `0xB0000 < CODE < 0xC7400` | `EngineRpg2k3` (legacy 2k3 < 1.0.5.0) | `:505` |
| No VERSIONINFO, `logos == 1`, `CODE ≤ 0xB0000` | `EngineRpg2k \| MajorUpdated` (VALUE!-era 2k) | `:509` |
| VERSIONINFO, `logos == 0`, ver `1.1.2.1`, no `CODE`/`CHERRY` | `EngineRpg2k3 \| MajorUpdated \| English` (Maniac Patch *rewrite* line) | `:524`–`:529` |
| VERSIONINFO, `logos == 1`, ver `1.6.x` | `EngineRpg2k \| MajorUpdated \| English` (English RM2k ≥ 1.61) | `:541` |
| VERSIONINFO, `logos == 1`, ver `1.0.x`, minor `< 5` | `EngineRpg2k3` (2k3 < 1.0.5.0) | `:546` |
| VERSIONINFO, `logos == 1`, ver `1.0.x`, minor `≥ 5` | `EngineRpg2k3 \| MajorUpdated` (2k3 ≥ 1.0.5.0, JP) | `:549` |
| VERSIONINFO, `logos == 1`, ver `1.1.x` (incl. `1.1.2.1` hacked Maniac) | `EngineRpg2k3 \| MajorUpdated \| English` (English/Steam 2k3 ≥ 1.10) | `:551`–`:562` |
| anything else | `EngineNone` | `:566` |

Notes on the Maniac-Patch detection embedded here (sets `mp_version`, not just the engine):
older Maniac builds are a hacked `1.1.2.1` — early ones carry a `GEEP` section, later ones drop
`GEEP` for an enlarged `CHERRY` (`src/exe_reader.cpp:556`–`:558`); the newest Maniac line is the
logo-less `1.1.2.1` rewrite (`:524`). The VERSIONINFO is read from the `VS_FIXEDFILEINFO`
structure (`src/exe_reader.cpp:343`–`:397`); only RM2k ≥ 1.60, RM2k3 ≥ 1.0.2.1, EasyRPG, and
Maniacs have one — and Maniacs additionally embeds a UTF-16 `"Maniacs, vXXXXXX"` string
(`:392`) parsed into `maniac_patch_version`. See [maniac-patch.md](maniac-patch.md) for that.

`logos` is the count of startup-logo images packed into the exe's resource/XYZ table; it is
computed by `FileInfo::GetLogoCount` (`src/exe_reader.cpp:451`, which reads the XYZ resource
directory entry count) and the images themselves are extracted by `GetStartupLogos`
(`src/exe_reader.cpp:270`–`:330`). Known/bootleg logo CRC32s are listed at
`src/exe_reader.cpp:32` and used to skip known logos (detail in [bootlegs-translations.md](bootlegs-translations.md)). The
"3 logos = old RM2k, 1 logo = VALUE!-era or 2k3" heuristic is the backbone of the no-VERSIONINFO
path above.

### 4.3 LDB signals: `ldb_id` and the 0x1A version chunk

Two LDB facts feed the fallback:

- **`ldb_id == 2003`** distinguishes the 2k3 editor's database from the 2k one
  (`src/player.cpp:821`).
- **Database version chunk `0x1A`** (`lcf::rpg::Database::version`,
  `lib/liblcf/.../generated/lcf/ldb/chunks.h:1538`). The liblcf comment states: *"Indicates
  version of database. When 1 the database was converted to RPG Maker 2000 v1.61."* In 2k
  databases the chunk is omitted when its value is 0 (treated as absent —
  `lib/liblcf/src/reader_struct.h:440`–`:454`, `DatabaseVersionField`), and in 2k3 databases it
  is always present (`:448`). EasyRPG reads it as `lcf::Data::data.version` and uses
  `>= 1` → English+MajorUpdated for 2k (`src/player.cpp:828`). So a 2k game whose LDB was
  re-saved by English RM2k ≥ 1.61 self-identifies even without the exe.

### 4.4 `IsMajorUpdatedTree` — file-size / MP3 fallback (`src/filefinder.cpp:522`)

When neither exe nor LDB settled the Major-Updated bit, EasyRPG sniffs the tree:

1. If a `.mp3` exists in `Music/` → Major-Updated (MP3 BGM only exists from 2k 1.50 / 2k3 1.05).
   This test is suppressed when a *non-official* `Harmony.dll` is present (size ≠ 473600,
   `KnownFileSize::OFFICIAL_HARMONY_DLL`, `src/filefinder.h:368`), because the
   Disharmony/Ineluki MP3 patches let older engines play MP3 too (`src/filefinder.cpp:531`).
2. Else compare `RPG_RT.exe` size against a threshold (`src/filefinder.h:391`): 2k > 735000
   bytes, 2k3 > 927000 bytes → Major-Updated. Reference sizes are tabulated at
   `src/filefinder.h:378`–`:393` (e.g. 2k 1.51 JP = 746496; 2k3 1.09a JP = 950784).
3. Else assume newer for Japanese or 2k3 games, older for non-JP 2k (`src/filefinder.cpp:565`).

### 4.5 Cross-references

This spec only covers *official* identification. Logo-CRC identification of bootleg/translated
runtimes (Don Miguel, RPG Advocate, Hellsoft, Gnaf, Spezial-Patch, the Ahriman's-Prophecy
no-VERSIONINFO exe) lives in [bootlegs-translations.md](bootlegs-translations.md). The `--patch-*` / `Game_ConfigGame`
mechanism is in [easyrpg-extensions.md](easyrpg-extensions.md). Several version-keyed behaviors
are also reachable as opt-in patches for older engines — `PicUnlock` (§8.1) and
`CommonThisEvent` (§8.3) — see [runtime-micro-patches.md](runtime-micro-patches.md).

---

## 5. Event commands

Not applicable as a per-code table — the official versions do not *add* event command codes
(that is what forks like Maniac do). What changes across versions is the *behavior* of existing
commands. Those behavioral deltas are documented in §8 (e.g. Key Input Processing gaining
direction keys + Shift in 2k 1.50 / 2k3 1.05; Show/Input Number becoming valid in battle events).
One subtlety EasyRPG keys off engine version inside a command handler: a game can carry 2k3-only
event commands inside a 2k project (data-level hacks); EasyRPG exposes the `RPG2k3Commands` flag
(`Player::IsRPG2k3Commands()`, declared `src/player.h:276`, inline body
`src/player.h:492`–`:499`: `IsRPG2k3() || patch_rpg2k3_commands`) so those run on a 2k engine —
see [easyrpg-extensions.md](easyrpg-extensions.md) and
[runtime-micro-patches.md](runtime-micro-patches.md).

## 6. Modified baseline commands

Not applicable in the fork sense — there is no fork here. The closest analogue is "the same
command does X on the legacy engine and Y on the major-updated/English engine"; those are in §8.

## 7. File-format changes

Across official versions the LDB/LMU/LMT/LSD chunk *layout* is stable; what changes is which
fields are meaningful and the database **version chunk `0x1A`** (covered in §4.3). The Steam 2k3
1.10 cap raises (items/skills/switches/variables 5000→9999, pictures 50→1000, labels 100→1000)
widen *value ranges* inside existing chunks rather than adding chunks; liblcf models the fields
losslessly regardless. New standalone files are a fork concern (e.g. Maniac's global save), not
an official-version one.

---

## 8. Runtime behavior changes (the version-keyed deltas EasyRPG replicates)

This is the operational core of the spec: each official behavior change, which engine bit
selects it in EasyRPG, and the anchor.

### 8.1 RPG Maker 2000: 1.10 → 1.50 (VALUE!) → 1.60/1.61

From the official JP update page
(<https://rpgmakerofficial.com/product/support/update_rpg2000.html>) and EasyRPG issue
[#588](https://github.com/EasyRPG/Player/issues/588):

| Behavior | Legacy (≤1.10) | Major-Updated (≥1.50) | English (≥1.61) | EasyRPG gate |
|---|---|---|---|---|
| **Picture limit** | 20 | 50 | 1000 | `Game_Pictures::GetDefaultNumberOfPictures` — `IsRPG2kLegacy()`→20, `IsMajorUpdatedVersion()`→50, `IsEnglish()`→1000 (`src/game_pictures.cpp:129`–`:146`) |
| Battle animations drawn above pictures | no | yes | yes | priority-layer feature `feature_priority_layers = IsMajorUpdatedVersion()` (`src/sprite_picture.cpp:33`) |
| `\n[…]` actor-name substitution in messages | no | yes | yes | EasyRPG does **not** version-gate this — `Game_Message::ParseActor` (`src/game_message.cpp:381`) always parses `\n[]`. (UNVERIFIED whether any legacy-2k game breaks on the always-on behavior) |
| Key Input Processing: separate direction keys + Shift | no | yes | yes | `CommandKeyInputProc` (code 10101): 2k legacy uses one combined direction flag (`com.parameters[2]`), ≥1.50 reads Shift `parameters[5]` + individual Down/Left/Right/Up `parameters[6..9]` — gated `Player::IsRPG2kLegacy()` (`src/game_interpreter.cpp:3310`–`:3324`) |
| Show Choices & Input Number in **battle events** | no | yes | yes | an *editor*-era restriction (pre-1.50 editors disallowed placing these in battle events); EasyRPG runs whatever the data contains and does **not** version-gate `CommandShowChoices`/`CommandInputNumber` (codes 10140/10150, `src/game_interpreter.cpp:1010`/`:1043`) (UNVERIFIED that no legacy game ships such data) |
| MP3 BGM, MS-ADPCM WAV | no | yes | yes | audio decoder selection; also feeds `IsMajorUpdatedTree` MP3 test (§4.4) |
| **0%-effect damage = exactly 0** (was randomly 1) | random ±1 | strict 0 | strict 0 | `Algo::VarianceAdjustEffect` (`src/algo.cpp:164`–`:171`): `var > 0 && (base > 0 \|\| Player::IsLegacy())` — legacy applies variance even at `base==0` (→ can produce ±1); Major-Updated only when `base>0` (0 stays 0) |
| Default message speed | slower | slightly faster | — | **not replicated per-version** — no `IsLegacy()`/`IsMajorUpdated` gate for message timing exists in the tree (the only Legacy/MajorUpdated gates are picture caps, picture move, 0%-damage, priority layers, Key-Input) (UNVERIFIED whether this is observable) |
| Random-encounter step variance | wider | reduced | — | **not replicated per-version** (no encounter-RNG version gate in tree) (UNVERIFIED impact) |
| Enemy HP to 5 digits | 4 | 5 | 5 | a data-range fact carried losslessly by liblcf field widths, not a runtime gate |
| **Pictures during message** not blocked (Show/Move/Erase) | blocked | blocked | unblocked (≥1.60) | the *not-English* path blocks: `!IsEnglish() && !IsPatchUnlockPics() && IsMessageActive()` (`src/game_interpreter.cpp:2773`, `:2920`, `:3075`). `PicUnlock` patch emulates 1.60 unblocking on older engines. |
| **Battle-message placeholders** `%S %O %V %U` (subject/object/value/unit), battle word-wrap | no | no | yes (≥1.61) | `Feature::HasPlaceholders()` → true for `IsRPG2kE()` (`src/feature.cpp:49`); consumed throughout `src/game_message_terms.cpp` |

1.51/1.52 are bugfix-only; 1.62 is editor-only — neither has a distinct EasyRPG bit.

### 8.2 RPG Maker 2003: 1.04 → 1.05 → 1.10 (Steam)

From the phylomortis/EasyRPG-wiki 2k3 version history and the Steam changelogs (§3.2 links):

| Behavior | Legacy (≤1.04) | Major-Updated (≥1.05) | English (≥1.10) | EasyRPG gate |
|---|---|---|---|---|
| **Picture limit** | 40 | 50 (2000 under DynRPG) | 1000 | `src/game_pictures.cpp:129`–`:146` (`IsRPG2k3Legacy()`→40). The DynRPG→2000 path is in the **Major-Updated** branch (`IsMajorUpdatedVersion() && IsPatchDynRpg() && IsRPG2k3()`, `:134`) — DynRPG targets 2k3 1.08, which is Major-Updated, *not* English; so 2000 applies to the 50-column, not the 1000-column |
| MP3 BGM | no | yes | yes | audio / `IsMajorUpdatedTree` |
| Nested `\n[\v[..]]` name substitution | no | yes | yes | EasyRPG does not version-gate this; `ParseParam` recurses (`max_recursion` arg, `src/game_message.cpp:381`) for all engines |
| Key Input direction restrictions + Shift | no | yes | yes | `CommandKeyInputProc` (code 10101): 2k3 legacy combined direction flag, ≥1.05 individual keys at `parameters[5..9]` — gated `Player::IsRPG2k3Legacy()`; the `param_size > 10 && IsMajorUpdatedVersion()` branch is Maniac middle/wheel only (`src/game_interpreter.cpp:3327`–`:3356`) |
| Decision key speeds up messages | no | yes | yes | message timing (RPG_RT delta; EasyRPG per-version gate not individually pinned — RE-pending) |
| Input Number in battle; hero-targetable battle anims; MP shown in "traditional" battle | no | yes | yes | battle/interpreter (RPG_RT deltas; partly editor-era restrictions — EasyRPG runs the data as authored; per-item gates not individually pinned — RE-pending) |
| Caps 5000→9999, labels 100→1000, 12-char names | — | — | yes | data ranges (English) |
| **Battle damage / message algorithm = 2k3E** | classic | classic | 2k3E | `use_2k3e_algo = IsRPG2k3E()` (`src/game_battle.cpp:197`); battle message window placement also forks on `IsRPG2k3E()` (`src/window_message.cpp:107`, `:303`) |
| Picture **spritesheet** feature (frames) | no | no | yes | `feature_spritesheet = IsRPG2k3ECommands()` (`src/sprite_picture.cpp:32`); savegame stores `frames` only for `IsRPG2k3E()` (`src/game_pictures.cpp:115`) |
| Picture **bottom-transparency** quirk | yes | yes | no | `feature_bottom_trans = IsRPG2k3() && !IsRPG2k3E() && !powermode` (`src/sprite_picture.cpp:34`) — i.e. **all** non-English 2k3 (legacy and Major-Updated), suppressed under PowerMode |

### 8.3 RPG Maker 2003 Steam line: 1.10 → 1.12 → 1.12a

These are *not* separated by an EasyRPG bit (all are `IsRPG2k3E()`), but the behaviors matter:

| Behavior | Introduced | EasyRPG handling |
|---|---|---|
| **"This Event" usable in common events** (refers to the last map event in the call stack) | 1.12 | `event_id == 0 && (IsRPG2k3E() \|\| IsPatchCommonThisEvent())` (`src/game_interpreter.cpp:329`). The `CommonThisEvent` patch backports it to older engines. |
| Show-Picture flag defaults for ≤1.11-authored pictures ("Erase on Map Change" / "Affected by Tint" / "Affected by Shake") | 1.12a fix | (RE-pending whether EasyRPG special-cases this; it currently treats all `IsRPG2k3E()` uniformly) |
| Restart hotkey F12 → Alt+F12; resizable dialogs | 1.12 | editor/host concern, not interpreter |

Because EasyRPG cannot reliably tell 1.10/1.11/1.12/1.12a apart from the exe (all VERSIONINFO
`1.1.x`, one logo), it assumes the newest 2k3-English behavior. For Beloved Rapture this is fine
(it is 1.12a + Maniac), but a game shipping a 1.10 exe that relies on pre-1.12 "This Event"
errors would not be reproduced. (hypothesis — no known affected corpus game.)

### 8.4 Limits / timing / input summary

- **Picture count caps:** 20 (2k legacy) / 40 (2k3 legacy) / 50 (Major-Updated) / 1000 (English)
  / 2000 (DynRPG). Single source of truth: `Game_Pictures::GetDefaultNumberOfPictures`
  (`src/game_pictures.cpp:129`).
- **Move-Picture on a map-fixed picture:** on **legacy** engines (2k ≤1.10 and 2k3 ≤1.04) a Move
  Picture command ignores the new x/y for a `fixed_to_map` picture; Major-Updated+ honors it —
  `ignore_position = Player::IsLegacy() && data.fixed_to_map` (`src/game_pictures.cpp:264`).
- **Refresh / timing model:** 60 fps logical update; not version-keyed.
- **Variable/switch counts:** 5000 (classic) vs 9999 (English 2k3 1.10+); a data-range fact, not
  a separate engine bit. (Maniac widens variables to 32-bit — out of scope here, see
  [maniac-patch.md](maniac-patch.md).)
- **Text escape codes:** the official set (`\v \n \c \s \! \. \| \> \< \^ \_ \$`) is version-
  stable; `\n[…]` name substitution and `%S/%O/%V/%U` placeholders are the version-keyed parts
  (§8.1/§8.2).

---

## 9. EasyRPG support matrix

| Capability | Status | Anchor / notes |
|---|---|---|
| 2k vs 2k3 classification | FULL | `EngineRpg2k`/`EngineRpg2k3`; exe + `ldb_id` (`src/player.cpp:821`, `src/exe_reader.cpp:483`) |
| Major-Updated detection (2k≥1.50 / 2k3≥1.05) | FULL | `EngineMajorUpdated`; exe CODE size, MP3/file-size tree (`src/exe_reader.cpp:499`, `src/filefinder.cpp:522`) |
| English/Steam detection (2k≥1.61 / 2k3≥1.10) | FULL | `EngineEnglish`; VERSIONINFO ver + `ultimate_rt_eb.dll` (`src/exe_reader.cpp:541`/`:562`, `src/player.cpp:823`) |
| DB version chunk `0x1A` for 2k 1.61 | FULL | `lcf::Data::data.version >= 1` (`src/player.cpp:828`; liblcf `chunks.h:1538`) |
| Picture-count caps per version | FULL | `src/game_pictures.cpp:129` |
| Pictures-during-message (1.60) | FULL | `!IsEnglish()` block + `PicUnlock` patch (`src/game_interpreter.cpp:2773` etc.) |
| Battle-message placeholders (1.61) | FULL | `Feature::HasPlaceholders()` (`src/feature.cpp:49`) |
| 2k3E battle/damage/message algorithm | FULL | `src/game_battle.cpp:197`, `src/window_message.cpp:107` |
| "This Event" in common events (1.12) | FULL | `src/game_interpreter.cpp:329` + `CommonThisEvent` patch |
| **1.60 vs 1.61** discrimination | WONTFIX | only ≥1.61 gets `EngineEnglish`; 1.60 hard to detect ([#588](https://github.com/EasyRPG/Player/issues/588), `src/player.h:218`) |
| **1.10/1.11/1.12/1.12a** discrimination | MISSING (by design) | all map to `IsRPG2k3E()`; no per-build bit |
| 1.12a Show-Picture flag-default fix | PARTIAL / RE-pending | not known to be special-cased; see §8.3 |
| 0%-effect strict-0 damage (1.50/1.05) | FULL | `Algo::VarianceAdjustEffect` `IsLegacy()` gate (`src/algo.cpp:166`) |
| Key-Input direction keys + Shift (1.50/1.05) | FULL | `CommandKeyInputProc` `IsRPG2kLegacy()`/`IsRPG2k3Legacy()` gates (`src/game_interpreter.cpp:3310`/`:3327`) |

Release-vs-master: engine classification is long-standing and present in releases (≥0.8.x); the
opt-in backport patches (`PicUnlock`, `CommonThisEvent`, `RPG2k3Commands`) are documented in
[easyrpg-extensions.md](easyrpg-extensions.md) / [runtime-micro-patches.md](runtime-micro-patches.md)
with their own release annotations.

---

## 10. Test assets

- **Beloved Rapture** — RM2k3 1.12a (+Maniac 220325); exercises the 2k3-English/Major-Updated
  path and the `1.1.2.1`/Maniac detection branch. See [../games/beloved-rapture.md](../games/beloved-rapture.md).
- **Ahriman's Prophecy** (Amaranth) — a 2k3 `1.0.8.0` exe **without VERSIONINFO**; the canonical
  test for the CODE-size ≥ 0xC7400 fallback (`src/exe_reader.cpp:502`).
- Any genuine VALUE! 2k game — tests the `logos==1, CODE≤0xB0000 → 2k MajorUpdated` branch.
- A classic 3-logo 2k 1.07 game and a legacy 2k3 ≤1.04 game — test the no-VERSIONINFO branches.
- Corpus: `c:\rg\easyrpg_library` (cross-ref `docs/games/`). Specific per-version repro assets
  are not yet curated. (TODO)

## 11. Open questions

- Exact build dates for English/Steam RM2k 1.60/1.61/1.62 and RM2k3 1.10/1.11/1.12/1.12a are
  community-sourced; no vendor changelog with ISO dates was found for several. (RE-pending)
- Does any shipped game depend on pre-1.12 "This Event"-in-common-event behavior, such that
  collapsing 1.10–1.12a hurts it? (hypothesis: no; needs corpus confirmation)
- ~~Precise interpreter anchor for the 0%-effect→strict-0 damage change and the Key-Input
  direction/Shift change.~~ **Resolved:** 0%-effect → `Algo::VarianceAdjustEffect`
  (`src/algo.cpp:166`, `IsLegacy()` gate); Key-Input → `CommandKeyInputProc`
  (`src/game_interpreter.cpp:3310`/`:3327`, `IsRPG2kLegacy()`/`IsRPG2k3Legacy()` gates).
- Whether EasyRPG should special-case the 1.12a Show-Picture flag-default regression fix. (§8.3)
- Can 1.60 ever be distinguished from 1.61 in practice (VERSIONINFO both `1.6.x`)? Currently
  treated as not-English. (open, per [#588](https://github.com/EasyRPG/Player/issues/588))

## 12. References

Primary / vendor:
- Official JP RM2k update page (versions 1.50/1.51/1.52, `RPG2000UP52.exe`) —
  <https://rpgmakerofficial.com/product/support/update_rpg2000.html>
- RM2k3 version history (phylomortis backup) —
  <https://easyrpg.github.io/wiki/development/technical-details/rpg-maker-2003-version-history/>
- Steam RM2k3 patch notes: 1.10 changelog —
  <https://steamcommunity.com/app/362870/discussions/0/618460171314070796/>; 1.12 —
  <https://store.steampowered.com/news/app/362870/view/3928784047051996238>; 1.12a —
  <https://store.steampowered.com/news/app/362870/view/3928784047051996141> /
  <https://steamdb.info/patchnotes/2173406/>
- Steam RM2k patch notes: 1.61 —
  <https://store.steampowered.com/news/app/383730/view/5260723642047802353>; 1.62 —
  <https://store.steampowered.com/news/app/383730/view/5260723642047801731>

Community / cross-reference:
- Makerpendium Patch-DB per-build matrices —
  <https://dev.makerpendium.de/docs/patch_db/main-en.htm> (e.g. `allow_msg_move.htm`,
  `affinity.htm` for the build labels)
- EasyRPG issue #588 (1.60/1.61 detection, placeholders) —
  <https://github.com/EasyRPG/Player/issues/588>
- VIPRPG dev wiki (JP version notes) — <https://wikiwiki.jp/viprpg-dev/2003>
- Don Miguel / RPG Advocate context — <https://www.makerpendium.de/index.php/Don_Miguel>,
  <https://rpgmaker.fandom.com/wiki/RPG_Advocate>
- Research synthesis (this spec's source) — `/home/john/research/other_patches_web.md` §9

In-tree (verified 2026-06-09):
- `src/player.h:38`–`:48` (`EngineType`), `:201`–`:237`/`:276`–`:303` (helper decls),
  `:448`–`:508` (inline helper defs), `:218` (`IsRPG2kE` doc comment: ≥1.61 only)
- `src/exe_reader.cpp:483`–`:567` (`GetEngineType`), `:451`–`:465` (`GetLogoCount`),
  `:270`–`:330` (`GetStartupLogos`), `:32` (logo CRCs), `:343`–`:397` (VERSIONINFO + Maniac
  string `:392`); `src/exe_reader.h:58` (`FileInfo` struct)
- `src/player.cpp:782`–`:839` (detection flow), `:795`/`:820` (`engine == EngineNone` gates),
  `:1465`–`:1470` (`--engine` help), `:697` (`CreateGameObjects`)
- `src/game_config_game.cpp:38`–`:59` (`--engine` token map, `LoadFromArgs`), `:215`
  (`engine_str.FromIni`)
- `src/filefinder.cpp:522`–`:568` (`IsMajorUpdatedTree`), `src/filefinder.h:367`–`:394`
  (size thresholds / `OFFICIAL_HARMONY_DLL` = 473600)
- `src/feature.cpp:49` (`HasPlaceholders`), `src/game_pictures.cpp:129` (picture caps),
  `:115` (savegame `frames`), `:264` (legacy fixed-to-map Move-Picture quirk),
  `src/sprite_picture.cpp:32`–`:34`, `src/game_battle.cpp:197`, `src/window_message.cpp:107`/`:303`,
  `src/game_interpreter.cpp:329` (This-Event), `:2773`/`:2920`/`:3075` (pics-during-message),
  `:3310`/`:3327` (Key-Input direction/Shift), `:1010`/`:1043` (ShowChoices/InputNumber),
  `src/game_message.cpp:381` (`ParseActor`/`ParseParam`), `src/algo.cpp:164`–`:171`
  (`VarianceAdjustEffect`, 0%-effect damage)
- liblcf: `lib/liblcf/src/generated/lcf/ldb/chunks.h:1538` (DB version chunk `0x1A`),
  `lib/liblcf/src/reader_struct.h:432`–`:455` (`DatabaseVersionField`)
