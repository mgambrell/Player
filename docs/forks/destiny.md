<!--
Spec: Destiny Patch. Part of the EasyRPG fork-support series (docs/forks/).
Follows TEMPLATE.md section order. Evidence: EasyRPG/liblcf source (file:line into
/home/john/Player), the Bananen-Joe docs mirrored at cherrytree.at, and the research
corpus (/home/john/research/other_patches_web.md, easyrpg_patch_inventory.md).
-->

# Destiny Patch (Destiny / „Destiny-Patch")

> Destiny is a German-scene runtime extension for **RPG Maker 2000** by **Bananen-Joe**
> (David Gausmann). It is delivered as a patcher (`DestinyPatcher`) plus a runtime DLL
> (`Destiny.dll`) that adds an embedded scripting language, **DestinyScript**, written
> inside event **Comment** commands whose first character is `$`. DestinyScript gives RM2k
> games floating-point math, string handling, picture processing, file I/O, and Internet
> (socket) communication — capabilities the stock RM2k runtime lacks. A game uses Destiny to
> escape RM2k's integer-only variables and 50-picture/limited-feature sandbox without moving
> to RM2k3 or a newer engine. The patcher half doubles as a generic RPG_RT tweaker (key
> assignment, default window mode, picture count, etc.).

**Status at a glance:** EasyRPG support = **STUB**. Detection = **`Destiny.dll` present in the
game folder** (auto-detect only). Spec confidence = **MEDIUM** — detection, data-container
scaffolding, and the (non-functional) interpreter skeleton are verified in-tree against
`src/game_destiny.{h,cpp}`; the DestinyScript language semantics and the binary container
layout are documented from the author's published docs and the unfinished EasyRPG constant
tables, and are **(RE-pending)** because no DestinyScript opcode is actually executed yet.

## 1. Identity

| Field | Value |
|---|---|
| Name (EN) | Destiny Patch / Destiny |
| Name (DE) | „Destiny-Patch" |
| Components | **DestinyPatcher** (the patcher tool) + **`Destiny.dll`** (runtime extension) + **DestinyScript** (the embedded language) |
| Author | **Bananen-Joe** = **David Gausmann** (German RPG Maker scene) |
| Continuation | **KotatsuAkira (炬燵あきら)** has maintained an extended, *unreleased* version since 2016 ([Makerpendium: Destiny Patch](https://www.makerpendium.de/index.php/Destiny_Patch)) |
| License | **Open-sourced in 2012** alongside the final v2.0.1.8 release (source published; license terms not restated here — verify before shipping derived assets) |
| Docs (EN) | Mirrored by Cherry at cherrytree.at: [Introduction](https://cherrytree.at/misc/destiny/DestinyPatcher_Eng/Introduction.htm) · [Function](https://cherrytree.at/misc/destiny/DestinyPatcher_Eng/Function.htm) · [DestinyScript "Assembly" language spec](https://cherrytree.at/misc/destiny/DestinyScript_Eng/Assembly.htm) |

- **Liveness:** the public lineage is **dead** — development was discontinued in 2012 with
  v2.0.1.8. The KotatsuAkira extended branch is active but unpublished
  ([Makerpendium](https://www.makerpendium.de/index.php/Destiny_Patch)). There is also a
  community "DestinyV2 32bit-Fix" patch listed in the Makerpendium category.
- **Redistribution stance:** because Destiny modifies RPG_RT, it falls under the historical
  German-scene tolerance for binary patching (unlike the Japanese scene's EULA aversion). The
  source release makes RE legitimate, but EasyRPG must not ship `Destiny.dll` or game assets.

## 2. Target engine builds

Destiny targets the **classic Don-Miguel-era RPG Maker 2000 runtimes**, specifically
**RM2k v1.05 and v1.07** (the two builds the patcher accepts). It is a **patch applied to an
existing `RPG_RT.exe`** plus a dropped-in `Destiny.dll`; it is **not** a wholesale replacement
runtime.

- Confirmation that the runtime version is read from the patched EXE, not assumed: EasyRPG's
  loader parses a `gameVersion` dword out of `RPG_RT.exe` and only accepts major `0x2000`
  (RM2k) or `0x2003` (RM2k3); anything else is a fatal error
  (`src/game_destiny.cpp:191-203`, `CheckVersionInfo`). The Emscripten fallback hardcodes
  `gameVersion = 0x20000107` — i.e. **RM2k 1.07** — as the canonical target
  (`src/game_destiny.cpp:80`).
- Although the container header carries a `0x2003` (RM2k3) code path, the published patcher is
  an RM2k tool; RM2k3 support is the latent capability of the DLL, not a shipped target
  (UNVERIFIED whether any released RM2k3 game uses Destiny).

## 3. Version lineage

Published lineage is sparse; only the terminal version and the (private) continuation are
firmly dated.

| Build | Date | Headline changes | Published |
|---|---|---|---|
| (early 1.x / 2.x dev builds) | pre-2012 | DestinyScript language, float/string/file/net/picture features | yes (details (RE-pending)) |
| **v2.0.1.8** | **2012** | Final public release; **source code released** | yes |
| KotatsuAkira extended branch | 2016– | Extended DestinyScript; new features | **no** (unreleased) ([Makerpendium](https://www.makerpendium.de/index.php/Destiny_Patch)) |

- The DLL version is itself stored in the EXE container and decoded as a `major.minor` pair
  (`Destiny::Version`, `src/game_destiny.h:143-197`): `ver_major = version >> 16`,
  `ver_minor = version & 0xFFFF`. The Emscripten default is `0x20000` → **"2.0"**
  (`src/game_destiny.cpp:78`), consistent with the v2.0.1.8 era.
- A per-build dated changelog is **(RE-pending)** — reconstruct from the 2012 source drop and
  the cherrytree.at docs if exact dates are needed.

## 4. Detection

Recognize a Destiny game **without running it**:

- **Bundled DLL:** **`Destiny.dll`** in the game directory. This is the sole signal EasyRPG
  uses. The constant is `DESTINY_DLL = "Destiny.dll"` (`src/game_destiny.h:29`).
- **Patcher-stamped `RPG_RT.exe`:** the patched EXE carries a Destiny **initialization-parameter
  block** at a fixed file offset (see §7); its presence is a strong secondary signature, but
  EasyRPG does not currently scan for it independently of the DLL.
- **No VERSIONINFO marker:** unlike Maniacs (`"Maniacs, v…"`) the Destiny patcher does not add a
  distinguishing PE version string; detection is DLL-only. There is no dedicated `exe_reader.cpp`
  branch for Destiny (verified: no Destiny references in `src/exe_reader.cpp`).
- **Data files:** Destiny does **not** change LDB/LMU/LMT/LSD chunk layout (its data lives in
  `$`-comment event commands and in `RPG_RT.exe`), so there is no data-file signature.

### EasyRPG mapping

| Aspect | Value | Anchor |
|---|---|---|
| Config flag | `Game_ConfigGame::patch_destiny` (`BoolConfigParam`, default `false`) | `src/game_config_game.h:43` |
| `[Patch]` ini key | `Destiny` — **declared but NOT read** by `LoadFromStream` (see below) | `src/game_config_game.h:43`; `src/game_config_game.cpp:206-262` |
| CLI flag | **none** — there is no `--patch-destiny` | (absent from `src/game_config_game.cpp` `LoadFromArgs`) |
| Auto-detect anchor | `Destiny.dll` present → `game_config.patch_destiny.Set(true)` | `src/player.cpp:857-859` |
| Runtime predicate | `Player::IsPatchDestiny()` | `src/player.h:537-544` |
| Post-detect hook | `Game_Destiny::Load()` runs after object creation | `src/player.cpp:884-886` |

**Precedence / override.** Auto-detection of `Destiny.dll` is inside the
`if (!game_config.patch_override)` block (`src/player.cpp:843`). Setting any patch option
explicitly (ini or CLI) sets `patch_override` and disables **all** DLL sniffing — so an
explicit, unrelated patch flag can silently suppress Destiny auto-detection. See
[easyrpg-extensions.md](easyrpg-extensions.md) for the override mechanism.

**Verified omission (bug).** Although `patch_destiny` carries the ini key `Destiny`
(`src/game_config_game.h:43`), `Game_ConfigGame::LoadFromStream` enables `patch_easyrpg`,
`patch_dynrpg`, `patch_maniac`, `patch_common_this_event`, `patch_unlock_pics`,
`patch_key_patch`, `patch_rpg2k3_commands`, `patch_anti_lag_switch`, `patch_direct_menu`,
`patch_powermode` and the `RuntimePatches` group from ini — but **never calls
`patch_destiny.FromIni(ini)`** (`src/game_config_game.cpp:219-261`). Consequently the only ways
to turn Destiny on are: (a) `Destiny.dll` auto-detect, or (b) the per-savegame runtime flag
`destiny` via `EasyRpg_SetInterpreterFlag` (see §8). A user cannot force `[Patch] Destiny=1`
from `EasyRPG.ini`. (It *is* listed by `PrintActivePatches`, `src/game_config_game.cpp:274`.)

## 5. Event commands

Destiny **adds no new numeric event-command codes.** All Destiny functionality is carried by
the **existing RM2k `Comment` / `Comment_2` commands** (lcf codes **12410** `Comment` and
**22410** `Comment_2`), distinguished only by the convention that the first character of the
comment string is `$`.

| Code | Name (EN / JP) | Since | String field | Parameters (per index) | RPG_RT quirks to replicate | EasyRPG status + anchor |
|---:|---|---|---|---|---|---|
| 12410 | Comment / 注釈 (chūshaku) | RM2k | First line of the DestinyScript block; treated as a `$`-prefixed script **iff** `string[0] == '$'` | none used by Destiny | A leading `$` switches the comment from inert text to executable DestinyScript | **STUB** — routed to `Game_Destiny::Main`, then the interpreter immediately exits. `src/game_interpreter.cpp:2093-2104` |
| 22410 | Comment_2 (注釈の続き, continuation line) | RM2k | Continuation line(s); each is appended to the script with a `'\n'` separator | none | Multi-line scripts are formed by a `Comment` followed by consecutive `Comment_2` rows | **STUB** — consumed by `MakeString` while assembling the block. `src/game_destiny.cpp:216-235` |

**Dispatch path.** `Game_Interpreter::CommandComment` first offers the comment to DynRPG
(`HandleDynRpgScript`, `@`-prefixed), then to Destiny (`HandleDestinyScript`, `$`-prefixed)
(`src/game_interpreter.cpp:2106-2114`). `HandleDestinyScript` returns "not handled" (empty
optional) unless `Player::IsPatchDestiny()` **and** `com.string[0] == '$'`
(`src/game_interpreter.cpp:2093-2104`). When both hold it calls
`Main_Data::game_destiny->Main(GetFrame())`, which assembles and logs the script but executes
nothing (see §9).

**Block assembly (`Interpreter::MakeString`, `src/game_destiny.cpp:216-235`).** Starting at the
current command, the `Comment` string is taken verbatim, then **every immediately following
`Comment_2` command** is appended with a `'\n'`, and `frame.current_command` is advanced past
the whole block. So a DestinyScript program is a contiguous run of one `Comment` + N
`Comment_2` rows authored in the editor as a single multi-line comment.

The DestinyScript language itself is **not** an event-command-level concern; its syntax and
opcodes are documented in §7/§8 and remain (RE-pending) for execution.

## 6. Modified baseline commands

Destiny does **not** alter the parameter layout, ranges, or semantics of any standard RM2k
event command at the LDB/LMU level. Its only interaction with a baseline command is the
**re-interpretation of `Comment`/`Comment_2` strings** described in §5; no other command is
extended, widened, or gated. (Section kept per template; it genuinely does not apply beyond the
comment-hijack already covered.)

Behavioral tweaks the **DestinyPatcher** applies to the engine — key assignment, default window
mode, picture count, etc. ([Function.htm](https://cherrytree.at/misc/destiny/DestinyPatcher_Eng/Function.htm))
— are EXE byte-patches, not command changes; see §8.

## 7. File-format changes

Destiny does **not** add or modify any LDB/LMU/LMT/LSD chunk. Two non-LCF surfaces matter:

### 7.1 DestinyScript carried in event data

DestinyScript source lives inside the **string field of `Comment`/`Comment_2`** commands
(§5). Because liblcf reads event-command strings raw and re-emits them losslessly with no
enum validation (`RawStruct<rpg::EventCommand>::ReadLcf`/`WriteLcf`,
`lib/liblcf/src/ldb_eventcommand.cpp:39-66`), DestinyScript **round-trips through liblcf
untouched** — it is preserved as ordinary comment text and requires no liblcf model changes.

### 7.2 Destiny initialization block embedded in `RPG_RT.exe`

The patcher writes a fixed-layout parameter block into `RPG_RT.exe`. EasyRPG reads it at
**file offset `0x00030689`** with a strict interleaving of `uint32` fields each followed by a
**1-byte separator** that is skipped (`seekg(1, cur)`). Read order
(`src/game_destiny.cpp:55-75`):

| # | Field (read order) | Type | Meaning | Notes |
|---:|---|---|---|---|
| 1 | `stringSize` | `uint32` | Capacity of the **string** container | container element count |
| 2 | `floatSize` | `uint32` | Capacity of the **float** container | |
| 3 | `dwordSize` | `uint32` | Capacity of the **dword** (int) container | |
| 4 | `extra` | `uint32` | Destiny feature flags (see DF_* below) | |
| 5 | `gameVersion` | `uint32` | RPG_RT version; major must be `0x2000`/`0x2003` | `0x20000107` ⇒ RM2k 1.07 |
| 6 | `language` | `uint32` | DLL language: `0` Deutsch, `1` English | controls decimal separator |
| 7 | `dllVersion` | `uint32` | `Destiny.dll` version (`major<<16 | minor`) | `0x20000` ⇒ 2.0 |

Each field is separated by a single byte that the reader skips; the leading field starts
exactly at `0x00030689`. These three container sizes determine the runtime arrays
`_dwords` / `_floats` / `_strings` (`src/game_destiny.cpp:127-129`). **(RE-pending):** the
offset `0x00030689` is build-specific (it is hardcoded with no version guard); whether it is
identical for RM2k 1.05 vs 1.07 and across `Destiny.dll` versions is unverified. On Emscripten,
where the EXE is not memory-mapped, hardcoded defaults are used instead
(`src/game_destiny.cpp:76-83`): `dllVersion=0x20000`, `language=ENGLISH`,
`gameVersion=0x20000107`, `extra=0x01`, all three container sizes `= 0x64` (100).

### 7.3 Feature flags (`extra` field → `DF_*`)

The `extra` dword is a bitfield (`src/game_destiny.h:92-96`):

| Flag | Bit | Meaning | EasyRPG state |
|---|---|---|---|
| `DF_TRUECOLOR` | `0b0001` | Use 32-bit "AuroraSheets" graphics | Decoded into `_trueColor = (extra & 1) << 9` (`src/game_destiny.cpp:187`); not otherwise acted on |
| `DF_EVENTSYSTEM` | `0b0010` | The Destiny event system is used | **"not yet implemented"** (`src/game_destiny.h:94`) |
| `DF_HARMONY` | `0b0100` | `Harmony.dll` exposes a DestinyInterface | **"not yet implemented"** (`src/game_destiny.h:95`) |
| `DF_PROTECT` | `0b1000` | Potentially unsafe functions (file/net) blocked | Decoded into `_protect` (`src/game_destiny.cpp:188`); enforcement not implemented |

The chosen `language` sets `_decimalComma = (language == DEUTSCH)`
(`src/game_destiny.cpp:121`) — German builds format decimals with a comma. `gameVersion` major
`0x2003` sets `_rm2k3 = true` (`src/game_destiny.cpp:202`).

### 7.4 Runtime data containers

`Destiny.dll` exposes three indexable containers parallel to RM2k's switches/variables, plus
(planned) file and socket containers:

| Container | EasyRPG type | Sized from | Status |
|---|---|---|---|
| dword (integer) | `std::vector<int> _dwords` | `dwordSize` | allocated, unused |
| float | `std::vector<double> _floats` | `floatSize` | allocated, unused |
| string | `std::vector<std::string> _strings` | `stringSize` | allocated, unused |
| file | — | — | **TODO** (`src/game_destiny.cpp:131`) |
| ClientSocket (net) | — | — | **TODO** (`src/game_destiny.cpp:132`) |

These are EasyRPG's planned mirrors of the DLL's own containers. The exact element widths and
index conventions used by DestinyScript are **(RE-pending)** against the 2012 source.

## 8. Runtime behavior changes

DestinyScript is a small C-like / "assembly"-like language
([Assembly.htm](https://cherrytree.at/misc/destiny/DestinyScript_Eng/Assembly.htm)). The
EasyRPG header pre-declares the full instruction/flag taxonomy for a future interpreter even
though none of it executes yet. Treat the following as the **language model to implement**
(all anchors `src/game_destiny.h`); concrete per-opcode semantics are **(RE-pending)** against
the published spec and 2012 source.

### 8.1 Control-flow keywords (`InterpretFlag`, `src/game_destiny.h:114-136`)

`IF_ERROR, IF_COMMAND, IF_IF, IF_ELSEIF, IF_ELSE, IF_ENDIF, IF_DO, IF_LOOP, IF_WHILE,
IF_UNTIL, IF_BREAK, IF_FOR, IF_NEXT, IF_SWITCH, IF_CASE, IF_DEFAULT, IF_ENDSWITCH,
IF_CONTINUE, IF_PAUSE, IF_EXIT`. This implies DestinyScript supports `if/elseif/else/endif`,
`do/loop`, `while`, `until`, `for/next`, `switch/case/default/endswitch`, plus
`break/continue/pause/exit`. The interpreter tracks nested-loop bookkeeping with `_breaks`,
`_continues`, `_loopOperation`, `_loopOperationLevel` (`src/game_destiny.h:309-312`).

### 8.2 Object flags (`OF_*`, `src/game_destiny.h:38-42`)

`OF_NONE 0x00`, `OF_CALL 0x01`, `OF_ARRAY 0x02`, `OF_CONSTANT 0x04` — classify a script token
as a callable, an array, and/or a constant.

### 8.3 Parameter flags (`PF_*`, `src/game_destiny.h:44-59`)

Low nibble selects the **base type**: `PF_BYTE 0x0001`, `PF_WORD 0x0002`, `PF_DWORD 0x0003`,
`PF_DOUBLE 0x0004`, `PF_BOOL 0x0005`, `PF_STRING 0x0006`. High bits are **modifiers**:
`PF_EXTRA1..4` (`0x0100/0x0200/0x0400/0x0800`), `PF_FIXED 0x1000`, `PF_OBJECT 0x2000`,
`PF_FUNCTION 0x4000`, `PF_POINTER 0x8000`. So a parameter is a typed, possibly
fixed/object/function/pointer value — the float (`PF_DOUBLE`) and string (`PF_STRING`) types
are the headline RM2k-unavailable additions.

### 8.4 Sign / unary flags (`VF_*`, `src/game_destiny.h:62-68`)

`VF_PLUS 0x0001`, `VF_MINUS 0x0002`, `VF_BINARYNOT 0x0003`, `VF_LOGICALNOT 0x0004`,
`VF_INCREMENT 0x0100`, `VF_DECREMENT 0x0200` — unary prefixes/suffixes (`+`, `-`, `~`, `!`,
`++`, `--`).

### 8.5 Operators (`OF_*` operator group, `src/game_destiny.h:70-90`)

`||`(`OF_LOGICALOR 0x01`), `&&`(`0x02`), `==`(`0x03`), `!=`(`0x04`), `>`(`0x05`), `>=`(`0x06`),
`<`(`0x07`), `<=`(`0x08`), `.`/concat(`OF_CONCAT 0x09`), `+`(`0x0A`), `-`(`0x0B`), `*`(`0x0C`),
`/`(`0x0D`), `%`(`0x0E`), `<<`(`0x0F`), `>>`(`0x10`), `|`(`0x11`), `^`(`0x12`), `&`(`0x13`),
and an assignment marker `OF_SET 0x0100`. The presence of string concat (`OF_CONCAT`) and
shift/bitwise operators confirms DestinyScript is a full expression language over the typed
parameters above.

### 8.6 File access modes (`src/game_destiny.h:98-101`)

`FILE_READ 0b0001`, `FILE_WRITE 0b0010`, `FILE_APPEND 0b0100` — the file-editing feature the
docs advertise. Net ("ClientSocket") access is referenced as a planned container but has no
flag table yet.

### 8.7 Lexer rules already implemented (skeleton only)

The EasyRPG interpreter skeleton implements only **lexing/whitespace/comment skipping**, not
evaluation:

- **Whitespace** = space, HT `0x09`, CR `0x0D`, LF `0x0A`, VT `0x0B`
  (`IsWhiteSpace`, `src/game_destiny.h:342-349`).
- **Word characters** = `_`, `0-9`, `A-Z`, `a-z` (`IsWordChar`, `src/game_destiny.h:356-362`).
- **Statement terminator** = `;` (`IsEndOfLine`, `src/game_destiny.h:289-292`).
- **Comments**: `//` line comments and `/* … */` block comments are recognized and skipped
  (`LineComment` / `BlockComment`, `src/game_destiny.cpp:315-343`). So DestinyScript uses
  C-style comments *inside* the `$`-comment event command.

### 8.8 Other runtime behavior (from the docs, not implemented)

Per the author's docs, the runtime additionally provides **picture processing** (beyond stock
RM2k), **floating-point math**, **file editing**, and **Internet communication**
([Function.htm](https://cherrytree.at/misc/destiny/DestinyPatcher_Eng/Function.htm)). The
**DestinyPatcher** tool separately rebinds keys, changes the default window mode, and raises the
picture count by byte-patching `RPG_RT.exe`. None of these effects is reproduced by EasyRPG
today; they are listed here as the implementation surface. Quantitative limits (max pictures,
container sizes beyond the EXE-declared values, socket/file constraints) are **(RE-pending)**.

## 9. EasyRPG support matrix

| Feature | Status | Implementing `file:line` | Release / notes |
|---|---|---|---|
| Detect Destiny via `Destiny.dll` | **FULL** | `src/player.cpp:857-859`; `src/game_destiny.h:29` | auto-detect; in 0.8.1 |
| Read EXE init block (sizes, version, flags) | **FULL** | `src/game_destiny.cpp:55-86` | offset `0x00030689`; Emscripten uses defaults |
| Allocate dword/float/string containers | **PARTIAL** | `src/game_destiny.cpp:126-129` | allocated but never read/written |
| Recognize `$`-prefixed comment as DestinyScript | **FULL** | `src/game_interpreter.cpp:2093-2104` | gated on `IsPatchDestiny()` |
| Assemble multi-line script block | **FULL** | `src/game_destiny.cpp:216-235` | `Comment` + consecutive `Comment_2` |
| Lex/skip whitespace + `//` and `/* */` comments | **PARTIAL** | `src/game_destiny.cpp:243-343` | lexer only |
| **Execute DestinyScript** | **STUB / MISSING** | `Interpreter::Interpret`, `src/game_destiny.cpp:263-278` | **returns `IF_EXIT` immediately**; body is commented out — no statement is ever evaluated |
| Log the script text | **FULL** | `src/game_destiny.cpp:179` | `Output::Debug("DestinyScript Code:\n{}", …)` |
| File container | **MISSING (TODO)** | `src/game_destiny.cpp:131,150` | |
| ClientSocket / net container | **MISSING (TODO)** | `src/game_destiny.cpp:132,151` | |
| `DF_EVENTSYSTEM` / `DF_HARMONY` / `DF_PROTECT` enforcement | **MISSING** | `src/game_destiny.h:94-96` | flags decoded, behavior absent |
| `--patch-destiny` CLI flag | **MISSING** | — | no CLI surface exists |
| `[Patch] Destiny` ini key | **MISSING (bug)** | `src/game_config_game.cpp:219-261` | key declared (`game_config_game.h:43`) but `FromIni` never called |
| Per-savegame runtime toggle | **FULL** | `EasyRpg_SetInterpreterFlag` name `destiny` → id 1, `src/game_interpreter.cpp:5556,5585-5591`; predicate `src/player.h:537-544` | also surfaced in debug window `src/window_interpreter.cpp:38,56` |

**Net effect.** EasyRPG's Destiny support is **detection + data-container scaffolding + a
no-op lexer**. The actual interpreter (`Interpret`) is a stub that returns `IF_EXIT` on the
first call (`src/game_destiny.cpp:263-278`), so `Game_Destiny::Main` assembles the block, logs
it once, and returns `true` without running anything (`src/game_destiny.cpp:154-183`). The
0.8.1 release blog states plainly that, "as it's a stub it has currently no useful
functionality"
([EasyRPG Player 0.8.1 "Stun"](https://blog.easyrpg.org/2025/04/easyrpg-player-0-8-1-stun/)).

**Provenance / history.** The in-tree implementation was contributed by **Dr.XGB**
(`src/game_destiny.{h,cpp}` first landed 2023-10-25 "Destiny: OOP approach"; later
"Destiny: Skip whitespaces and comments" 2024-11-10), all carrying the same stub interpreter
(git history of `src/game_destiny.cpp`). No DestinyScript opcode has been wired up since.

## 10. Test assets

No Destiny game is catalogued in this repo's `docs/games/` yet, and none is known in the corpus
at `c:\rg\easyrpg_library` (UNVERIFIED — search the corpus for any folder containing
`Destiny.dll`). To exercise the current code path you need an **RM2k 1.05/1.07 game with
`Destiny.dll` present and at least one `$`-prefixed comment**; that triggers detection,
container init from the EXE block, block assembly, and the debug log — but produces no gameplay
effect. A minimal repro project (one map, one event with a `$`-comment) would suffice to
regression-test detection and block assembly once execution is implemented.
Cross-reference: the related comment-command dispatch shared with DynRPG is documented in
[dynrpg.md](dynrpg.md); the override/auto-detect interaction is in
[easyrpg-extensions.md](easyrpg-extensions.md).

## 11. Open questions

1. **DestinyScript execution semantics** — the entire language is (RE-pending). Map the
   `OF_*`/`PF_*`/`VF_*`/`InterpretFlag` taxonomy in `src/game_destiny.h` to concrete evaluation
   rules using the 2012 source drop and
   [Assembly.htm](https://cherrytree.at/misc/destiny/DestinyScript_Eng/Assembly.htm).
2. **EXE init-block offset stability** — is `0x00030689` constant across RM2k 1.05 vs 1.07 and
   across `Destiny.dll` versions, or must EasyRPG derive it per build? It is currently
   hardcoded with no guard (`src/game_destiny.cpp:60`).
3. **Container element layout** — exact width/index semantics of the dword/float/string
   containers, and the planned file/socket container formats.
4. **Picture/feature byte-patches** — what exactly DestinyPatcher changes in `RPG_RT.exe`
   (key map, window mode, picture count) and whether any of it must be emulated for games to
   render correctly.
5. **`[Patch] Destiny` ini bug** — should `patch_destiny.FromIni(ini)` be added to
   `LoadFromStream` so users can force-enable Destiny without the DLL? (Confirmed missing,
   `src/game_config_game.cpp:219-261`.)
6. **Real-world corpus** — are there shipped, distributable Destiny games (esp. any RM2k3
   ones) to validate against? Liveness of the KotatsuAkira extended branch for forward-compat.

## 12. References

Primary sources first; archive links where rot is likely.

- **EasyRPG/Player source (this tree):** `src/game_destiny.h`, `src/game_destiny.cpp`,
  `src/game_interpreter.cpp:2093-2114` (dispatch), `:5546-5591` (runtime flag),
  `src/player.cpp:857-886`, `src/game_config_game.{h:43,cpp:219-274}`,
  `src/player.h:537-544`, `src/window_interpreter.cpp:38,56`,
  `lib/liblcf/src/ldb_eventcommand.cpp:39-66` (lossless command round-trip).
- **Bananen-Joe / Destiny docs (mirrored by Cherry):**
  [Introduction](https://cherrytree.at/misc/destiny/DestinyPatcher_Eng/Introduction.htm) ·
  [Function](https://cherrytree.at/misc/destiny/DestinyPatcher_Eng/Function.htm) ·
  [DestinyScript Assembly spec](https://cherrytree.at/misc/destiny/DestinyScript_Eng/Assembly.htm).
- **Makerpendium — Destiny Patch** (authorship, 2012 open-source, KotatsuAkira continuation):
  https://www.makerpendium.de/index.php/Destiny_Patch
- **EasyRPG issue #1227** (DestinyScript in `$`-comments; floats/pictures/net/files):
  https://github.com/EasyRPG/Player/issues/1227
- **EasyRPG Player 0.8.1 "Stun" release notes** ("stub … no useful functionality"):
  https://blog.easyrpg.org/2025/04/easyrpg-player-0-8-1-stun/
- **EasyRPG wiki — Known patches:**
  https://wiki.easyrpg.org/development/technical-details/known-patches
- **Research corpus (local, read-only):** `/home/john/research/other_patches_web.md` §4.2;
  `/home/john/research/easyrpg_patch_inventory.md` §6 and §1.1.
