<!--
Spec: Bootlegs, translations & engine identification.
Part of the docs/forks/ series (see README.md). Follows TEMPLATE.md section order.
This document is about ENGINE IDENTIFICATION, not feature support: the binaries
catalogued here are re-skinned / translated / rebranded copies of the *stock* RPG_RT
runtime. They add no event commands and no file-format changes. EasyRPG only needs to
(a) classify them as the correct underlying RM2k/RM2k3 engine, and (b) suppress their
startup logos. Both jobs live in src/exe_reader.cpp.
-->

# Bootlegs, translations & engine identification (海賊版・翻訳版 / kaizokuban・hon'yakuban)

> This spec covers the family of **non-feature** RPG_RT.exe variants: unofficial English
> translations (Don Miguel, RPG Advocate), Spanish/Italian/French/Thai national
> translations and bootlegs (Hellsoft "RPG Maker PRO", SoLaCe, Rabbi-Bodom, Thaiware…),
> and pure logo-swap rebrands (Gnaf's Picture Patch, Rikku2000's Spezial-Patch). None of
> them add event commands, chunks, or runtime behavior beyond the stock engine they were
> built from. EasyRPG's only job is **identification**: map each binary to the right
> underlying RM2k/RM2k3 engine class (so the correct version behavior is selected — see
> [official-versions.md](official-versions.md)) and **not display their stock startup
> logos**. The work is done almost entirely in `src/exe_reader.cpp` via a curated list of
> startup-logo CRC32s plus a small set of byte-signature special cases.

**Status at a glance:** EasyRPG support = **FULL** for the identification job these binaries
require (they need no feature support). Detection = startup-logo CRC32 match + PE
section-size / VERSIONINFO heuristics + byte signatures in `src/exe_reader.cpp`. Spec
confidence = **HIGH** (the CRC table and decision tree are read verbatim from our tree;
provenance of individual logos is the engine authors' own annotations and is partly
self-described as unreliable — see §1).

---

## 1. Identity

These are distinct artifacts grouped here because they share one property: from EasyRPG's
point of view they are **the stock engine wearing a different coat**. The grouping follows
the EasyRPG blog post that introduced the curated logo collection,
["A collection of RPG Maker 2000/2003 (Bootleg) Logos"](https://blog.easyrpg.org/2025/03/a-collection-of-rpg-maker-2000-2003-bootleg-logos/)
(2025-03), and the CRC comment block in `src/exe_reader.cpp:36-79`.

| Artifact | Author / scene | What it is | License / redistribution | Liveness |
|---|---|---|---|---|
| **Don Miguel English RM2k** | Don Miguel (RU→EN) | First unofficial English translation of the RM2k editor + runtime; founded the international RPG Maker community; shipped sample game *Don's Adventures*; translated RTP filenames | Unofficial/unlicensed (Kadokawa/ASCII IP) | Dead; historical. [makerpendium.de/Don_Miguel](https://www.makerpendium.de/index.php/Don_Miguel) |
| **RPG Advocate English RM2k3** | RPG Advocate (~2003) | Unofficial English RM2k3 (successor to Don Miguel); distributed via phylomortis.com | Unofficial | Site dead since ~2008. [makerpendium.de/RPG_Advocate](https://www.makerpendium.de/index.php/RPG_Advocate), [rpgmaker.fandom/RPG_Advocate](https://rpgmaker.fandom.com/wiki/RPG_Advocate) |
| **Hellsoft "RPG Maker PRO" + ES RM2k3** | Hellsoft (Spanish scene) | Spanish bootlegs: "PRO 1.05" (actually a rebranded **RM95**!), "PRO 1.10/1.15", and Spanish RM2k3 translations covering builds 1.0.2–1.0.9 | Bootleg | [hellsoft.net/H5](https://www.hellsoft.net/H5/); logos curated in EasyRPG blog post (above) |
| **Spanish RM2k3 1.0.9.1** (Makerhack / fdelapena) | fdelapena / "Makerhack" | Spanish community translation of the final JP RM2k3 build | Community | EasyRPG blog post (above) |
| **Spanish RM2k "SoLaCe"** | SoLaCe (Spanish scene) | Spanish RM2k translation (3 logo variants) | Community | Added to EasyRPG 2025-03 (`src/exe_reader.cpp:74-75`, commit `2142cadec`) |
| **Italian RM2k translation** | Matteo Sciutteri & Christian Crocenzi | Italian RM2k translation | Community | EasyRPG blog post (above) |
| **Italian "RPG Maker 4.0"** | Bassking / Hel / DragoVerde | Italian RM2k rebrand "RPG Maker 4.0" | Community | EasyRPG blog post (above) |
| **French "Rabbi-Bodom" RM2k3 1.0.9.1** | Rabbi-Bodom (French scene) | French translation of RM2k3 1.0.9.1 | Community | EasyRPG blog post (above) |
| **Thai "Thaiware" RM2k** | Thaiware | Thai translation of RM2k (VALUE!-era), 3 logo variants | Community | EasyRPG blog post (above) |
| **Thai "House of the Dev" RM2k** | House of the Dev | Thai RM2k translation, 3 logo variants | Community | EasyRPG blog post (above) |
| **Thai "Somprasongk Team" RM2k3 1.0.6** | Somprasongk Team | Thai RM2k3 1.0.6 translation | Community | EasyRPG blog post (above) |
| **Spezial-Patch** | Rikku2000 ("Big Vio Soft") | RM2k 1.51 rebrand with a swapped startup logo only | Rebrand | [makerpendium.de/Spezial-Patch_(RPG_Maker_2000)](https://www.makerpendium.de/index.php/Spezial-Patch_(RPG_Maker_2000)) |
| **Gnaf's Picture Patch** | Gnaf | Famous **fake**: claimed to raise the picture limit; is just a rebranded **VALUE! 1.50** RPG_RT with the logo swapped — does nothing | Rebrand | [makerpendium.de/Gnaf](https://www.makerpendium.de/index.php/Gnaf) |
| **WhiteDragon Italian RM2k3 1.0.8** | "WhiteDragon" (Italian scene) | RM2k3 1.0.8.0 byte-patch that **raises stat/var caps** — the one entry here that *does* change runtime behavior (handled as a constant-override, §6) | Patch | Recognized by `NoTitolo` string (`src/exe_reader.cpp:600`) |
| **Ahriman's Prophecy** (special case) | Amaranth Games (Aldorlea) | A commercial game shipping a modified RM2k3 **1.0.8.0 RPG_RT without VERSIONINFO** — special-cased because the missing VERSIONINFO breaks normal version detection | Commercial game | [Aldorlea](https://www.aldorlea.org/) |

**License / EULA note.** All of these except *Ahriman's Prophecy* and *WhiteDragon* are
unofficial copies of a copyrighted runtime. EasyRPG ships only their **logo CRC32 hashes**
(not the logos or the binaries), which is why this spec catalogues hashes rather than test
assets. The hashes are derived from binaries "that have historically been in circulation"
(`src/exe_reader.cpp:37-44`).

**Provenance caveat (read before trusting the version labels).** The version strings in the
CRC table are the authors' own annotations from recovered installers/readmes and are
**explicitly flagged as unreliable** by the EasyRPG maintainers:

> "The specified version strings here refer to the info given in either the recovered
> Installer packages or is taken from accompanying Readme files & do not necessarily give
> reliable information about the actual, original RPG_RT version on which these
> translations & patches were based on!" — `src/exe_reader.cpp:37-44`

So "Hellsoft RPG Maker PRO 1.05" being actually RM95 (not RM2k) is the rule, not the
exception: treat the *label* as a name, and the *engine class* as whatever
`FileInfo::GetEngineType()` decides from PE structure (§4).

The original **official** runtimes these derive from (Don Miguel/RPG Advocate ⇐ JP RM2k
1.05/1.07 and JP RM2k3 1.08/1.09; the rebrands ⇐ RM2k VALUE! 1.50/1.51) are documented in
[official-versions.md](official-versions.md).

---

## 2. Target engine builds

Each artifact is a whole replacement `RPG_RT.exe` (translations re-skin/re-string the
official binary; rebrands swap only the logo resource). They do **not** patch an existing
RPG_RT in place. The underlying official build each is "based on" is, per the labels (with
the §1 caveat):

| Artifact | Nominal underlying engine | EasyRPG-classified engine (via §4) |
|---|---|---|
| Don Miguel EN RM2k | RM2k 1.05/1.07-era | `EngineRpg2k` (3 logos, no VERSIONINFO) |
| RPG Advocate EN RM2k3 | RM2k3 ≈1.08/1.09 (reports `1.0.9.1` VERSIONINFO) | `EngineRpg2k3 \| MajorUpdated` (ver 1.0.x, minor ≥5) |
| Hellsoft "PRO 1.05" | actually **RM95** | (out of scope — RM95 not an RPG_RT engine) |
| Hellsoft "PRO 1.10/1.15" | RM2k-era | `EngineRpg2k` family |
| Hellsoft ES RM2k3 1.0.2 / 1.0.4 / 1.0.7 / 1.0.8 / 1.0.9 | RM2k3 1.0.x | `EngineRpg2k3` (±`MajorUpdated` by minor) |
| Spanish 1.0.9.1, French Rabbi-Bodom 1.0.9.1 | RM2k3 1.0.9.1 | `EngineRpg2k3 \| MajorUpdated` |
| Italian RM2k, "RPG Maker 4.0", SoLaCe ES RM2k | RM2k | `EngineRpg2k` family |
| Thai Thaiware/HoTD RM2k | RM2k | `EngineRpg2k` family |
| Thai Somprasongk RM2k3 1.0.6 | RM2k3 1.0.6 | `EngineRpg2k3 \| MajorUpdated` |
| Spezial-Patch | RM2k **1.51** (VALUE!) | `EngineRpg2k \| MajorUpdated` (1 logo, code ≤0xB0000) |
| Gnaf's Picture Patch | RM2k **1.50** (VALUE!) | `EngineRpg2k \| MajorUpdated` |
| WhiteDragon | RM2k3 **1.0.8.0** | `EngineRpg2k3 \| MajorUpdated` + constant overrides (§6) |
| Ahriman's Prophecy | RM2k3 **1.0.8.0** (no VERSIONINFO) | `EngineRpg2k3 \| MajorUpdated` via CODE-size special case (§4) |

Detection never trusts the marketing label; it derives the engine from PE structure (§4).

---

## 3. Version lineage

There is no single product line here — this is a registry of independent binaries. The
relevant lineage is **when EasyRPG learned to recognize each one**, i.e. the growth of the
`logo_crc32` list and the special cases in `src/exe_reader.cpp`. All commits below are in
our tree (`/home/john/Player`, branch `RISKY`); upstream-equivalent.

| Commit | Date | Change | Published |
|---|---|---|---|
| `36b3139d5` | 2023-05-13 | Detect Maniac Patch from RPG_RT.exe (VERSIONINFO machinery this spec reuses) | yes |
| `61db60f8f` | 2023-09-23 | Special-case **Ahriman's Prophecy** (1.0.8.0 without VERSIONINFO) | yes |
| `7debbb89c` / `d8289a30b` | 2023-11-20 | Load startup logos from RPG_RT.exe; switch logo hashing from `djb2` to **CRC32** | yes |
| `e95b9899d` | 2023-11-20 | **Skip Kadokawa stock startup logos by default**; add `StartupLogos` setting (`None`/`Custom`/`All`). The first 5 CRCs (`src/exe_reader.cpp:34`) are the stock Kadokawa logos. | yes |
| `dd1f4faa7` | 2023-11-20 | Support 2k VALUE! `LOGO` resource name (vs `LOGO1`) | yes |
| `5ffacf0b0` | 2025-01-28 | Extend CRC list with bootleg-bundled logos (initial Hellsoft/translations batch) | yes |
| `77b661df9` | 2025-01-28 | Thai bootleg translations (Thaiware / House of the Dev / Somprasongk) | yes |
| `838de3574` | 2025-01-29 | More bootleg translations | yes |
| `594e6f8a7` | 2025-01-30 | More Hellsoft variants + the "unreliable version strings" clarifying comment | yes |
| `66b706af9` | 2025-02-10 | Recompressed unaltered RM2k logos + pure logo-swap rebrands (Gnaf, Rikku2000 Spezial) | yes |
| `2142cadec` | 2025-03-03 | SoLaCe (Spanish) logos; fix mislabel "Brazilian → Spanish" for the 1.0.9.1 entry | yes |
| (blog) | 2025-03 | Public write-up: ["A collection of RPG Maker 2000/2003 (Bootleg) Logos"](https://blog.easyrpg.org/2025/03/a-collection-of-rpg-maker-2000-2003-bootleg-logos/) | yes |
| `33292ecc0` | 2025-04-28 | WhiteDragon (Italian) + StatDelimiter constant-override patches (`GetOverriddenGameConstants`, §6) | yes |
| `b8c59d018` | 2025-07-31 | Parse Maniacs build number from EXE (shares the VERSIONINFO parser) | yes |

`git log --oneline -- src/exe_reader.cpp` reproduces this list.

---

## 4. Detection

All identification logic lives in `src/exe_reader.cpp`. There are **two independent jobs**,
which must not be confused:

1. **Engine classification** — `EXEReader::FileInfo::GetEngineType(int& mp_version)`
   (`src/exe_reader.cpp:483-567`): decide which RM2k/RM2k3 engine bitmask the binary is, so
   the right version behavior is selected. This is what makes a translated/rebranded binary
   behave like its underlying engine. It does **not** read the logo CRC list.
2. **Startup-logo suppression** — `EXEReader::GetLogos()`
   (`src/exe_reader.cpp:259-330`): when extracting startup logos from the EXE, drop any whose
   CRC32 is in the curated `logo_crc32` list (`src/exe_reader.cpp:33-80`), so stock/bootleg
   logos are not shown. This is what the CRC table is for; it has **no effect on engine
   identification or features**.

### 4.1 Engine classification decision tree (`GetEngineType`)

Inputs come from `FileInfo` (`src/exe_reader.h:58-72`): `version`/`version_str`
(VS_FIXEDFILEINFO product version), `logos` (count of XYZ startup-logo resources),
`code_size` (PE `CODE` section virtual size), `cherry_size` (`CHER`/CHERRY section),
`geep_size` (`GEEP` section), `machine_type`, `is_easyrpg_player`, `maniac_patch_version`.
The tree (`src/exe_reader.cpp:483-567`):

| Condition | Verdict | Anchor | Bootlegs hitting this branch |
|---|---|---|---|
| `is_easyrpg_player` or unknown machine | `EngineNone` (don't detect) | :486-488 | — (EasyRPG itself) |
| **No VERSIONINFO**, `logos == 3` | `EngineRpg2k` | :492-494 | Don Miguel EN RM2k; old Hellsoft PRO; Italian/Thai RM2k translations (pre-VALUE!) |
| No VERSIONINFO, `logos == 1`, `code_size > 0xB0000`, `code_size >= 0xC7400` | `EngineRpg2k3 \| MajorUpdated` | :498-503 | **Ahriman's Prophecy** (1.0.8.0, no VERSIONINFO) |
| No VERSIONINFO, `logos == 1`, `0xB0000 < code_size < 0xC7400` | `EngineRpg2k3` | :504-506 | RM2k3 <1.0.2.1-based translations w/o VERSIONINFO |
| No VERSIONINFO, `logos == 1`, `code_size <= 0xB0000` | `EngineRpg2k \| MajorUpdated` (VALUE! 1.5x) | :509 | **Gnaf's Picture Patch** (1.50), **Spezial-Patch** (1.51) |
| `logos == 0`, version `1.1.2.1`, no CODE, no CHERRY | Maniacs full rewrite → `2k3\|MajorUpdated\|English` | :521-530 | — (Maniac Patch; see [maniac-patch.md](maniac-patch.md)) |
| VERSIONINFO present, `logos != 1` (and not the Maniacs 0-logo case) | `EngineNone` | :536-538 | — |
| VERSIONINFO `1.6.x`, 1 logo | `EngineRpg2k \| MajorUpdated \| English` (official EN RM2k) | :540-543 | (official Steam RM2k) |
| VERSIONINFO `1.0.x`, 1 logo, minor `< 5` | `EngineRpg2k3` | :546-547 | Hellsoft ES 1.0.2/1.0.4 |
| VERSIONINFO `1.0.x`, 1 logo, minor `>= 5` | `EngineRpg2k3 \| MajorUpdated` | :548-550 | **RPG Advocate**, French Rabbi-Bodom, Spanish/Thai 1.0.6+/1.0.9.1, WhiteDragon |
| VERSIONINFO `1.1.2.1`, 1 logo | old Maniacs (hacked 1.1.2.1) → `2k3\|MajorUpdated\|English` | :551-563 | — (see [maniac-patch.md](maniac-patch.md)) |
| else | `EngineNone` | :566 | — |

Key thresholds to replicate: `CODE`-section size `0xB0000` separates RM2k VALUE! from RM2k3
(no-VERSIONINFO path), and `0xC7400` marks RM2k3 ≥1.0.5.0 (`MajorUpdated`). The
Ahriman's-Prophecy branch (`:499-503`) exists *only* because that one binary lacks a
VERSIONINFO it normally would have; the comment says so verbatim (`:501-502`).

When `GetEngineType` returns `EngineNone`, `Player::CreateGameObjects` falls back to LDB-based
detection (`lcf::Data::system.ldb_id == 2003`, `ultimate_rt_eb.dll`, `Data::data.version`) —
`src/player.cpp:820-837`. **`ultimate_rt_eb.dll`** is the marker for the **official English
Steam RM2k3** (it ships that DLL); its presence sets `EngineEnglish | EngineMajorUpdated`
(`src/player.cpp:823-825`). That is an engine flag, *not* a patch — it does not appear in the
`patch_*` config (contrast `harmony.dll`/`dynloader.dll`/`accord.dll`/`warp.dll`, which do).

### 4.2 Startup-logo CRC32 table (`logo_crc32`)

`src/exe_reader.cpp:33-80`, a `constexpr std::array` of **36** CRC32 values. Reproduced
exactly (the comments are the maintainers' provenance annotations; see §1 caveat):

| CRC32 | Provenance (per `exe_reader.cpp` comments) | Engine implied |
|---|---|---|
| `0xdf3d86a7` | **Stock Kadokawa logo** (skipped by default since commit `e95b9899d`) | RM2k/2k3 stock |
| `0x2ece66f9` | Stock Kadokawa logo | stock |
| `0x2fe0de56` | Stock Kadokawa logo | stock |
| `0x25c4618f` | Stock Kadokawa logo | stock |
| `0x91b2635a` | Stock Kadokawa logo | stock |
| `0x6a88587e` | Recompressed, unaltered RPG2000 logo | RM2k |
| `0x4beedd9a` | Recompressed, unaltered RPG2000 logo | RM2k |
| `0x1c7f224b` | Recompressed, unaltered RPG2000 logo | RM2k |
| `0x5ae12b1c` | Hellsoft bootleg "RPG Maker PRO 1.05" (actually RM95) | RM95 |
| `0x3d1cb5f1` | Hellsoft "RPG Maker PRO 1.05" | RM95 |
| `0x04a7f11a` | Hellsoft "RPG Maker PRO 1.05" | RM95 |
| `0x9307807f` | Hellsoft bootleg "RPG Maker PRO 1.10" | RM2k |
| `0x652529ec` | Hellsoft "RPG Maker PRO 1.10" | RM2k |
| `0x5e73987b` | Hellsoft "RPG Maker PRO 1.10" | RM2k |
| `0x2e8271cb` | Hellsoft bootleg "RPG Maker PRO 1.15" | RM2k |
| `0x4e3f7560` | Hellsoft translation of RM2k3 "1.0.2" | RM2k3 |
| `0x59ab3986` | Hellsoft translation of RM2k3 "1.0.4" & "1.0.7" | RM2k3 |
| `0xd333b2dd` | Hellsoft translation of RM2k3 "1.0.8" & "1.0.9" | RM2k3 |
| `0x476138cb` | French "Rabbi-Bodom" translation of RM2k3 1.0.9.1 | RM2k3 |
| `0x29efaf6a` | "Thaiware" translation of RM2k | RM2k |
| `0xfeb8f6b2` | "Thaiware" translation of RM2k | RM2k |
| `0x265855ad` | "Thaiware" translation of RM2k | RM2k |
| `0xa8be4ed3` | Thai "House of the Dev" translation of RM2k | RM2k |
| `0xc75ccc6d` | Thai "House of the Dev" translation of RM2k | RM2k |
| `0xcea40e5f` | Thai "House of the Dev" translation of RM2k | RM2k |
| `0xc9b2e174` | Thai "Somprasongk Team" translation of RM2k3 1.0.6 | RM2k3 |
| `0x1a1ed6dd` | Italian translation of RM2k (Matteo S. & Christian C.) | RM2k |
| `0xad73ccf5` | Italian translation of RM2k | RM2k |
| `0x4ad55e84` | Italian translation of RM2k | RM2k |
| `0x8afe1239` | Italian "RPG Maker 4.0" patch of RM2k | RM2k |
| `0x089fb7d8` | Spanish version of RM2k3 1.0.9.1 | RM2k3 |
| `0x544ffca8` | Spanish version of RM2k (SoLaCe) | RM2k |
| `0x4fbc0849` | Spanish version of RM2k (SoLaCe) | RM2k |
| `0x7420f415` | Spanish version of RM2k (SoLaCe) | RM2k |
| `0x806b6877` | **Spezial-Patch** by Rikku2000 (RM2k 1.51 with swapped logo) | RM2k VALUE! 1.51 |
| `0xc5e846a7` | **Gnaf's Picture Patch** (RM2k 1.50 with swapped logo) | RM2k VALUE! 1.50 |

The CRC32 is computed over the **raw XYZ logo bytes** as extracted from the resource
(`crc32(0, logo.data(), logo.size())`, `src/exe_reader.cpp:315`; zlib `crc32`). The first
five entries (`:34`) are the stock Kadokawa logos; the rest (`:46-79`, under the "bootleg
versions" comment block) are the translation/rebrand logos.

### 4.3 How the CRC table is consumed (logo suppression only)

`GetLogos()` (`src/exe_reader.cpp:259-330`) is gated by the player setting
`show_startup_logos` (`Game_ConfigPlayer::show_startup_logos`, default **`Custom`**;
`src/game_config.h:91-95`, enum `StartupLogos { None, Custom, All }` at
`src/game_config.h:55-59`):

| `StartupLogos` value | ini `[Player] StartupLogos` | Behavior in `GetLogos()` |
|---|---|---|
| `None` | `none` | return no logos at all (`:266-268`) |
| **`Custom`** (default) | `custom` | extract logos, but **drop any whose CRC32 is in `logo_crc32`** (`:314-318`) — i.e. show only genuinely game-author-custom logos |
| `All` | `all` | show every embedded logo, including stock Kadokawa ones (`:319-321`) |

So a CRC match means "this is a known stock/bootleg logo, don't display it." It is purely
cosmetic; nothing about gameplay, version detection, or feature support depends on it. The
extracted, non-suppressed logos are later shown by `Scene_Logo`
(`src/scene_logo.cpp:212-241`, `LoadLogos()` falls back to `EXEReader::GetLogos()` when no
loose `Logo/LOGO*` images exist).

### 4.4 PE structural markers used alongside the logo count

`EXEReader` records special PE section names while walking sections
(`src/exe_reader.cpp:141-150`):

| Section name (FourCC) | `FileInfo` field | Meaning |
|---|---|---|
| `CODE` (`0x45444F43`) | `code_size`, `code_ofs` | main code section; size drives the version thresholds in §4.1 |
| `CHER` (`0x52454843`) | `cherry_size` | Cherry-patched exe (DynRPG / Hyper-Patcher lineage) |
| `GEEP` (`0x50454547`) | `geep_size` | EasyRPG-extended exe (also early Maniacs) |
| `UPX0` (`0x30585055`) | — (debug warning only) | UPX-packed; "Engine detection could be incorrect" (`:149`) |

None of these are produced by the *translations/rebrands* in this spec (those are plain
re-skins of the official exe); they matter to sibling specs
([maniac-patch.md](maniac-patch.md), [dynrpg.md](dynrpg.md),
[easyrpg-extensions.md](easyrpg-extensions.md)) and are listed here only so a reader of the
detection code knows what each section name is for.

### 4.5 EasyRPG config mapping

None of the artifacts in this spec map to a `Game_ConfigGame` `patch_*` flag — they are
engine classes, not patches. The relevant config surface is:

- **`[Game] Engine=` / `--engine`** (`src/game_config_game.cpp:38-59`): manual override of
  the engine bitmask. Values `rpg2k|2000`, `rpg2kv150|2000v150`, `rpg2ke|2000e`,
  `rpg2k3|2003`, `rpg2k3v105|2003v105`, `rpg2k3e|2003e`. Use this to force the right engine
  for a bootleg whose binary auto-detects wrong.
- **`[Game] EnginePath=` / `--engine-path`** (CLI parse `src/game_config_game.cpp:80-87`;
  consumed at `src/player.cpp:785-788`): point detection at an alternate `.exe`.
- **`[Player] StartupLogos`** (`src/game_config.h:91`): the only setting the CRC table
  obeys.

The patch-override precedence note (manual patch flags set `patch_override` and disable
DLL/auto-detection) is in [easyrpg-extensions.md](easyrpg-extensions.md); it does not gate
engine classification, which always runs (`src/player.cpp:795-803`).

---

## 5. Event commands

**Not applicable.** None of these binaries add, remove, or alter event-command codes — they
are translations/rebrands of the stock interpreter. Games made with them emit ordinary
RM2k/RM2k3 command codes, which EasyRPG already handles. (The one runtime-affecting entry,
WhiteDragon, only widens *value caps*, not commands — see §6.) The baseline command set is in
liblcf (`lib/liblcf/src/generated/lcf/rpg/eventcommand.h`); fork command extensions are
documented per-fork (e.g. [maniac-patch-commands.md](maniac-patch-commands.md)).

---

## 6. Modified baseline commands

Only **WhiteDragon** (Italian RM2k3 1.0.8 byte-patch) and the generic **StatDelimiter**
patch modify baseline behavior, and only by **raising numeric caps** — they change the
*range* of values commands like ChangeHP/ChangeParameters/ChangeGold accept, not the commands
themselves. EasyRPG models this as **game-constant overrides**, not as command changes.

`EXEReader::GetOverriddenGameConstants()` (`src/exe_reader.cpp:569-615`) keys on the `CODE`
section size, then confirms with a byte signature, then applies a preset:

| `code_size` | Underlying build | Signature check | Preset applied |
|---|---|---|---|
| `0x9CC00` | RM2k 1.62 | `"XXX"` (3× `POP EAX`) at `code_ofs + 0x07DAA6` | `StatDelimiter` |
| `0xC8E00` | RM2k3 1.0.8.0 | `"NoTitolo"` at `code_ofs + 0x08EBE0` | **`Rm2k3_Italian_WD_108`** (WhiteDragon) |
| `0xC8E00` | RM2k3 1.0.8.0 | `"XXX"` at `code_ofs + 0x09D279` | `StatDelimiter` |
| `0xC9000` | RM2k3 1.0.9.1 | `"XXX"` at `code_ofs + 0x09C5AD` | `StatDelimiter` |

The `"NoTitolo"` string is the WhiteDragon-translated form of an RPG_RT internal string and is
"the only one to translate this string," making it a reliable single-signature detector
(`src/exe_reader.cpp:597-600`). The presets (`src/game_constants.h:139-158`):

**`Rm2k3_Italian_WD_108` (WhiteDragon):**

| Constant | Override |
|---|---|
| `MinVarLimit` / `MaxVarLimit` | ±999,999,999 |
| `MaxActorHP` | 99,999 |
| `MaxActorSP` | 9,999 |
| `MaxStatBaseValue` | 9,999 |
| `MaxDamageValue` | 99,999 |
| `MaxGoldValue` | 9,999,999 |

**`StatDelimiter`:**

| Constant | Override |
|---|---|
| `MaxActorHP` / `MaxActorSP` | 9,999,999 |
| `MaxStatBaseValue` / `MaxStatBattleValue` | 999,999 |

Overrides are applied at `src/player.cpp:871-876` via
`Game_Constants::OverrideGameConstant`. The same caps can alternatively be set per-game via
LDB `System.easyrpg_*` fields (see [easyrpg-extensions.md](easyrpg-extensions.md) and
[runtime-micro-patches.md](runtime-micro-patches.md), which covers StatDelimiter as a
behavioral patch in its own right). The remaining artifacts in this spec apply **no** baseline
modifications.

---

## 7. File-format changes

**None.** Translations and rebrands do not introduce new or changed LDB/LMU/LMT/LSD chunks,
new standalone files, or ini keys. A game built with Don Miguel's editor or a Hellsoft
bootleg produces a stock-format project; liblcf reads it with no special handling. (The one
file-system convention worth noting is Don Miguel's **translated RTP filenames**, which is a
*resource-naming* matter, not a file-format one — see §8.)

---

## 8. Runtime behavior changes

Aside from the value-cap widening of WhiteDragon/StatDelimiter (§6), the binaries here are
behaviorally identical to the official engine they derive from. The only runtime-relevant
quirks EasyRPG cares about:

- **Startup-logo display.** Stock/bootleg logos are suppressed by default (§4.3). This is the
  primary reason the CRC table exists.
- **Don Miguel translated RTP filenames.** Don Miguel's RTP renamed the bundled resource
  files. EasyRPG maintains RTP-name lookup/migration tables so games referencing the
  translated names resolve correctly; this is RTP-database territory, not engine detection.
  See the [RTP migration database](https://easyrpg.github.io/wiki/rtp-database/migration-2000-2003/).
- **Early-translation "Enter Hero Name" freeze bug.** Some early Don Miguel-era English RM2k
  runtimes froze on the *Enter Hero Name* screen (community-fixed by Deejee's "Enter Hero Name
  Patch", a modified RPG_RT for "RM2000 v1.05"). EasyRPG reimplements the screen correctly and
  is not subject to the bug, so no emulation is needed; noted here for corpus forensics
  ([rpgmaker.net topic 689](https://rpgmaker.net/forums/topics/689/)). (UNVERIFIED that any
  EasyRPG code path special-cases this binary; treat as informational.)
- **Encoding.** National translations frequently imply a non-UTF-8 codepage (Thai CP874,
  Spanish/Italian/French CP1252, Don Miguel CP1252). Encoding selection is a separate
  subsystem (`Player::IsCP932/949/936/Big5/CP1251`, `src/player.h:243-273`; per-game
  `encoding` config) and is independent of the logo/engine detection here. (hypothesis: a Thai
  bootleg game needs `encoding=874` set manually if the project lacks an `ldb` codepage; not
  verified against a Thai asset.)

Resolution, picture/variable limits, timing, input, audio, and save semantics are exactly
those of the underlying official engine (selected via §4.1 and documented in
[official-versions.md](official-versions.md)).

---

## 9. EasyRPG support matrix

The "feature" these binaries demand is *correct identification*, and that is FULL.

| Capability | Status | Anchor / reference |
|---|---|---|
| Classify translated/rebranded EXE as correct RM2k/RM2k3 engine | **FULL** | `FileInfo::GetEngineType` `src/exe_reader.cpp:483-567` |
| Suppress stock Kadokawa startup logos by default | **FULL** | `GetLogos` + `logo_crc32` `src/exe_reader.cpp:259-330,33-80`; setting `src/game_config.h:91` |
| Suppress known bootleg/translation startup logos | **FULL** (36 CRCs catalogued) | `src/exe_reader.cpp:46-79`; commits `5ffacf0b0`…`2142cadec` |
| Recognize Ahriman's Prophecy (1.0.8.0 w/o VERSIONINFO) | **FULL** | `src/exe_reader.cpp:499-503`; commit `61db60f8f` |
| Recognize official English Steam RM2k3 via `ultimate_rt_eb.dll` | **FULL** | `src/player.cpp:823-825` |
| WhiteDragon (Italian 1.0.8) raised caps | **FULL** | `GetOverriddenGameConstants` `src/exe_reader.cpp:596-602`; preset `src/game_constants.h:141-149` |
| StatDelimiter raised caps (RM2k 1.62 / RM2k3 1.0.8 / 1.0.9.1) | **FULL** | `src/exe_reader.cpp:590-611`; preset `src/game_constants.h:151-157` |
| Translated RTP filename resolution (Don Miguel) | **FULL** (RTP DB, separate subsystem) | [RTP migration DB](https://easyrpg.github.io/wiki/rtp-database/migration-2000-2003/) |
| National-translation encoding auto-selection | **PARTIAL** (may need manual `encoding=`) | `src/player.h:243-273` (UNVERIFIED for Thai assets) |

Release annotation: the bootleg-logo CRC collection and the WhiteDragon/StatDelimiter
constant overrides landed in **2025** (commits above) — after the 0.8.1 release tree this fork
is based on (`CMakeLists.txt` version 0.8.1). Treat them as **master-only** unless a newer
release has shipped. The stock-logo skip and Ahriman's Prophecy case predate them (2023).

There is no "WONTFIX" item here; the open work is only *coverage* (more bootleg CRCs) and
better encoding auto-detection (§11).

---

## 10. Test assets

EasyRPG ships **hashes, not binaries**, so there is no in-tree test asset for these. For
corpus validation use the EasyRPG library corpus (`c:\rg\easyrpg_library`) and look for:

- A project whose `RPG_RT.exe` embeds a logo matching a `logo_crc32` entry (verify the logo
  is suppressed in `Custom` mode and shown in `All` mode).
- **Ahriman's Prophecy** (Amaranth Games) — exercises the no-VERSIONINFO 1.0.8.0 branch
  (`src/exe_reader.cpp:499-503`).
- A WhiteDragon-patched Italian RM2k3 1.0.8 game — exercises the `NoTitolo` constant override.
- A Don Miguel-era English RM2k game — exercises translated-RTP filename resolution.
- A Thai/Spanish/Italian translation game — exercises encoding selection (§8).

No sibling `docs/games/` entry currently targets a pure-bootleg case;
[../games/beloved-rapture.md](../games/beloved-rapture.md) is a Maniacs game, not relevant
here except as the project driving the broader effort.

---

## 11. Open questions

- **Encoding auto-detection** for national translations: does any code path infer codepage
  from the detected bootleg, or must `encoding=` always be set per-game? (Suspected manual;
  UNVERIFIED.)
- **CRC table coverage** is curated and incomplete by nature — the maintainers add hashes as
  bootlegs surface. Are there common bootlegs (e.g. Brazilian-PT, Korean, Russian RM2k
  translations) whose logos are not yet hashed? (RE-pending: would need samples.)
- **Mislabel risk**: because version strings are author-supplied and unreliable
  (`src/exe_reader.cpp:37-44`), some CRC comments may name the wrong underlying build. The
  one already-fixed instance was "Brazilian → Spanish" for `0x089fb7d8` (commit `2142cadec`).
  Are others mislabeled? (RE-pending.)
- **"Enter Hero Name" freeze**: confirm whether EasyRPG needs (or has) any special handling
  for Deejee-patched vs unpatched early English RM2k runtimes, or whether the reimplemented
  screen makes it moot. (UNVERIFIED.)
- **Ahriman's Prophecy** is the only special-cased commercial game; are there other commercial
  RM2k/2k3 titles shipping VERSIONINFO-stripped or otherwise-mangled runtimes that fall
  through to `EngineNone`? (RE-pending.)

---

## 12. References

Primary (our tree):
- `src/exe_reader.cpp` — logo CRC table (`:33-80`), `GetLogos` suppression (`:259-330`),
  `GetEngineType` decision tree (`:483-567`), `GetOverriddenGameConstants` (`:569-615`).
- `src/exe_reader.h` — `FileInfo` struct (`:58-72`).
- `src/player.cpp:784-837` — detection driver, LDB fallback, `ultimate_rt_eb.dll`.
- `src/game_config.h:55-95` — `StartupLogos` enum and setting.
- `src/game_constants.h:123-158` — `KnownPatchConfigurations` presets.
- `src/scene_logo.cpp:212-241` — startup logo display.

Primary (external):
- EasyRPG blog, ["A collection of RPG Maker 2000/2003 (Bootleg) Logos"](https://blog.easyrpg.org/2025/03/a-collection-of-rpg-maker-2000-2003-bootleg-logos/) (2025-03).
- Makerpendium wiki: [Don Miguel](https://www.makerpendium.de/index.php/Don_Miguel),
  [RPG Advocate](https://www.makerpendium.de/index.php/RPG_Advocate),
  [Gnaf](https://www.makerpendium.de/index.php/Gnaf),
  [Spezial-Patch (RM2k)](https://www.makerpendium.de/index.php/Spezial-Patch_(RPG_Maker_2000)).
- [RPG Maker 2003 version history (phylomortis backup)](https://easyrpg.github.io/wiki/development/technical-details/rpg-maker-2003-version-history/).
- [RTP migration database (RM2000↔2003)](https://easyrpg.github.io/wiki/rtp-database/migration-2000-2003/).
- Hellsoft repository: [hellsoft.net/H5](https://www.hellsoft.net/H5/).
- [rpgmaker.fandom.com — RPG Advocate](https://rpgmaker.fandom.com/wiki/RPG_Advocate).

Sibling specs:
- [official-versions.md](official-versions.md) — the official RM2k/2k3 version matrix the
  classified engine maps onto.
- [easyrpg-extensions.md](easyrpg-extensions.md) — `[Game] Engine=`, patch override
  precedence, `[Player]` settings.
- [runtime-micro-patches.md](runtime-micro-patches.md) — StatDelimiter as a behavioral patch.
- [maniac-patch.md](maniac-patch.md) / [dynrpg.md](dynrpg.md) — the `CHER`/`GEEP`/VERSIONINFO
  PE markers and the 1.1.2.1 Maniacs branches in the same `GetEngineType` tree.

Research provenance (outside repo): `/home/john/research/other_patches_web.md` §6,
`/home/john/research/easyrpg_patch_inventory.md` §1.2, §7.1.
