<!--
Spec: Maniac Patch — file formats, chunks, ini, global save, expression bytecode.
Part of the Maniac Patch spec triple: maniac-patch.md (overview/detection/runtime),
maniac-patch-commands.md (event commands — PLANNED, not yet written; see maniac-patch.md §11),
this file (data formats). Cross-links to maniac-patch-commands.md are deliberate forward
references to that planned doc, per the convention set in maniac-patch.md.
Follows docs/forks/TEMPLATE.md. Sections that do not apply to a pure data-format spec
are kept with a one-line note rather than dropped.
-->

# Maniac Patch — file formats, chunks, ini, global save, expression bytecode (マニアクスパッチ / Maniacs)

> This is the **data-format** member of the Maniac Patch spec set. It documents every change the
> patch makes to the on-disk LCF containers (`RPG_RT.ldb`, `Save**.lsd`), the new standalone files
> it introduces (`Save.lgs` global save, `RPG_RT.rs1` encrypted asset pack), the `RPG_RT.ini` key
> surface the patched runtime actually reads, the `Font/` folder convention, and the serialized
> **expression bytecode** that backs the patch's "式" (*shiki*, expression) operands. Command-level use of
> these formats lives in the sibling spec `maniac-patch-commands.md` (**planned — not yet written**;
> see [maniac-patch.md](maniac-patch.md) §11); identity / detection / runtime behavior live in
> [maniac-patch.md](maniac-patch.md).

**Status at a glance:** EasyRPG support for these formats = **PARTIAL**. liblcf models most Maniac
chunks as real, named fields (codegen from `lib/liblcf/generator/csv/fields_easyrpg.csv`); a handful
are parsed-but-unused, a few are unmodeled. `Save.lgs` is fully implemented; `RPG_RT.rs1` is
detected only. Detection of the format family = `accord.dll` + EXE VERSIONINFO `Maniacs, vNNNNNN`
(see [maniac-patch.md](maniac-patch.md) §4). Spec confidence = **HIGH** for chunks present in liblcf
and for the v220325 (Beloved Rapture) binary; **MEDIUM** for 241028-only additions verified only from
the official changelog.

---

## 1. Identity

This section is covered in full by [maniac-patch.md](maniac-patch.md) §1. In brief: author 氷山羊
(**BingShan** / `BingShan1024`); a Steam RPG Maker 2003 **v1.12a**-only extension patch; Degica/Enterbrain
Patch EULA + "use at your own risk"; distributed as password-protected ZIPs on
[uploader.jp/xingqier](https://ux.getuploader.com/xingqier/); actively maintained (engine display
version 241028 as of 2024-10, editor 240822). Relevant to this spec: **BingShan publishes no
byte-level format spec.** The de-facto reference for the chunk layout is liblcf's reverse-engineered
chunk map ([`lib/liblcf/generator/csv/fields_easyrpg.csv`](https://github.com/EasyRPG/liblcf/blob/master/generator/csv/fields_easyrpg.csv)),
plus the two official **expression** spec files in the docs repo
([`vexpr.txt`](https://github.com/BingShan1024/steam2003_maniacs/blob/master/vexpr.txt),
[`vexpr_bytecode.txt`](https://github.com/BingShan1024/steam2003_maniacs/blob/master/vexpr_bytecode.txt)).

---

## 2. Target engine builds

Steam RPG Maker 2003 **v1.12a** (`RPG_RT.exe` product version `1.1.2.1`). Format additions are dated
by the build (YYMMDD) that introduced them; this spec calls out the introducing build per chunk where
known. The reference binary for byte-level claims is **`BelovedRapture.exe`** = build **v220325**, an
unpublished Discord-era build (see [maniac-patch.md](maniac-patch.md) §3). Note that several formats
in §7 (`RPG_RT.rs1`, System2 scene-control chunks, the `SaveData/`+`Map/` folder relocation) are
**241028-only** and are therefore *absent* from a v220325 game; they are documented here for
completeness and flagged.

---

## 3. Version lineage

Format-relevant additions only (full build lineage in [maniac-patch.md](maniac-patch.md) §3). Dates
from the official 更新履歴 ("changelog") at <https://bingshan1024.github.io/steam2003_maniacs/>.

| Build | Date | Format-level change | Published |
|---|---|---|---|
| ≤181209 | 2018 | save/load command family (3001–3003) → LSD title-block reads | yes |
| 190119 | 2019-01-19 | Show String Picture → string-picture save state | yes |
| 190217 | 2019-02-17 | Enemy `0x0F` per-enemy attack animation; CE triggers **6/7** (battle start / battle parallel) | yes |
| 190904 | 2019-09-04 | **`Save.lgs`** global save (Control Global Save 3016) | yes |
| 191020 | 2019-10-20 | Set Game Option → FatalMix LSD chunks `0x88–0x8B` | yes |
| 200126 | 2020-01-26 | expression ("式") operands for Control Variables → expression bytecode in command params | yes |
| 210519 | 2021-05-19 | **String variables** (`t[n]`): LDB `0x21` names, LSD `0x24` values; Edit Picture → `SavePicture 0x1C` | yes |
| 210809 | 2021-08-09 | partial support for EasyRPG-extended Terms IDs 203–216 | yes |
| 211010 | 2021-10-10 | separate X/Y picture scale → `SavePicture 0x0A/0x24`; `Encoding=` ini key (EN build) | yes |
| **220325** | 2022-03 | **Beloved Rapture's build** (no public changelog; interim Discord build) | no |
| 241028 | 2024-10-28 | **`RPG_RT.rs1`** asset encryption; Terms `0xA1–0xA6`; message-hook chunks `0x32`,`0x42–0x46`; `SaveData/`+`Map/` relocation; `pf=1` ini key | yes |

---

## 4. Detection

Detecting the *format family* (i.e. "this is a Maniac-patched project") is documented in
[maniac-patch.md](maniac-patch.md) §4 (EXE VERSIONINFO `Maniacs, v` + `accord.dll`). At the
file-format level there are two additional, file-only signals:

- **`RPG_RT.rs1` present** ⇒ a 241028+ Maniacs project with encrypted assets. EasyRPG keys on this
  filename directly: `src/filefinder.cpp:355` returns
  `FileFinder::ProjectType::Encrypted2k3Maniacs` when `RPG_RT.rs1` exists. (Detection only — see §7.4.)
- **`Save.lgs` present** ⇒ the game uses Control Global Save (3016). Not by itself proof of the patch
  (it is an output file), but a strong corroborating signal.

There is **no LDB/LMU header-string change**: Maniacs keeps the stock `"LcfDataBase"` / `"LcfMapUnit"`
/ `"LcfSaveData"` headers, so header sniffing does not distinguish a Maniac project (liblcf only warns
on a bad header anyway — `lib/liblcf/src/ldb_reader.cpp:74-75`). The discriminator is the *presence of
Maniac chunk IDs* inside otherwise-standard files (§7).

---

## 5. Event commands

Out of scope for this spec — see [maniac-patch-commands.md](maniac-patch-commands.md) for the full
per-code parameter layout of the 26 dispatched Maniac commands (3001–3029) and the modified baseline
commands. This document only describes the **bytes those commands read and write**. Where a command
serializes state into a chunk, the chunk is documented in §7 with a back-reference to the command code.

---

## 6. Modified baseline commands

Out of scope here (see [maniac-patch-commands.md](maniac-patch-commands.md) §6). The two baseline
*command* changes with a **data-format footprint** are documented in §7 / §9 instead: Control
Variables' **expression operand** serialization (§9, the expression bytecode), and Show/Move
Picture's extra effect/zoom serialization into `SavePicture` chunks (§7.2).

---

## 7. File-format changes

Maniacs reuses the stock LCF container format (length-prefixed `[chunk-id][length][payload]` chunks,
all integers LEB128-style variable-length per liblcf `LcfReader::ReadInt`). It adds **new chunk IDs**
inside `Database` (LDB) and the savegame structs (LSD), plus the standalone `Save.lgs` and (241028)
`RPG_RT.rs1`. No Maniacs-specific LMU/LMT chunks are documented as of 2026-06; the only map-tree change
is the optional `Map/` folder relocation (241028, §7.5).

**liblcf coverage convention** (see [README.md](README.md) "liblcf"): Maniac chunks use the **real
Maniacs chunk IDs**; EasyRPG's own invented chunks live at `0xC8+` to avoid collision. Unknown chunks
are **dropped on rewrite** (liblcf `LcfReader::Skip`, `lib/liblcf/src/reader_lcf.cpp:281-286`) — so any
Maniac chunk *not* modeled below is silently lost when liblcf re-saves the file.

### 7.1 LDB (`RPG_RT.ldb`, header `"LcfDataBase"`)

| Struct | Chunk | liblcf field (type) | Meaning | Default | Since | liblcf coverage |
|---|---|---|---|---|---|---|
| `Database` | **0x21** | `maniac_string_variables` (`Array<StringVariable>`) | Name table for string variables `t[n]`. Element struct `StringVariable` = `{0x01 name (DBString)}`. Values are **not** here (they live per-save in LSD `SaveSystem 0x24`). | empty | 210519 | **Modeled** — `fields_easyrpg.csv:157-158` |
| `Enemy` | **0x0F** | `maniac_unarmed_animation` (`Ref<Animation>`) | Per-enemy normal-attack battle animation. **Replicate-quirk:** editing the enemy with a non-Maniacs editor resets this to `1`. | 1 | 190217 | **Modeled** — `fields_easyrpg.csv:78` |
| `Terms` | **0xA1** | `maniac_item_received_a` (DBString) | Item-received message, part 1 (vanilla `item_received` is part 2) | "" | 241028 | **Modeled** — `fields_easyrpg.csv:111` |
| `Terms` | **0xA2** | `maniac_level_up_a` (DBString) | Level-up message, part 1 | "" | 241028 | **Modeled** — `:112` |
| `Terms` | **0xA3** | `maniac_level_up_b` (DBString) | Level-up message, part 3 (`level_up` is part 2) | "" | 241028 | **Modeled** — `:113` |
| `Terms` | **0xA4** | `maniac_level_up_c` (DBString) | Level-up message, part 4 | "" | 241028 | **Modeled** — `:114` |
| `Terms` | **0xA5** | `maniac_exp_received_a` (DBString) | EXP-received message, part 1 (`exp_received` is part 2) | "" | 241028 | **Modeled** — `:115` |
| `Terms` | **0xA6** | `maniac_skill_learned_a` (DBString) | Skill-learned message, part 1 (`skill_learned` is part 2) | "" | 241028 | **Modeled** — `:116` |
| `CommonEvent` | (existing `trigger` chunk, value space) | enum `Trigger`: **6** = `maniac_battle_start`, **7** = `maniac_battle_parallel` | Two new common-event trigger types beyond vanilla (3=auto, 4=parallel, 5=call). On disk this is the *value* of the existing trigger chunk, not a new chunk. | — | 190217 | **Modeled** (enum) — `enums_easyrpg.csv:2-3`; CE `Trigger_maniac_battle_start=6` / `_parallel=7` in `lib/liblcf/src/generated/lcf/rpg/commonevent.h`. Player decodes them in `GetGameInfo` (`src/game_interpreter.cpp:4253-4256`) but does **not** yet auto-run them (battle hooks are used instead). Confirmed by Ghabry: "Start is 6, Battle Parallel is 7" ([Player PR #3545](https://github.com/EasyRPG/Player/pull/3545)) |
| `State` | **0x28** | *(none)* | A **1-byte flag** Maniacs writes between vanilla `State 0x27` (`battler_animation_id`) and `0x29` (`restrict_skill`). **Not modeled in liblcf** — `fields.csv` jumps 0x27→0x29 with no 0x28 (`lib/liblcf/generator/csv/fields.csv:362-363`), so this chunk is **dropped on rewrite**. Semantics tentatively the state-#1-editability / "count only the target's actions toward turns" option family added 190904. **(RE-pending — resolve in [maniac-patch-commands.md](maniac-patch-commands.md) / br_command_semantics.md.)** | — | 190904? | **Unmodeled** — gap in `fields.csv` |
| `EventCommand` | (raw stream) | codes 3001–3029, widened params on 10220/11110/11120/11130/11610/12010/12210/12330 | The Maniac command codes are the genuine on-disk codes emitted by the Maniacs editor; they appear verbatim in LDB/LMU event data. | — | varies | **Lossless** — liblcf reads `EventCommand` raw (`code,indent,string,params`) with no enum validation (`lib/liblcf/src/ldb_eventcommand.cpp:39-67`), so unknown codes survive round-trip |

Beyond the table, the patch widens the *value range* of `Database`-stored integers from RM2k3's
±9,999,999 to full int32 (`-2147483648…2147483647`, still saturating). That is a runtime/limit change,
not a chunk change — the integer encoding is unchanged. See [maniac-patch.md](maniac-patch.md) §8.

> **241028-only LDB gap:** the System2 / BattleCommands-area settings for title/save/load-scene
> control and message-window defaults added in 241028 are **not yet mapped in liblcf**
> ([liblcf issue #493](https://github.com/EasyRPG/liblcf/issues/493)). Irrelevant to v220325 unless a
> dev build already wrote them. **(RE-pending.)**

### 7.2 LSD (`Save**.lsd`, header `"LcfSaveData"`)

Roughly 20 Maniac chunks across five savegame structs. All are LEB128-int / typed payloads inside the
standard LSD chunk framing.

#### `SaveSystem`

| Chunk | liblcf field (type) | Meaning | Default | liblcf coverage |
|---|---|---|---|---|
| **0x24** | `maniac_strings` (`Vector<DBString>`) | **String-variable values** `t[n]` (names are in LDB `0x21`). This is the per-save half of the string-variable system. | — | **Modeled** — `fields_easyrpg.csv:42`. Player stores in `Game_Strings` and persists at `src/scene_save.cpp:141`, restores at `src/player.cpp:1229` |
| **0x2D** | `maniac_message_window_width` (Int32) | Message window width (px) | 0 | **Modeled** — `:43` |
| **0x2E** | `maniac_message_window_height` (Int32) | Message window height (px) | 0 | **Modeled** — `:44` |
| **0x2F** | `maniac_message_font_name` (DBString) | Message-window font name | "" | **Modeled** — `:45` |
| **0x30** | `maniac_message_font_size` (Int32) | Message-window font size | 0 | **Modeled** — `:46` |
| **0x32** | `maniac_message_hook_flags` (`ManiacMessageHook_Flags`) | Which message events invoke the Control-Message common event. Flag bits: `user_event` (`\E` seen in text), `create_window`, `destroy_window`, `text_rendering` (`flags_easyrpg.csv:30-33`) | 0 | **Modeled** — `:47` (Control-Message cmd 3029 **not** implemented in Player) |
| **0x42** | `maniac_message_hook_common_event_id` (Int32) | Common event to call for the hook | 0 | **Modeled** — `:48` |
| **0x43** | `maniac_message_hook_callback_system_variable` (Int32) | First variable in the **system** callback range | 0 | **Modeled** — `:49` |
| **0x44** | `maniac_message_hook_callback_system_string_variable` (Int32) | **System** callback string variable | 0 | **Modeled** — `:50` |
| **0x45** | `maniac_message_hook_callback_user_variable` (Int32) | First variable in the **user** callback range | 0 | **Modeled** — `:51` |
| **0x46** | `maniac_message_hook_callback_user_string_variable` (Int32) | **User** callback string variable. (liblcf TODO note: exact system-vs-user split unconfirmed.) | 0 | **Modeled** — `:52` |
| **0x88** | `maniac_frameskip` (Int32) | "FatalMix" frame skip: `0=None, 1=1/5, 2=1/3, 3=1/2`. Written by Set Game Option (3018). | 0 | **Modeled-but-unused** — `:53`; Player has no frameskip op |
| **0x89** | `maniac_picture_limit` (Int32) | "FatalMix" picture-count limit (3018) | 0 | **Modeled-but-unused** — `:54`; Player has no picture limit (3018 op 2 is a no-op) |
| **0x8A** | `maniac_options` (`Vector<UInt8>`) | "FatalMix" misc options, layout `XX XA XB XC`: **A** = MsgSkip `OFF/RShift` (0/4); **B** = TestPlay `Keep/ON/OFF` (0/2/4); **C** = pause-on-focus-lost `Wait/Run` (0/1) | — | **Modeled-but-unused** — `:55` |
| **0x8B** | `maniac_joypad_bindings` (`Vector<UInt8>`) | Joypad→key map: `JoyLeft, JoyRight, JoyUp, JoyDown, Joy1 … Joy12`. Written by Key Input Proc EX (3014). | — | **Modeled-but-unused** — `:56`; Player KeyInputProcEx does not support joypad remap |
| **0x8E** | `maniac_message_spacing_char` (Int32) | Extra inter-character spacing in message window (editor value − 1) | 0 | **Modeled** — `:57` |
| **0x8F** | `maniac_message_spacing_line` (Int32) | Extra inter-line spacing (editor value − 1) | 0 | **Modeled** — `:58` |

> The `0x88–0x8B` quartet is the "FatalMix" group (named after an older FatalMix-era options block).
> liblcf parses them but Player does not consume them (`maniac_frameskip`/`maniac_picture_limit`/
> `maniac_options`/`maniac_joypad_bindings` are read into the model and ignored at runtime).

#### `SavePicture`

| Chunk | liblcf field (type) | Meaning | Default | liblcf coverage |
|---|---|---|---|---|
| **0x0A** | `maniac_current_magnify_height` (Double) | Current Y-axis zoom % (separate from X). Pairs with vanilla `current_magnify` (X). | 100.0 | **Modeled** — `fields_easyrpg.csv:19`; Player applies it (separate W/H scaling, build-gated ≥240423, `src/game_interpreter.cpp:2837-2843`) |
| **0x1C** | `maniac_image_data` (`Vector<UInt8>`) | **Deflate-compressed** raw image data of a picture modified by Edit Picture (3025). Lets edited pictures persist across save/load. | — | **Modeled (chunk only)** — `:20`. Player **never handles cmd 3025**, so this chunk is read into the model but the edits are not reproduced |
| **0x24** | `maniac_finish_magnify_height` (Int32) | Target Y-axis zoom % to animate toward (separate from X) | 100 | **Modeled** — `:21` |
| (Effect enum value) | `Effect::maniac_fixed_angle` = **3** | Picture effect mode 3 = fixed angle (Maniacs rotation effect). On disk this is a *value* of the existing effect chunk, not a new chunk ID. | — | **Modeled** — `enums.csv:136`; Player `sprite_picture.cpp:145`, `game_pictures.cpp:199,580` |

#### `SavePartyLocation`

| Chunk | liblcf field (type) | Meaning | Default | liblcf coverage |
|---|---|---|---|---|
| **0x8D** | `maniac_horizontal_pan_speed` (Double) | Horizontal screen-scroll speed (double precision; vanilla pan is integer) | 0 | **Modeled** — `fields_easyrpg.csv:40` ([PR #482](https://github.com/EasyRPG/liblcf/pull/482)); Player `src/game_player.cpp:811-928` |
| **0x8E** | `maniac_vertical_pan_speed` (Double) | Vertical screen-scroll speed (double precision) | 0 | **Modeled** — `:41` |

#### `SaveEventExecFrame` — interpreter exec-frame loop state

These five chunks carry the Maniacs interpreter-frame metadata (used by Get Game Info 3021 and the
Maniacs typed-loop machinery). Stored **per stack frame**.

| Chunk | liblcf field (type) | Meaning | Default | liblcf coverage |
|---|---|---|---|---|
| **0x0E** | `maniac_event_info` (`Ref<ManiacEventInfo>`) | Bitfield: `EventType << 4 \| ExecutionType`. Low nibble = exec type (`action=0, touched=1, collision=2, auto_start=3, parallel=4, called=5, battle_start=6, battle_parallel=7`); source bits `map_event=16, common_event=32, battle_event=64` (`enums_easyrpg.csv:41-51`) | 0 | **Modeled** — `fields_easyrpg.csv:6`; Player sets it in `PushInternal` (`src/game_interpreter.cpp:127-135`) |
| **0x0F** | `maniac_event_id` (Int32) | Event ID of the running event | 0 | **Modeled** — `:7` |
| **0x10** | `maniac_event_page_id` (Int32) | Page ID (when a map event) | 0 | **Modeled** — `:8` |
| **0x11** | `maniac_loop_info_size` (Int32) | Number of loop-info groups (one per active indentation level) | 0 | **Modeled** — `:9` |
| **0x12** | `maniac_loop_info` (`Vector<Int32>`) | One group `(current loop count, end loop value)` **per indentation level**, flattened. Backs Maniacs counted/conditional Loop (12210). | — | **Modeled** — `:10`; Player `src/game_interpreter.cpp:3817-3951` |

#### `SaveEventExecState` — mouse bits piggy-backed on key-input fields

Maniacs does **not** add new chunk IDs for mouse state; it overloads **bit 1** of existing key-input
result fields. liblcf documents this inline in `fields.csv` (these are *official* fields with a Maniac
note, hence in `fields.csv`, not `fields_easyrpg.csv`):

| Chunk | Field | Maniac overload | Anchor |
|---|---|---|---|
| **0x18** | `keyinput_decision` (Int32) | Mouse **Left** (bit 1) | `lib/liblcf/generator/csv/fields.csv:927` |
| **0x19** | `keyinput_cancel` (Int32) | Mouse **Right** (bit 1) | `:928` |
| **0x1C** | `keyinput_2kleft_2k3shift` (Int32) | Mouse **Middle** (bit 1) | `:931` |
| **0x23** | `keyinput_2k3down` (Int32) | Mouse **Scroll Down** (bit 1) | `:936` |
| **0x26** | `keyinput_2k3up` (Int32) | Mouse **Scroll Up** (bit 1) | `:939` |

(See [liblcf issue #425](https://github.com/EasyRPG/liblcf/issues/425) for the mouse/keyinput RFC.)

### 7.3 `Save.lgs` — Maniacs global save ("共有セーブ" / shared save)

A cross-savegame shared store written by Control Global Save (3016, added 190904). Fixed filename
**`Save.lgs`** in the save directory (since 241028 also honored inside `SaveData/`). Holds **only
switches and variables** — no party, map, or picture state. EasyRPG's implementation in
`src/maniac_patch.cpp:881-989` (`ManiacPatch::GlobalSave::Load/Save/Close`) is the de-facto byte spec
(BingShan publishes none):

```
[int  13]                      ; length-prefix of header string (LEB128 int)
"LcfGlobalSave"                ; 13-byte ASCII header (validated: must be exactly this)
; then a sequence of chunks, each: [int id][int length][payload], until EOF
[int 1][int len][switches]     ; chunk 1 = global switches.   payload = packed Switches_t
                               ;           (one byte per switch flag; len = switch count)
[int 2][int len][variables]    ; chunk 2 = global variables.  payload = packed Variables_t
                               ;           (int32 each; len = variable_count * 4 bytes)
```

Key facts (all from `src/maniac_patch.cpp`):
- The header length-prefix is `13` and the string is `"LcfGlobalSave"`; mismatch ⇒ rejected with
  Debug "This is not a valid global save." (`:910-911`).
- Chunk **1** = switches, chunk **2** = variables; any other chunk ID is skipped
  (`reader.Skip(..., "CommandManiacControlGlobalSave")`, `:934`) — i.e. the format is forward-extensible.
- On write, the writer is constructed with `lcf::EngineVersion::e2k3` framing (`:972`); switches written
  as `GetSize()` bytes, variables written as `GetSize() * sizeof(int32_t)` bytes (`:976-981`).
- "Open when missing" semantics: `Load()` marks the global save opened even if the file does not
  exist (it is created on Save) — `:889-894`.
- Storage targets: `Main_Data::game_switches_global` / `game_variables_global` (separate from the
  per-game switch/variable arrays). Flushed to disk on exit (`src/player.cpp:946`).

liblcf does **not** model `Save.lgs` (it is not an LSD struct); Player parses/writes it directly with
`lcf::LcfReader`/`LcfWriter`. **liblcf coverage = N/A (Player-side only).**

### 7.4 `RPG_RT.rs1` — encrypted asset pack (241028+)

A proprietary-format ("独自形式の暗号化" — *dokuji keishiki no angōka*, "proprietary-format encryption")
encrypted container that can pack/encrypt *any* project files, generated by a dedicated 241028 editor
plugin. Engine ≥241028 reads it; encrypted assets take priority over loose files; all assets are
decrypted into memory at load; BingShan explicitly disclaims it as weak crypto
([official site, 素材暗号化 *sozai angōka* / "asset encryption"](https://bingshan1024.github.io/steam2003_maniacs/)).

**Not relevant to v220325** (Beloved Rapture) — the 220325 binary has no `.rs1` string and never reads
one (RE: `/home/john/re/br_dispatch_findings.md` string scan finds no `.rs1`). EasyRPG **detects** the
file (`src/filefinder.cpp:355` → `ProjectType::Encrypted2k3Maniacs`) but does **not decrypt** it. The
encryption algorithm is undocumented publicly. **(RE-pending — only matters for 241028+ games.)**

### 7.5 Folder conventions

- **`Font/`** — project-local font directory. The runtime loads every `Font/*.*` file at startup via
  Win32 **`AddFontResourceExA`** / `AddFontMemResourceEx` (verified in the v220325 binary: string
  `/Font/`, `/Font/*.*` and the `AddFontResourceExA` import — `/home/john/re/br_dispatch_findings.md`).
  TTF since the patch's inception, **OTF since 190904**. Fonts become usable in string pictures and
  message windows. The embedded `FONTRES "RPG2000"`/`"RPG2000G"` resources remain as fallbacks.
  EasyRPG mirrors this with its own `Font/` lookup (`src/filefinder.cpp:452,513-514`,
  `src/player.cpp:1125`) and `Font/Font`/`Font/Font2` custom gothic/mincho fonts (`scene_logo.cpp:259-260`).
- **`Text/`** — output/input directory for Control Strings (3020) file IO; subfolders auto-created
  since 210809; the runtime warns "Outputting multiple files in a short time…" on rapid writes
  (string @ `0x0054d938` in v220325). The `Encoding=` ini key (§8) affects this IO.
- **`Picture/`** — output directory for Output Picture / WritePicture (3026).
- **`SaveData/`** and **`Map/`** (241028 only) — optional relocation: `Save.lgs` + `Save**.lsd` may
  live under `SaveData/` (preferred for new saves if present); `RPG_RT.lmt` + `Map****.lmu` may live
  under `Map/`. Absent in v220325.

### 7.6 What is *not* a Maniacs format

For disambiguation (these appear in Beloved Rapture's folder and are **not** Maniacs-specific):
`.r3proj` = stock Steam RM2k3 project marker; `RPG_RT.edb` = EasyRPG `lcf2xml` XML dump of the LDB (a
toolchain artifact, inert at runtime unless named `EASY_RT.edb`); `RPG_RT2.lmt` = no public
references, presumed dev-side backup. See [../games/beloved-rapture.md](../games/beloved-rapture.md)
and `/home/john/research/maniac_patch_web.md` §8.4 for the full disposition.

---

## 8. Runtime behavior changes — the `RPG_RT.ini` key surface

The patched runtime reads `RPG_RT.ini`, section **`[RPG_RT]`** only, through a single reader function
(`FUN_004a3820`, called from startup `FUN_004a4630` in `BelovedRapture.exe` v220325). Per the binary
RE (`/home/john/re/br_dispatch_findings.md`), this is the **complete** Win32 profile surface — only
`GetPrivateProfileStringA` / `GetPrivateProfileIntA` are imported, all call sites are inside that one
function, and there is no custom `.ini` parser:

| Key | Win32 API | Default | Meaning |
|---|---|---|---|
| `GameTitle` | `GetPrivateProfileStringA` | `"Untitled"` | Window title |
| `Winw` | `GetPrivateProfileIntA` | `0x140` (320) | Window width, clamped 64..1920 (→ global `0x0055b214`). Custom resolution, since 210519. |
| `Winh` | `GetPrivateProfileIntA` | `0xF0` (240) | Window height, clamped 32..1440 (→ global `0x0055b1ac`) |
| `Encoding` | `GetPrivateProfileIntA` | `0` | Code page for text-file IO (EN build, since 211010; "currently only relevant for text file IO") |
| `RuntimePackageKey` | `GetPrivateProfileStringA` | `"KADOKAWA\rpg2003"` | Registry subkey under `HKCU\Software\` for RTP lookup (replaces FullPackageFlag's role) |

**Critical RE finding (replicate this):** the v220325 runtime reads **only** the five keys above. In
particular it does **NOT read `FullPackageFlag`**, even though some games (and stock RM2k3) write it
into `RPG_RT.ini` — the patch replaced the FullPackageFlag mechanism with the
`RuntimePackageKey`/`RuntimePackagePath` registry path. It also does **not** read `Fps` or other stock
keys in this build. The `pf=1` key (im/pf selection) is **241028-only** and absent in v220325 (the
220325 binary is an `im` build; im/pf was still split into two executables at that time — see
[maniac-patch.md](maniac-patch.md) §2.2).

For contrast, EasyRPG Player reads `RPG_RT.ini` for `GameTitle`, `FullPackageFlag`, `WinW`/`WinH` only
(`src/player.cpp:746-762`) and **does** honor `FullPackageFlag`; patch selection never consults
`RPG_RT.ini` (it uses `EasyRPG.ini [Patch]` / `--patch-*`; see
[easyrpg-extensions.md](easyrpg-extensions.md) and [README.md](README.md)). So a faithful Maniacs
emulation should ignore `FullPackageFlag` when the Maniac patch is active, matching the original.

**Registry surface** (v220325, `HKCU\Software\KADOKAWA\rpg2003` or `Software\` + `RuntimePackageKey`):
`VideoSettings` (read+written back), `DebugSettings`, `RuntimePackagePath`. **Command-line switches**
(case-insensitive): `TestPlay`, `BattleTest`, `HideTitle`, `FullScreen`. (RE: `br_dispatch_findings.md`.)

Other runtime behavior (variable range widening, picture limits, text escape codes, timing) is not a
file format and is documented in [maniac-patch.md](maniac-patch.md) §8.

---

## 9. Expression bytecode ("式" / vexpr)

Maniacs' expression operands (Control Variables target/operand "Expression" mode, Conditional Branch
"Expression", Call Command arg evaluation, and the standalone expression-statement command 3022) embed
a **serialized expression tree** directly in the event command's parameter array. Two official spec
files in the docs repo are the authoritative source:
[`vexpr.txt`](https://github.com/BingShan1024/steam2003_maniacs/blob/master/vexpr.txt) (text-level
semantics) and
[`vexpr_bytecode.txt`](https://github.com/BingShan1024/steam2003_maniacs/blob/master/vexpr_bytecode.txt)
(the serialization). EasyRPG implements the decoder in `src/maniac_patch.cpp`
(`ManiacPatch::ParseExpression` / `ParseExpressions`).

### 9.1 Wire encoding

The bytecode is a **byte stream packed 4 bytes per `int32` parameter**, little-endian (EasyRPG unpacks
it at `src/maniac_patch.cpp:566-577`). An expression is a single **node tree**; each node is `[1-byte
opcode][opcode-specific args]`, decoded recursively.

**Placement inside a Control Variables (10220) command** (from `vexpr_bytecode.txt`):

| Param index | Value / meaning |
|---|---|
| `arg0` | `4` ⇒ "target is expression" |
| `arg1` | start index of the target expression |
| `arg2` | *(not described in `vexpr_bytecode.txt`; the vanilla 10220 "operand-type" slot)* (RE-pending) |
| `arg3` | operation |
| `arg4` | `21` ⇒ "operand is expression" |
| `arg5` | operand-expression length |
| `arg6 … argX` | operand expression body |
| (then) | target-expression length, then target-expression body |

### 9.2 Node opcodes

IDs from `vexpr_bytecode.txt`, cross-checked against EasyRPG's `enum class Op`
(`src/maniac_patch.cpp:51-100`):

| ID | Op | Meaning | EasyRPG support |
|---:|---|---|---|
| 0 | Null | null / terminator | yes |
| 1/2/3 | U8/U16/S32 | decimal int literal (1/2/4 bytes) | yes |
| 4/5/6 | UX8/UX16/SX32 | hex int literal (1/2/4 bytes) | yes |
| 8 | Var | `v[n]` (n = inline literal) | yes |
| 9 | Switch | `s[n]` | yes |
| 13 | VarIndirect | `v[v[n]]` | yes |
| 14 | SwitchIndirect | `s[v[n]]` | yes |
| 19 | Array | pseudo-array `[a,b,c]`: 1-byte count, `0x80` flag ⇒ 4-byte count follows | **unsupported** — warning "Expression contains unsupported operation" (`:535`) |
| 24 | Negate | unary `-` | yes |
| 25 | Not | unary `!` | yes |
| 26 | Flip | unary `~` (bit-not) | yes |
| 34–44 | AssignInplace … BitShiftRightInplace | in-place assignment ops `= += -= *= /= %= \|= &= ^= <<= >>=` | yes — **but refused when EasyRPG extensions are simultaneously active** (`:150-154`) |
| 48–57 | Add … BitShiftRight | binary `+ - * / % \| & ^ << >>` | yes |
| 58–63 | Equal, GreaterEqual, LessEqual, Greater, Less, NotEqual | comparisons in **ID order** `58 ==`, `59 >=`, `60 <=`, `61 >`, `62 <`, `63 !=` (note: the on-disk order is **not** the UI/text order — it interleaves `>=`/`<=` ahead of `>`/`<`; from `enum class Op` `src/maniac_patch.cpp:88-93`) | yes |
| 64 | Or | `\|\|` (short-circuit) | yes |
| 65 | And | `&&` (short-circuit) | yes |
| 66 | Range | `a..b` | **unsupported** (array family) — `:535` |
| 67 | Subscript | index `[ ]` | **unsupported** (array family) — `:535` |
| 72 | Ternary | `?:` | yes |
| 78 | Function | builtin call: `[1-byte fn id][1-byte argc (0x80 ⇒ 4-byte argc)][arg nodes]` | yes — **4-byte argc form unsupported** (`:399-403`) |

### 9.3 Builtin functions (opcode 78)

Function IDs from `vexpr_bytecode.txt`, cross-checked against EasyRPG `enum class Fn`
(`src/maniac_patch.cpp:102-122`). All delegate to `ControlVariables::*`
(`src/game_interpreter_control_variables.cpp`).

| ID | Fn | Notes |
|---:|---|---|
| 0 | Rand (rnd) | random a..b |
| 1 | Item | |
| 2 | Event | |
| 3 | Actor | |
| 4 | Party (member) | |
| 5 | Enemy | |
| 6 | Misc | **`misc(type)` type values do NOT match the UI operand order** (explicit warning in `vexpr.txt`); the type-value table is **(RE-pending)** |
| 7 | Pow | |
| 8 | Sqrt | `sqrt(a,b)` = √a × b |
| 9 | Sin | `sin(a,b,c)` = sin(a/b°) × c |
| 10 | Cos | |
| 11 | Atan2 | |
| 12 | Min | |
| 13 | Max | |
| 14 | Abs | |
| 15 | Clamp | |
| 16 | Muldiv | int64 intermediate |
| 17 | Divmul | double intermediate |
| 18 | Between | |

### 9.4 Text-level semantics (from `vexpr.txt`)

For completeness, the source grammar the bytecode encodes: decimal / `0x` hex literals with `_`
separators; **int32 saturating** arithmetic ("2003標準と同じく飽和演算" — same saturation as stock 2003);
`s[n]`, `v[n]`, assignment ranges `a..b` (with zero-padding rules); temp arrays `[a,b,c]` with index
access; unary `- ! ~`; the full binary/assignment/comparison/logical operator set listed above plus
range `..` and ternary `?:`; the builtins above. The **(RE-pending)** items are `misc(type)`'s
type→meaning table and the exact array zero-padding rules (EasyRPG does not implement the array ops).

### 9.5 The standalone expression command (3022)

The v220325 binary dispatches a command **3022** (not in liblcf) whose handler (`FUN_00448a50` →
`FUN_00445140`, the recursive expression VM) executes inline expression bytecode as a *statement*
(`/home/john/re/br_dispatch_findings.md`). It uses the same VM that backs every other Maniacs
expression operand. liblcf has no enum entry for 3022 and Player does not dispatch it; it is harmless
(survives load as a raw command, silently skipped). **(See [maniac-patch-commands.md](maniac-patch-commands.md)
for command-level detail.)**

---

## 10. EasyRPG support matrix (formats)

| Format / chunk | Status | liblcf / Player anchor | Notes |
|---|---|---|---|
| LDB `0x21` string-var names | **FULL** (model) | `fields_easyrpg.csv:157-158` | round-trips |
| LSD `SaveSystem 0x24` string-var values | **FULL** | `fields_easyrpg.csv:42`; `scene_save.cpp:141` | |
| Enemy `0x0F` attack animation | **FULL** | `fields_easyrpg.csv:78` | |
| Terms `0xA1–0xA6` | **FULL** (model) | `fields_easyrpg.csv:111-116`; honored `game_message_terms.cpp:33-80,630-664` (level-up at 33-80, exp/item at 630-664) | |
| CE triggers 6/7 | **PARTIAL** | `commonevent.h`; decoded but not auto-run (`game_interpreter.cpp:4253`) | [PR #3545](https://github.com/EasyRPG/Player/pull/3545) unmerged |
| State `0x28` 1-byte flag | **MISSING** | gap in `fields.csv:362-363` | dropped on rewrite; **(RE-pending)** |
| LSD message geometry/font/spacing `0x2D–0x30,0x8E,0x8F` | **FULL** (model); message-box size/font **TODO** at runtime | `fields_easyrpg.csv:43-46,57-58`; `game_interpreter.cpp:975` | |
| LSD message hooks `0x32,0x42–0x46` | **MODEL-ONLY** | `fields_easyrpg.csv:47-52` | cmd 3029 not implemented in Player |
| LSD FatalMix `0x88–0x8B` | **MODEL-ONLY** (parsed, unused) | `fields_easyrpg.csv:53-56` | |
| SavePicture `0x0A/0x24` Y-zoom | **FULL** | `fields_easyrpg.csv:19,21` | build-gated ≥240423 |
| SavePicture `0x1C` edited image | **MODEL-ONLY** | `fields_easyrpg.csv:20` | cmd 3025 not implemented |
| SavePicture Effect=3 fixed angle | **FULL** | `enums.csv:136` | |
| SavePartyLocation pan `0x8D/0x8E` | **FULL** | `fields_easyrpg.csv:40-41`; `game_player.cpp:811-928` | |
| SaveEventExecFrame `0x0E–0x12` | **FULL** | `fields_easyrpg.csv:6-10` | |
| SaveEventExecState mouse bits | **FULL** | `fields.csv:927-939` | |
| `Save.lgs` global save | **FULL** | `maniac_patch.cpp:881-989` | Player-side; liblcf N/A |
| `RPG_RT.rs1` | **DETECT-ONLY** | `filefinder.cpp:355` | no decryption; 241028+ |
| `RPG_RT.ini` keys | **PARTIAL** | `player.cpp:746-762` | EasyRPG honors `FullPackageFlag` (Maniacs does not) |
| Expression bytecode | **PARTIAL** | `maniac_patch.cpp:51-122,566-606` | array ops (19/66/67) + 4-byte argc unsupported |

---

## 11. Test assets

- **Beloved Rapture** (RM2k3 v1.12a + Maniacs **220325**, `en, im`) — exercises string variables
  (LDB `0x21` / LSD `0x24`), the `Font/` folder, custom `Winw`/`Winh`, and (per its event usage)
  expression operands. The primary driver for this spec. See
  [../games/beloved-rapture.md](../games/beloved-rapture.md).
- Any Maniacs game using **Control Global Save** stresses `Save.lgs` (§7.3).
- A **241028** Maniacs project is needed to exercise `RPG_RT.rs1`, Terms `0xA1–0xA6`, message-hook
  chunks, and `SaveData/`+`Map/` relocation — not yet acquired (corpus task #7). **(test-asset gap.)**
- General corpus at `c:\rg\easyrpg_library`.

---

## 12. Open questions

1. **State `0x28`** — the unmodeled 1-byte State flag: confirm its meaning (likely the 190904
   state-#1 / "count only the target's actions toward turns" option) and add it to liblcf.
   **(RE-pending — br_command_semantics.md / [maniac-patch-commands.md](maniac-patch-commands.md).)**
2. **`misc(type)` type-value table** for the expression VM — explicitly *not* matching UI operand
   order; needs RE of `FUN_00445140` or the patch-bundle docs.
3. **Expression array semantics** — opcodes 19/66/67 (pseudo-array, range, subscript) and the
   zero-padding rules for range assignment; EasyRPG does not implement them.
4. **`RPG_RT.rs1`** encryption algorithm (241028+) — undocumented; EasyRPG only detects the file.
5. **241028 System2 / scene-control LDB chunks** — unmapped in liblcf
   ([issue #493](https://github.com/EasyRPG/liblcf/issues/493)).
6. **`Save.lgs` chunk extensibility** — whether any build writes chunk IDs other than 1/2 (Player
   skips them); Maniacs may add party-scoped global data in future builds.
7. **Show String Picture parameter layout** — official docs defer to a text file inside the patch
   ZIP ("詳細はパッチ付属のテキストを参照"); extract from `patch_maniacs_211010` and cross-check Player's
   `SaveEasyRpgWindow`/`SaveEasyRpgText` persistence.

---

## 13. References

Primary:
- liblcf chunk map (de-facto byte spec): [`lib/liblcf/generator/csv/fields_easyrpg.csv`](https://github.com/EasyRPG/liblcf/blob/master/generator/csv/fields_easyrpg.csv),
  `enums_easyrpg.csv`, `flags_easyrpg.csv`, `structs_easyrpg.csv`; generated headers under
  `lib/liblcf/src/generated/lcf/{ldb,lsd}/chunks.h`.
- Expression spec: [`vexpr.txt`](https://github.com/BingShan1024/steam2003_maniacs/blob/master/vexpr.txt),
  [`vexpr_bytecode.txt`](https://github.com/BingShan1024/steam2003_maniacs/blob/master/vexpr_bytecode.txt)
  (BingShan docs repo).
- Official site (changelog / asset encryption / ini): <https://bingshan1024.github.io/steam2003_maniacs/>
  (dead-page mirror via [web.archive.org 2022-11-16 snapshot](https://web.archive.org/web/20221116205904/https://bingshan1024.github.io/steam2003_maniacs/)).
- EasyRPG Player implementation: `src/maniac_patch.{h,cpp}` (Global Save, expression VM),
  `src/filefinder.cpp:355` (`.rs1` detection), `src/player.cpp:746-762` (`RPG_RT.ini`).
- Binary RE (v220325): `/home/john/re/br_dispatch_findings.md` (ini key surface, `Save.lgs`/`Font/`/
  `Text/` strings, expression VM `FUN_00445140`, command 3022).

Research corpus: `/home/john/research/maniac_patch_web.md`,
`/home/john/research/easyrpg_patch_inventory.md`.

liblcf PRs adding these chunks: [#417](https://github.com/EasyRPG/liblcf/pull/417),
[#434](https://github.com/EasyRPG/liblcf/pull/434), [#445](https://github.com/EasyRPG/liblcf/pull/445),
[#473](https://github.com/EasyRPG/liblcf/pull/473), [#482](https://github.com/EasyRPG/liblcf/pull/482),
[#499](https://github.com/EasyRPG/liblcf/pull/499); open gaps
[#493](https://github.com/EasyRPG/liblcf/issues/493), [#425](https://github.com/EasyRPG/liblcf/issues/425).

Cross-links: [maniac-patch.md](maniac-patch.md) · [maniac-patch-commands.md](maniac-patch-commands.md) ·
[easyrpg-extensions.md](easyrpg-extensions.md) · [../games/beloved-rapture.md](../games/beloved-rapture.md) ·
[README.md](README.md).
