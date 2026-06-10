<!--
Spec: Maniac Patch — event-command reference (the "§5 Event commands" deep-dive).
Part of the Maniac Patch spec triple: maniac-patch.md (overview/detection/runtime),
maniac-patch-fileformats.md (chunks/ini/global-save/expression bytecode), this file
(per-command parameter layouts, sub-op tables, EasyRPG status).
Follows the spirit of docs/forks/TEMPLATE.md §5/§6, organized as a command reference.
Every nontrivial claim carries a source; unverified claims are marked (UNVERIFIED),
(RE-pending) or (hypothesis), per README.md conventions.
-->

# Maniac Patch — event-command reference (マニアクスパッチ 追加コマンド / Maniacs)

> This is the **event-command** member of the Maniac Patch spec set — the per-index parameter
> reference for every Maniacs command code **3001–3033**, the three undocumented codes
> **3022/3023/3024**, and the **modified baseline commands**. It merges three evidence classes
> per command: the **official Japanese documentation** (translated, JP kept), the **binary
> reverse engineering of build 220325** (`BelovedRapture.exe`, asm-verified), and the **EasyRPG
> Player implementation** in this tree (with `file:line` anchors). Identity, lineage, detection
> and engine-wide runtime behavior live in [maniac-patch.md](maniac-patch.md); chunk layouts,
> `Save.lgs`, `RPG_RT.ini` and the **expression bytecode** live in
> [maniac-patch-fileformats.md](maniac-patch-fileformats.md) — cross-linked, not duplicated here.

**Status at a glance:** EasyRPG support = **PARTIAL (deep)** — see the [rollup](#8-easyrpg-support-rollup):
10 FULL / 9 PARTIAL / 2 STUB / 1 WONTFIX / 10 MISSING across the 33 code points. Spec
confidence = **HIGH** for 3001–3022 (official docs + RE cross-checked; 3022 itself is RE-only,
asm-verified), **MEDIUM** for
3025–3029 (official docs, RE dispatch-only), **MEDIUM-LOW** for 3030–3033 (241028 docs only,
no RE). The former **3018 Set Game Option op-map conflict** was **resolved 2026-06-09** by
consumption-site adjudication against both the 220325 and 211010 binaries (§4.18) — the official
docs / PR #3487 reading is correct.

---

## 1. Sources, hierarchy, build coverage

When sources disagree, this document shows **all** readings and marks the conflict; it never
silently picks one. Precedence for *behavioral* claims about a given build: **asm-verified RE
of that build** > **official docs of the nearest build** > **EasyRPG implementation** >
community reports. Precedence for *naming*: the **liblcf enum**
(`lib/liblcf/src/generated/lcf/rpg/eventcommand.h:170-196`) is used for `Maniac_*` identifiers;
official EN/JP names are given alongside.

| Source | What it covers | Cited as |
|---|---|---|
| Official Maniacs docs, build **211010** (JP) — `command_new.txt` (per-command param layouts incl. コード/文字列引数/数値引数 blocks), `command_modified.txt`, `command_code.txt`, `cmdl_en.txt`, `vexpr.txt` | every published command through 211010, at per-index level | (Maniacs 211010 `command_new.txt`) — corpus `c:\rg\easyrpg_library\maniacs_ref_211010\docs\` |
| Official Maniacs docs, build **241029** — `command_list.txt` (full code index incl. 3029–3033), `command_code.txt` (per-code JP names incl. 3030/3031 split), `battle_command_list.txt`, `update.txt` (full dated changelog 2018-08-09 → 2024-10-28; the 241029 build itself is a JP-database-only hotfix with no changelog entry), `vexpr.txt` | command *index* + since-build assignment for every command/option | (Maniacs 241029 `update.txt`, entry YYYY/MM/DD) — corpus `c:\rg\easyrpg_library\maniacs_ref_241029\docs\` |
| **Binary RE of build 220325** (`BelovedRapture.exe`, PE32, "Maniacs, v220325, en, im") — dispatcher census (147 dispatched IDs) + parameter-level semantics of the priority commands | the asm-verified ground truth for the 220325 surface | (RE: BelovedRapture.exe `FUN_xxxxxxxx`; dispatch/semantics analysis 2026-06-09) |
| **liblcf** generated enum | authoritative `Maniac_*` names + code values | `lib/liblcf/src/generated/lcf/rpg/eventcommand.h:170-196` |
| **EasyRPG Player** (this tree, branch RISKY) | implementation status + anchors | `src/game_interpreter.cpp`, `src/game_interpreter_battle.cpp`, `src/maniac_patch.{h,cpp}`, `src/game_strings.*` |
| **Beloved Rapture usage census** (liblcf scan of the full game: 545 maps, 400 common events, 533,201 commands) | the "BR uses" column | corpus `c:\rg\easyrpg_library\beloved_rapture\command_usage.md` |
| Web dossier (official-site extraction, PR/issue links) | since-versions cross-check, EasyRPG PR history | research corpus `maniac_patch_web.md` (2026-06-09) |

**Build coverage statement.** Three builds anchor this spec:
- **211010** — last *published* build with full per-command docs (`command_new.txt` carries
  numbered コード/数値引数 blocks for 3001–3019 only; the renewal commands 3020+ are documented
  as TPC mnemonics without index numbers).
- **220325** — Beloved Rapture's *unpublished* dev build; its command surface is **RE-pinned**:
  dispatcher `FUN_004500d0` dispatches **3001–3022 and 3024–3029**; **3023, 3030, 3031, 3032 are
  NOT dispatched** (RE: BelovedRapture.exe `FUN_004500d0`, range-split + five two-level jump
  tables; dispatch analysis 2026-06-09). See [maniac-patch.md §3.4/§9](maniac-patch.md).
- **241028/241029** — newest published build; `command_list.txt`/`command_code.txt` index
  3029–3033 (incl. the **3030 = Script line 1 / 3031 = Script lines 2+** split), and `update.txt`
  is the dated changelog used for every "Since" cell below (Maniacs 241029 `command_code.txt`,
  `update.txt`).

**Legend** (used in all tables):
- **Since** — first *published* build (YYMMDD) per the official changelog; dev builds (incl.
  220325) may predate it. `≤181209` = present in the earliest documented builds (first test
  release 180809; the 181209 entry is the first itemized changelog).
- **220325** — ✓ dispatched / ✗ not dispatched by `FUN_004500d0` (RE).
- **BR uses** — instance count in Beloved Rapture (LDB + 545 LMUs; corpus census).
- **EasyRPG status** — **FULL** (behavior matches), **PARTIAL** (subset / known divergence),
  **STUB** (recognized, warns, does nothing), **MISSING** (not dispatched at all),
  **WONTFIX** (deliberately not implemented), **N/A** (not a command). Status is judged
  **against the 220325 surface**; 241028-only options are noted but not held against a status.
- **Indirection nibble** — the recurring per-value mode encoding: `0` = literal (定数),
  `1` = `v[x]` (変数), `2` = `v[v[x]]` (変数番変数). Packed 4 bits per value into a "mode word"
  parameter. EasyRPG decodes it with `ValueOrVariableBitfield`
  (`src/game_interpreter_shared.cpp:129-150`, nibble extract `(mode >> shift*4) & 0xF`); the
  220325 binary uses the identical nibble scheme
  (RE: parameter accessor pattern, semantics analysis 2026-06-09).
- **String-argument prefix** — Maniacs string fields pack multiple sub-strings, each preceded
  by a 1-byte type tag: `0x01` literal, `0x02` string-variable reference `t[x]`,
  `0x03` indirect `t[v[x]]` (RE: `FUN_00443b50`; the 211010 doc text still says "現在は0x01のみ
  有効" — "currently only 0x01 is valid" — which is stale: 211010 itself added string-variable
  specification per the changelog).

**In-memory `EventCommand` layout** every 220325 handler reads (RE: verified from
ShowStringPicture / SetGameOption / ControlSwitches handlers; semantics analysis 2026-06-09):
`+0x00` code, `+0x04` indent, `+0x08` string ptr, `+0x0c` parameter count, `+0x10` parameter
array ptr; `param[i] = *(int*)(array + i*4)`. This matches liblcf's
`{code, indent, string, parameters}` model exactly, so the per-index tables below apply
unchanged to the on-disk LDB/LMU command stream.

---

## 2. Master table — all Maniacs command codes 3001–3033

| Code | Name (EN / JP) | Since | 220325 | BR uses | EasyRPG status + anchor | § |
|---:|---|---|:--:|---:|---|---|
| 3001 | Get Save Info / セーブ情報の取得 | ≤181209 | ✓ | 7 | **PARTIAL** — `src/game_interpreter.cpp:4403` (renewal hero-name read missing) | [§4.1](#41-3001-get-save-info--セーブ情報の取得) |
| 3002 | Save / セーブの実行 | ≤181209 | ✓ | 2 | **FULL** — `:4483` | [§4.2](#42-3002-save--セーブの実行) |
| 3003 | Load / ロードの実行 | ≤181209 | ✓ | 7 | **PARTIAL** — `:4503` (no fade emulation; pre-check option ignored) | [§4.3](#43-3003-load--ロードの実行) |
| 3004 | End Load Process / ロード処理の終了 | ≤181209 | ✓ | 0 | **FULL** (no-op) — `:4534` | [§4.4](#44-3004-end-load-process--ロード処理の終了) |
| 3005 | Get Mouse Position / マウス座標の取得 | ≤181209 | ✓ | 0 | **FULL** (platform-gated) — `:4539` | [§4.5](#45-3005--3006-mouse-position--マウス座標の取得設定) |
| 3006 | Set Mouse Position / マウス座標の設定 | ≤181209 | ✓ | 0 | **WONTFIX** (warn stub) — `:4564` | [§4.5](#45-3005--3006-mouse-position--マウス座標の取得設定) |
| 3007 | Show String Picture / 文字列ピクチャの表示 | 190119 | ✓ | 2273 | **FULL** (RE FULL-match) — `:4573` | [§4.7](#47-3007-show-string-picture--文字列ピクチャの表示) |
| 3008 | Get Picture Info / ピクチャ情報の取得 | 190217 | ✓ | 1 | **PARTIAL** — `:4736` (op 3 missing) | [§4.8](#48-3008-get-picture-info--ピクチャ情報の取得) |
| 3009 | Control Battle / 戦闘処理の制御 | 190217 | ✓ | 4 | **FULL** — `src/game_interpreter_battle.cpp:706` | [§4.9](#49-3009-control-battle--戦闘処理の制御) |
| 3010 | Control ATB Gauge / ATBゲージの操作 | 190217 | ✓ | 110 | **FULL** (RE FULL-match) — `battle.cpp:723` | [§4.10](#410-3010-control-atb-gauge--atbゲージの操作) |
| 3011 | Change Battle Command EX / 戦闘コマンドの変更EX | 190217 | ✓ | 238 | **FULL** — `battle.cpp:810` | [§4.11](#411-3011-change-battle-command-ex--戦闘コマンドの変更ex) |
| 3012 | Get Battle Info / 戦闘情報の取得 | 190526 | ✓ | 274 | **PARTIAL** — `battle.cpp:846` (fields 1/2 diverge) | [§4.12](#412-3012-get-battle-info--戦闘情報の取得) |
| 3013 | Control Var Array / 変数配列の操作 | 190526 | ✓ | 0 | **PARTIAL** — `src/game_interpreter.cpp:4803` (210530/241028 ops missing) | [§4.13](#413-3013-control-var-array--変数配列の操作) |
| 3014 | Key Input Proc EX / キー入力の処理EX | 190904 | ✓ | 0 | **PARTIAL** — `:4896` (joypad ops 3–5 unsupported) | [§4.14](#414-3014-key-input-proc-ex--キー入力の処理ex) |
| 3015 | Rewrite Map / マップの書き換え | 190904 | ✓ | 0 | **PARTIAL** — `:4934` (range-ID mode unimplemented) | [§4.15](#415-3015-rewrite-map--マップの書き換え) |
| 3016 | Control Global Save / 共有セーブの操作 | 190904 | ✓ | 0 | **PARTIAL** — `:4972` (renewal string-var data type missing) | [§4.16](#416-3016-control-global-save--共有セーブの操作) |
| 3017 | Change Picture ID / ピクチャのID変更 | 191020 | ✓ | 0 | **STUB** — `:5030` (warns; upstream PR [#3500](https://github.com/EasyRPG/Player/pull/3500) open) | [§4.17](#417-3017-change-picture-id--ピクチャのid変更) |
| 3018 | Set Game Option / ゲームのオプション設定 | 191020 | ✓ | 16 | **STUB** — `:5039` (only op 2 as no-op); op map RESOLVED, PR #3487 adoptable | [§4.18](#418-3018-set-game-option--ゲームのオプション設定---resolved-2026-06-09) |
| 3019 | Call Command / コマンドの呼び出し | 191103 | ✓ | 0 | **FULL** (impl. difference: pushes a frame) — `:5469` | [§4.19](#419-3019-call-command--コマンドの呼び出し) |
| 3020 | Control Strings / 文字列変数の操作 | 210519 | ✓ | 1518 | **FULL** (RE FULL-match) — `:5057` | [§4.20](#420-3020-control-strings--文字列変数の操作) |
| 3021 | Get Game Info / ゲーム情報の取得 | 210519 | ✓ | 0 | **PARTIAL** — `:4204` (op 3 pixel-info missing) | [§4.21](#421-3021-get-game-info--ゲーム情報の取得) |
| **3022** | *(expression statement — undocumented)* | dev ≤220325 | ✓ | 0 | **MISSING** as a command (the expression VM itself is FULL) | [§5.1](#51-3022--expression-statement) |
| **3023** | *(absent)* | — | ✗ | 0 | **N/A** — not dispatched, not in liblcf, not in any official list | [§5.2](#52-3023--not-a-command-in-220325) |
| **3024** | *(recognized no-op — undocumented)* | dev ≤220325 | ✓ | 0 | **MISSING** (Player's silent-skip is near-equivalent; yield semantics differ) | [§5.3](#53-3024--recognized-no-op) |
| 3025 | Edit Picture / ピクチャの編集 | 210519 | ✓ | 0 | **MISSING** (no dispatch case in this tree; save chunk `SavePicture 0x1C` modeled only) | [§4.25](#425-3025-edit-picture--ピクチャの編集) |
| 3026 | Output Image (liblcf `WritePicture`) / 画像の出力 | 210519 | ✓ | 0 | **FULL** — `src/game_interpreter.cpp:5351` | [§4.26](#426-3026-output-image-writepicture--画像の出力) |
| 3027 | Add Move Route / キャラの動作追加 | 210519 | ✓ (dispatcher no-op) | 0 | **MISSING** (no dispatch case, no MoveEvent lookahead in this tree) | [§4.27](#427-3027-add-move-route--キャラの動作追加) |
| 3028 | Edit Picture (Tile) / ピクチャの編集(チップ) | 210809 | ✓ | 0 | **MISSING** | [§4.28](#428-3028-edit-picture-tile--ピクチャの編集チップ) |
| 3029 | Control Message / 文章処理の制御 | 241028 (**dispatched in 220325 dev**, RE) | ✓ | 0 | **MISSING** (LSD hook chunks `0x32/0x42–0x46` modeled only) | [§4.29](#429-3029-control-message--文章処理の制御) |
| 3030 | Script (line 1) / スクリプト(1行目) | 241028 | ✗ | 0 | **MISSING** (not in liblcf) | [§4.30](#430-30303031-script--スクリプト) |
| 3031 | Script (lines 2+) / スクリプト(2行目~) | 241028 | ✗ | 0 | **MISSING** (not in liblcf) | [§4.30](#430-30303031-script--スクリプト) |
| 3032 | Zoom Screen / 画面のズーム | 241028 | ✗ | 0 | **MISSING** (`Maniac_Zoom` enum exists; upstream PR [#3499](https://github.com/EasyRPG/Player/pull/3499) open) | [§4.32](#432-3032-zoom-screen--画面のズーム) |
| 3033 | Console / コンソール | 241028 | ✗ | 0 | **MISSING** (not in liblcf) | [§4.33](#433-3033-console--コンソール) |

Code-assignment provenance: 3001–3019 from liblcf and the 211010 `command_new.txt` コード
blocks (the renewal commands 3020/3021 carry no コード block in 211010 — their numbers come
from liblcf and the 241029 lists); 3025–3033 (incl. the 3030/3031 line split and the
*absence of 3022–3024*) from the
**official 241029 `command_code.txt`**, which lists `03001…03021, 03025…03033` and skips
3022–3024 entirely (Maniacs 241029 `command_code.txt`). The 220325 dispatch facts are RE
(BelovedRapture.exe `FUN_004500d0`; dispatch analysis 2026-06-09). BR counts from the corpus
census (11 distinct Maniac codes, 4,450 instances).

---

## 3. Conventions shared by the per-command sections

Each section below gives: official JP/EN doc translation → per-index parameter table (official
211010 layout merged with the RE-verified 220325 layout; conflicts shown side-by-side) →
string-field usage → sub-op tables → RPG_RT quirks → EasyRPG status detail. 220325 handler
addresses are from the dispatch census (RE: BelovedRapture.exe `FUN_004500d0` jump tables;
dispatch analysis 2026-06-09).

---

## 4. Per-command reference

### 4.1 3001 Get Save Info / セーブ情報の取得

Since ≤181209 · 220325 handler `FUN_00436ce0` · BR uses 7 (custom save menu, CE 77
`CMSGetSaveInfo`) · EasyRPG **PARTIAL** `src/game_interpreter.cpp:4403-4481`.

"デフォルトのロード画面に表示される内容と同じ情報を取得する" — reads the same information the
default load screen shows for slot N: date/time, lead-actor level and HP into variables, and the
party's **face graphics into 4 pictures**. Faceset files are treated as **4×4 sprite sheets**
("4*4のスプライトシートとして扱われます"); sheets with ≥16 patterns misalign. If the save does
not exist, the **date variable is set to 0**. Date order is YYMMDD even in the EN build.
(Maniacs 211010 `command_new.txt`.)

| idx | Official 211010 (translated) | Notes |
|---:|---|---|
| 0 | save-slot mode (0=literal 定数, 1=variable 変数) | 2-state, not the full nibble |
| 1 | save slot number | |
| 2 | dest var: save date `YYMMDD` | 0 when slot missing |
| 3 | dest var: save time `HHMMSS` | |
| 4 | dest var: lead actor level | |
| 5 | dest var: lead actor HP | |
| 6 | 予約項目(0固定) — reserved, fixed 0 | (hypothesis) the renewal "hero name" output may live here or in the string field — layout **(UNVERIFIED)** |
| 7 | picture-ID mode (0=literal, 1=variable) | |
| 8–11 | picture IDs receiving member 1–4 face graphics | member 1 = lead actor |

String field: none (211010). Renewal builds added **lead-actor name retrieval**
("先頭キャラの名前を取得できるようになりました", Maniacs 211010 `command_modified.txt`,
renewal section) — output channel **(UNVERIFIED / RE-pending)**.

**RPG_RT quirks.**
- The face pictures **keep the target picture's prior settings except the file/spritesheet
  items** ("ファイルとスプライトシート関連の項目を除き、もとの設定をそのまま使用します") — the doc
  advises moving the target IDs off-screen or transparent beforehand. **Conflict:** Player
  instead shows the face with *fresh default* `ShowParams` (`use_transparent_color=true`,
  `top_trans=100`, map/battle layer 7, spritesheet 4×4) — `src/game_interpreter.cpp:4463-4477`.
  Whether RPG_RT truly preserves prior position/effects is **(UNVERIFIED)**; flag for RE.
- On a **corrupted** (unparseable) save, RPG_RT writes the magic value **8991230** into the
  date variable; Player replicates this verbatim ("Maniac Patch writes this for whatever
  reason", `src/game_interpreter.cpp:4434`).
- Face path is resolved as `..\FaceSet\<name>` relative to the save dir (Player mirrors:
  `:4472`); empty face name erases the target picture (`:4458-4460`).

**EasyRPG detail.** All 12 params implemented; missing: renewal hero-name read. Upstream PR
history: [#2623](https://github.com/EasyRPG/Player/pull/2623).

### 4.2 3002 Save / セーブの実行

Since ≤181209 · 220325 handler `FUN_00437390` · BR uses 2 (`Auto-Save`, `CMSConfirmSave`) ·
EasyRPG **FULL** `src/game_interpreter.cpp:4483-4501`.

Default-equivalent save to slot N; slot ≤0 is invalid. Optional success flag → variable
(1=success, 0=failure). (Maniacs 211010 `command_new.txt`.)

| idx | Official 211010 | Player |
|---:|---|---|
| 0 | slot mode (0=literal, 1=variable) | `ValueOrVariable(p0,p1)` `:4488` |
| 1 | slot number | |
| 2 | receive result? (0=no, 1=yes) | `:4494` |
| 3 | result variable | |

**RPG_RT quirk (replicate):** "変数への代入はセーブを実行してからなされる" — the result variable
is assigned **after** the save is written, so the savegame on disk holds the *pre-assignment*
value. Player achieves the same ordering by yielding to the update loop via
`AsyncOp::MakeSave(slot, out_var)` (`:4498`, comment: game data could be in an undefined state).

### 4.3 3003 Load / ロードの実行

Since ≤181209 · 220325 handler `FUN_00437510` · BR uses 7 (`CMSConfirmLoad`) · EasyRPG
**PARTIAL** `src/game_interpreter.cpp:4503-4532`.

Default-equivalent load of slot N (≤0 ignored). Option: pre-validate the file with a
Get-Save-Info-equivalent check and abort when invalid ("確実に正しいファイルが存在する場合には
無効にしても構いませんがおすすめしません" — you may disable it but it is not recommended).
211010 added an option to disable the post-load fade ("暗転を無効にするオプションを追加",
Maniacs 211010 `command_new.txt` リニューアル版 addendum + 241029 `update.txt` 2021/10/10).

| idx | Official 211010 | Player |
|---:|---|---|
| 0 | slot mode (0=literal, 1=variable) | `:4508` |
| 1 | slot number | |
| 2 | pre-check (0=enabled 有効, 1=disabled 無効) — **note inverted-feeling encoding: 0 means check** | Player always checks; param ignored (`:4514-4516` "kinda useless feature") |
| 3? | (211010+) disable-fade option — index **(UNVERIFIED / RE-pending)** | not implemented |

**RPG_RT quirks.** With pre-check *disabled* and the file missing, **RPG_RT crashes**
(`:4516`). The load produces a **black screen followed by a fade-in that a transition event can
cancel** — Player loads instantly without the fade (FIXME at `:4527-4528`).

### 4.4 3004 End Load Process / ロード処理の終了

Since ≤181209 · 220325: inline handler (dispatcher case stub) · BR uses 0 · EasyRPG **FULL**
(no-op) `src/game_interpreter.cpp:4534-4537`.

Legacy command to clear the post-load blackout. Official doc, note dated 18/12/07: "このコマンド
は現状何の効果もありません" — **currently has no effect** (the blackout clears by itself after
one frame). No string, no parameters. (Maniacs 211010 `command_new.txt`.)

### 4.5 3005 / 3006 Mouse Position / マウス座標の取得・設定

Since ≤181209 · 220325 handlers `FUN_00437100` (get) / `FUN_00437240` (set) · BR uses 0 ·
EasyRPG: 3005 **FULL** `src/game_interpreter.cpp:4539-4562`; 3006 **WONTFIX** warn-stub
`:4564-4571` ("system specific, not exposed by SDL", metabug
[#1818](https://github.com/EasyRPG/Player/issues/1818)).

Both use window-relative coordinates on a **320×240 logical basis** ("320*240の解像度を基準に
します") — i.e. scaled regardless of actual window size; with a custom `Winw/Winh` resolution
the basis behavior is **(UNVERIFIED)** against the 220325 binary. Set clamps **X to 0–319 and
Y to 0–239** to avoid stray clicks. Known official caveat: values are wrong under
fullscreen + DirectDraw renderer ("おそらく問題ありません" — practically irrelevant).
(Maniacs 211010 `command_new.txt`.)

| Cmd | idx | Meaning |
|---|---:|---|
| 3005 | 0 | dest var: X |
| 3005 | 1 | dest var: Y |
| 3006 | 0 | coordinate mode (0=literal, 1=variable) |
| 3006 | 1 | X |
| 3006 | 2 | Y |

Mouse *state* (buttons/wheel) is read via the extended baseline Key Input Processing (§6.5)
and persists into LSD via overloaded keyinput chunks
([maniac-patch-fileformats.md §7.2](maniac-patch-fileformats.md)).

### 4.7 3007 Show String Picture / 文字列ピクチャの表示

Since 190119 · 220325 handler `FUN_00443b50` (a structural clone of ShowPicture
`FUN_00448c40` that renders text into the picture surface, text via `FUN_00442920`) · BR uses
**2273** (the single heaviest Maniacs command in BR — its whole custom UI is string-picture
driven) · EasyRPG **FULL** `src/game_interpreter.cpp:4573-4734` — the RE found **no
discrepancy at any index** ("FULL match", semantics analysis 2026-06-09).

Generates a picture from a string; afterwards it behaves like a normal picture. Window
background, frame, gradient and shadow can be disabled to lower generation cost.
(Maniacs 211010 `command_new.txt`.)

**Control characters inside the string** (official list; `[]` values accept `\V` references):
`$a-zA-Z` EXFONT glyph · `\\` literal backslash · `\C[n]` text color · `\D[n]` zero-pad
variable values to ≥n digits · `\N[n]` actor-n name · `\T[n]` "現時点では未実装" (*unimplemented
at 211010*; string-variable insert in later builds) · `\V[n]` variable value.

**String field** — packed sub-strings, each with a 1-byte type prefix:
`*1 表示する文字列 *1 ファイル名 *1 フォント名` = `[tag]display-text [tag]system-file
[tag]font-name`. 211010 doc: "現在は0x01のみ有効" (only `0x01` literal). 220325 RE: tags
`0x01`=literal, `0x02`=string-var, `0x03`=indirect, with the string-mode selector in
param[22] (RE: `FUN_00443b50`). Player implements the full 0x01/0x02/0x03 tokenizing
(`src/game_interpreter.cpp:4573` ff.).

Per-index layout — official 211010 (params 0–21) merged with RE 220325 (params 0–23). The two
agree index-for-index; 22–23 are renewal/dev additions absent from the 211010 numbered list:

| idx | Meaning (official 211010 / RE 220325) | Sub-fields |
|---:|---|---|
| 0 | 値のタイプ — packed mode word | bits 0–3 picture-ID nibble · 4–7 position nibble · 8–11 magnify nibble · 12–15 transparency nibble · 16–19 picture-size nibble · 20–23 **font-size nibble** (Ghabry's "5-bit font packing" = this nibble, #1818) · 24–27 **origin** (0=center 中心, 1=top-left 左上, 2=bottom-left 左下, 3=top-right 右上, 4=bottom-right 右下, 5=top 上, 6=bottom 下, 7=left 左, 8=right 右) |
| 1 | picture ID | |
| 2 / 3 | position X / Y | per origin in p0 bits 24–27 |
| 4 | magnify (width % since 211010 split) | |
| 5 | top transparency | |
| 6–9 | red / green / blue / saturation tone | |
| 10 | effect mode (特殊効果の種類) | incl. Maniacs fixed-angle effect 3 |
| 11 | effect power | |
| 12 | 追加エフェクト — blend/flip flags | bits 0–3 blend (0=none, 1=multiply 乗算, 2=add 加算, 3=overlay オーバーレイ) · bit 4 flip-H · bit 5 flip-V |
| 13 | オプション1 — Maniacs flags | bits 0–7 fixed-to-map (マップのスクロールに連動) · 8–11 background type (0=none, 1=stretch 拡大, 2=tile タイル) · 12 no-frame (枠を描画しない) · 13 gradient-off · 14 shadow-off · 15 bold · 16 margin-off (余白無効) · 24–27 / 28–31 (angle effect only) value-1 / value-2 type nibbles |
| 14 | オプション2 | bits 0–7 transparent-color enable · 8–15 letter spacing 文字間隔 (0–255, default 0) · 16–23 line spacing 行間隔 (0–255, default 4) |
| 15 / 16 | map layer / battle layer | |
| 17 | erase condition / applied screen effects (消去条件/適用する画面効果) | |
| 18 / 19 | picture width / height | 0 or "auto" mode ⇒ auto-sized |
| 20 | font size | clamped 1–255, default 12; mode = p0 nibble 5 |
| 21 | (angle effect) value 2 / RE: effect divisor for fixed-angle | |
| 22 | *(RE/renewal)* displayed-string id — string-mode selector for the 0x02/0x03 tags | absent from 211010 numbered list |
| 23 | *(RE/renewal)* magnify height — independent X/Y scaling (211010 feature) | Player gates the independent-height path on `param[0] >= 0x10000000`-style flagging per RE |

**RPG_RT quirks.** Frame is skipped when either dimension < 8 px regardless of the flag;
margins, when enabled, are 8 px left/right and 10 px top/bottom; with gradient disabled, the
glyph color is sampled from the top-left (0,0) of each 16×16 system-palette cell; font name
empty ⇒ DB system font; `<システム設定>`/`<System Settings>` as system-graphic name ⇒ DB system
graphic. Editor preview constraints (fixed 0,0 / 100% / opacity 0 / no effects / size 12 on
non-literal) are editor-only. (Maniacs 211010 `command_new.txt`.) 241028 fixed trailing-newline
handling ("末尾の改行が無視されていたのを修正", Maniacs 241029 `update.txt` 2024/10/28) — the
220325/211010 behavior *ignores a trailing newline* (replicate for those builds).

**EasyRPG detail.** Persistence of generated string pictures uses EasyRPG-private save chunks
(`SaveEasyRpgWindow`/`SaveEasyRpgText`), which is why Maniacs↔EasyRPG save round-trip of string
pictures is incompatible (issue [#3439](https://github.com/EasyRPG/Player/issues/3439); see
[maniac-patch-fileformats.md](maniac-patch-fileformats.md)).

### 4.8 3008 Get Picture Info / ピクチャ情報の取得

Since 190217 · 220325 handler `FUN_0044a850`, op jump table at `0x44af34` (ops 0–3) · BR uses 1
· EasyRPG **PARTIAL** `src/game_interpreter.cpp:4736-4801` (ops 0–2 implemented; **op 3
missing** — the switch at `:4758` has no `case 3`).

Reads a displayed picture's position/size into 4 variables; rotation/special effects are not
reflected. (Maniacs 211010 `command_new.txt`.)

| idx | Meaning |
|---:|---|
| 0 | picture-ID indirection nibble (0/1/2) |
| 1 | info op — official ピクチャの段階 (0=元画像 source image, 1=移動中 current/moving, 2=移動後 finish); RE adds **op 3 extended query** |
| 2 | value shape (0=center x,y+w,h 中心[xy]wh, 1=xywh top-left, 2=ltrb rect) |
| 3 | picture ID |
| 4–7 | dest vars 1–4 |

Op table (merged):

| op | Official 211010 | RE 220325 (`0x44af34`) | Player |
|---:|---|---|---|
| 0 | 元画像 — source-image values | current x,y → p4,p5; w,h zeroed | `case 0` `:4759` (x,y current; w,h = sprite size — **note** Player returns the unscaled sprite size rather than zeroing; divergence (UNVERIFIED which matches RPG_RT for op 0)) |
| 1 | 移動中 — current | current x,y + scaled w,h | `case 1` `:4763` |
| 2 | 移動後 — finish | finish x,y + scaled w,h | `case 2` `:4769` |
| 3 | *(not in the 211010 numbered layout; the 211010 `command_modified.txt` renewal section adds "画像のピクセルデータを取得可能になりました" — pixel-data query, so it is a renewal-era 210519–211010 addition; `update.txt` never itemizes it)* | extended query at `0x44ac6c`: reads **two extra value params** (p4/p5 with own mode nibbles) plus more picture fields, then writes a result | **missing** |

When the picture is unloaded, dest vars are zeroed (RE). Player additionally yields and retries
while the picture request is pending (`:4744-4748`) — an emulation-of-async artifact with no
RPG_RT equivalent.

### 4.9 3009 Control Battle / 戦闘処理の制御

Since 190217 (hooks 状態付与 state-infliction and ダメージ以外の値変動 non-damage stat change
added 190526) · 220325 handler `FUN_00412f00` · BR uses 4 (`BeginBattle=====`) · EasyRPG
**FULL** `src/game_interpreter_battle.cpp:706-721` (+ hook plumbing `:631-704`).

Lets the game override five default-battle processes with a common event. **Battle-only;
all hooks reset when the battle ends.** Designed to pair with trigger-6 battle-start common
events (§7). "処理をおこなうコモンイベント内ではウェイトのかかるコマンドを用いるべきではありません" —
the hook CE must not contain waiting commands. (Maniacs 211010 `command_new.txt`.)

| idx | Meaning |
|---:|---|
| 0 | hooked process (0=ATB増加 AtbIncrement, 1=ダメージポップ DamagePop, 2=ターゲッティング Targetting, 3=状態付与 SetState, 4=ダメージ以外の値変動 StatChange) — Player enum `ManiacBattleHookType` `battle.cpp:55-59`. *Note:* the official numbered list is stale — it enumerates only "(ATB=0, ポップ, ターゲット)" while the same file's prose documents all five processes; IDs 3/4 are the 190526 additions in prose order |
| 1 | common-event-ID indirection nibble (0/1/2) |
| 2 | common event ID (0 presumably unhooks — (UNVERIFIED), Player treats ≤0 as no hook `battle.cpp:639`) |
| 3 | first variable of the hook's argument block |

Hook argument blocks (written into consecutive variables starting at p3 before each CE
invocation; "取得" read-only, "取得/設定" read-write — the engine reads the value back after
the CE runs):

| Hook | Vars (1-based offsets) |
|---|---|
| ATBゲージ増加 (per-frame natural gauge gain) | 1 unit side (0=ally,1=enemy) · 2 unit index (index, **not** ID) · 3 current gauge (≥300000 ⇒ can act) · 4 **pending gauge increment (get/set)** |
| ダメージポップ (damage/heal pop; suppresses default text when hooked) | 1 side · 2 index · 3 X · 4 Y · 5 kind (0=damage,1=heal,2=evade) · 6 value |
| ターゲッティング (called just before an action) | 1 side · 2 index · 3 action kind (0=basic, 1=skill, 2=transform, 3=item) · 4 action value (basic: 0=attack,1=double attack,2=defend,3=observe,4=charge,5=self-destruct,6=escape,7=nothing; else skill/enemy/item ID) · 5 target scope (0=single foe,1=all foes,2=single ally,3=all allies) · 6 **target index (get/set)** — set ignored for self/all targets |
| 状態付与 (state inflicted) | 1 side · 2 index · 3 state ID |
| ダメージ以外の値変動 (non-HP change; args aligned with DamagePop) | 1 side · 2 index · 3 X · 4 Y · 5 kind (3=MP,4=atk,5=def,6=spi,7=agi) · 6 value |

**EasyRPG detail.** Player registers `(common_event_id, var_start)` per hook
(`battle.cpp:717-718`) and fires them through `ManiacBattleHook` → a dedicated
`maniac_interpreter` that runs queued sub-events to completion while the battle waits
(`battle.cpp:631-704`, `ProcessManiacSubEvents` `:696`). RPG_RT 190625 fixed "余計なフレーム
経過" (a stray frame elapsing) during hook CE execution — i.e. the hook is **synchronous within
the frame** (Maniacs 241029 `update.txt` 2019/06/25); Player's drain-until-idle loop matches.
241028 fixed errors when targeting writes out-of-range values and when a SetState hook
re-inflicts a state (same `update.txt` 2024/10/28) — 220325 still has those crash edges
(replicate-or-guard decision left to implementer).

### 4.10 3010 Control ATB Gauge / ATBゲージの操作

Since 190217 · 220325 handler `FUN_00412bf0`, gated on `DAT_0056c7f0 == 2` (battle scene) ·
BR uses 110 · EasyRPG **FULL** `src/game_interpreter_battle.cpp:723-808` — RE FULL-match
including the scaling.

Gauge valid range **0–300000** (officially documented; RE: ATB lives at battler+0x3c).

| idx | Meaning |
|---:|---|
| 0 | target type — official: 0=主人公ID actor by ID, 1=メンバー party slot, 2=パーティ whole party, 3=敵キャラ enemy by index, 4=敵全体 whole troop. **Conflict note:** the RE summary glossed type 0 as "active/first battler"; official docs and Player (`battle.cpp:776-805`, `GetActor(target_id)`) both say *actor by database ID* — the RE gloss is presumed a misreading **(RE-pending re-check)** |
| 1 | target indirection nibble (0/1/2) |
| 2 | target value |
| 3 | operation: 0=set 設定, 1=add 加算, 2=sub 減算 |
| 4 | value kind: 0=raw 値, 1=percent 割合 |
| 5 | value indirection nibble |
| 6 | value |

**The ATB scaling answer** (settles the question raised on PR
[#3545](https://github.com/EasyRPG/Player/pull/3545)): internal range is 0..300000; percent
input is scaled `value × 3000` (= value/100 × 300000) (RE: `FUN_00412bf0`, percent flag at
param offset 0x10; semantics analysis 2026-06-09). Player computes
`value/100 * Game_Battler::GetMaxAtbGauge()` with `GetMaxAtbGauge() = 300000`
(`battle.cpp:748`, `src/game_battler.h:1081`) — identical.

### 4.11 3011 Change Battle Command EX / 戦闘コマンドの変更EX

Since 190217 (party-command editing added 190526; empty-command behavior changed 190220) ·
220325 handler `FUN_00412b90` · BR uses 238 (troop pages) · EasyRPG **FULL**
`src/game_interpreter_battle.cpp:810-844`.

Battle-only; settings revert after the battle. Two features (Maniacs 211010 `command_new.txt`):

| idx | Meaning |
|---:|---|
| 0 | Row (隊列変更) command: 0=enabled 有効, 1=disabled 無効 — Player maps to `easyrpg_disable_row_feature` `battle.cpp:818` |
| 1 | party-command bitmask, bits 0–4 = 戦う fight, オート auto, 逃げる escape, 勝利 win, 敗北 lose |

Party-command semantics: pick any 4 of {fight, auto, escape, **win**, **lose**} — win/lose
*force* victory/defeat. All five selected ⇒ first four used; none ⇒ default three; **exactly
one ⇒ the party-command screen transition is skipped entirely**. Player's bit decode
(`battle.cpp:820-835`): fight/auto/escape bits *remove* a default command, win/lose bits *add*
one — equivalent given the defaults.

**RPG_RT quirks.** It is possible to leave a battler with **zero battle commands**: pre-190220
the engine fell through to AI action at selection time; from 190220 "透明の項目を選択で隊列変更
する" — an invisible row-change item is selectable instead (Maniacs 211010 `command_new.txt`
190220 addendum). 210809 fixed mis-parsed values in a battle-command command ("戦闘コマンドの
変更 コマンド: 指定値が正しく解釈されていなかったのを修正", Maniacs 241029 `update.txt`
2021/08/09) — **attribution caveat:** the entry omits the EX suffix, so it may refer to the
baseline Change Battle Commands (13120/1009) rather than 3011; pre-210809 builds mis-read some
inputs in whichever command is meant **(exact mis-parse and target command UNVERIFIED)**.

### 4.12 3012 Get Battle Info / 戦闘情報の取得

Since 190526 · 220325 handler `FUN_00412860`, per-battler field reader `FUN_00412490` · BR uses
274 (battle handlers) · EasyRPG **PARTIAL** `src/game_interpreter_battle.cpp:846-968` —
fields 1/2 have known divergences.

Reads in-battle values into consecutive variables; **no-op outside battle**. (Maniacs 211010
`command_new.txt`.)

| idx | Meaning |
|---:|---|
| 0 | target type (0..4, same set as 3010 §4.10) |
| 1 | info selector — single targets (0=能力補正値 stat modifiers, 1=状態 states, 2=属性 attributes, 3=その他 misc); group targets パーティ/敵全体 (0=総数 count-all, 1=生存者 alive, 2=行動可能者 can-act) |
| 2 | target indirection nibble |
| 3 | target value |
| 4 | dest variable base |

Single-target fields — official vs RE vs Player:

| field | Official 211010 | RE 220325 (`FUN_00412490`) | Player (`battle.cpp:865-909`) |
|---:|---|---|---|
| 0 | atk/def/spi/agi battle modifiers, in that order | 4 ints from battler+0x18..0x24 | `GetAtk/Def/Spi/AgiModifier` → 4 vars — **match** |
| 1 | **states**: v[dst]=total state count, then per-state **turns since infliction** (1 at infliction, 0 if not afflicted) — example: 3 DB states, just-blinded ⇒ `3,0,0,1` | count at battler+0x30 + ushort list at +0x34 (the battler's own state array) | resizes battler list to `Data::states.size()`, writes count then per-state values (`:874-888`). **Divergence (⚠):** the binary writes the battler's stored array length; Player always writes the DB count. If RPG_RT sizes the battler array to the DB count at battle start the two coincide — **(RE-pending: confirm array sizing)**. Official doc semantics (turn counters, DB-sized) side with Player |
| 2 | **attributes**: v[dst]=total count, then per-attribute resistance state (2=raised, 1=normal, 0=lowered); "各耐性の値は省略されることがあり、その場合は初期値(1)扱い" — trailing values **may be omitted**, treated as 1 | count at battler+0x28 + byte list at +0x2c (battler byte array, possibly shorter than DB) | iterates **all** `Data::attributes`, writes `rate_shift+1` for each (`:889-899`). **Divergence (⚠):** when the battler array is shorter, RPG_RT leaves the trailing dest vars **unwritten (stale)** while Player writes 1s |
| 3 | misc: sprite center X, center Y, can-act (0/1), defending, charged, appeared (出現状態) | battler+0x48/0x4c (x,y), can-act, defend flag, +0xbf, +0xbc | x, y, `CanAct`, `IsDefending`, `IsCharged`, `!IsHidden` (`:900-908`) — **match** |

Group targets: counts battlers matching the filter and (per RE) writes their **indices into the
var array**; Player writes only the count (`battle.cpp:941-963`) — **divergence (⚠,
RE-pending: confirm the index-list write in `FUN_00412860`)**.

### 4.13 3013 Control Var Array / 変数配列の操作

Since 190526 (参照外し dereference + linked-array sort: 210530; 連動シャッフル linked shuffle,
copy←, repeat-fill 値の繰り返し代入, reverse リバース: 241028) · 220325 handler `FUN_0044bad0` ·
BR uses 0 · EasyRPG **PARTIAL** `src/game_interpreter.cpp:4803-4894` — ops 0–15 (the full
211010 set) implemented; the 210530/241028 additions are absent (op IDs **(UNVERIFIED)**)
and hit the warning default `:4887`.

Treats consecutive variables as an array; length 1 = scalar. (Maniacs 211010 `command_new.txt`.)

| idx | Meaning |
|---:|---|
| 0 | operation (table below) |
| 1 | mode word: bits 0–3 target-1 (0=variable index literal 変数, 1=`v[v[x]]` 変数番変数) · bits 4–7 size (0=literal, 1=var, 2=var-var) · bits 8–11 target-2 (enumerate: 0=literal/1=var/2=var-var; otherwise 0=variable, 1=var-var) |
| 2 | target-1 (array A start) |
| 3 | size |
| 4 | target-2 (array B start / enumerate seed / scalar) |

| op | Name | Semantics | Player |
|---:|---|---|---|
| 0 | コピー copy | A → B (assigns left to right; **all other binary ops apply B onto A**) | `:4821` |
| 1 | 交換 swap | A ↔ B | `:4827` |
| 2 / 3 | ソート sort asc / desc | sort A | `:4831/:4835` |
| 4 | シャッフル shuffle | randomize A — **RPG_RT bug (≤241028): shuffles one element fewer than specified** ("対象が指定数より1少なくなっていた", fixed 241028; Maniacs 241029 `update.txt`). 220325 has the bug; Player does **not** replicate it (`:4839`) — faithful-emulation decision open | `:4839` |
| 5 | 列挙 enumerate | fill A with seed, seed+1, … | `:4843` |
| 6–15 | 加算/減算/乗算/除算/剰余/Or/And/Xor/Shl/Shr | element-wise `A[i] ∘= B[i]` ("オペランドも配列" — the operand is also an array) | `:4847-4885` |
| ? | 参照外し dereference (210530) / linked-sort option (210530) / linked shuffle, copy←, repeat-fill, reverse (241028) | op IDs and layout **(RE-pending)** | missing |

190904 fixed unspecified-condition errors in this command ("特定条件でエラー"); 200126 fixed
errors when using "current limit + 1"-numbered variables in "「変数配列の操作」等の追加コマンド"
(this and the other extension commands) — i.e. the command **auto-grows the variable array**
(Maniacs 241029 `update.txt` 2019/09/04 and 2020/01/26); Player's variable store grows on
demand, matching.

### 4.14 3014 Key Input Proc EX / キー入力の処理EX

Since 190904 · 220325 handler `FUN_0044c420` · BR uses 0 · EasyRPG **PARTIAL**
`src/game_interpreter.cpp:4896-4932` — ops 0/1 implemented (conflated: no joypad layer),
op 2 implemented, **ops 3–5 (joypad) unsupported** (warn `:4926`; upstream treats joypad remap
as WONTFIX — EasyRPG has its own remapping; metabug #1818; Shift/Ctrl/Alt quirk issue
[#3098](https://github.com/EasyRPG/Player/issues/3098)).

Extended key input: returns 0/1 per key, **never waits**. When a joypad assignment is active,
keyboard state queries include pad input. (Maniacs 211010 `command_new.txt`.)

| idx | Meaning |
|---:|---|
| 0 | operation 0–5 (table below) |
| 1 | first variable of the result/input array |
| 2 | (op 2 only) keycode indirection nibble (0/1/2) |
| 3 | (op 2 only) keycode 0–255 |

| op | Name | Semantics |
|---:|---|---|
| 0 | キーボードの状態一覧取得 | dump the editor keyboard list's key states into vars (Player: `ManiacPatch::GetKeyRange()` → **50 keys**, `src/maniac_patch.cpp:608`) |
| 1 | 同（割り当て無効） | same, ignoring any joypad assignment |
| 2 | キーコード指定で状態取得（割り当て無効） | query one raw keycode 0–255 (Player maps through `RuntimePatches::VirtualKeys` `:4918-4924`) |
| 3 | ジョイパッドの状態一覧取得 | dump joypad button states |
| 4 | ジョイパッドの割り当て取得 | read current joypad→key map (-1 = unassigned) |
| 5 | ジョイパッドの割り当て設定 | bulk-assign joypad buttons to keys (persisted in LSD `SaveSystem 0x8B maniac_joypad_bindings`, [fileformats §7.2](maniac-patch-fileformats.md)) |

History quirks: 190920 fixed state-dump not updating variables when no joypad present; 191007
fixed the joypad button↔doc mapping; 210809 fixed mouse-wheel state retrieval (Maniacs 241029
`update.txt`). Pre-fix builds mis-behave accordingly.

### 4.15 3015 Rewrite Map / マップの書き換え

Since 190904 · 220325 handler `FUN_0044d330` · BR uses 0 · EasyRPG **PARTIAL**
`src/game_interpreter.cpp:4934-4970` — single-tile-ID writes work; the **range-of-IDs mode
(p1=1) is unimplemented** (FIXME `:4940`); A/B autotile edge cases unsupported (the current
partial support landed upstream as merged PR
[#3306](https://github.com/EasyRPG/Player/pull/3306) "Partially implement Maniacs Command
3015"; the A/B-autotile follow-up is open PR
[#3502](https://github.com/EasyRPG/Player/pull/3502)).

Rewrites tiles of the hero's current map. **Changes are not saved and are lost on map move.**
(Maniacs 211010 `command_new.txt`.)

| idx | Meaning |
|---:|---|
| 0 | mode word: bits 0–3 tile-ID (single: 0=literal/1=var/2=var-var; range: 0=variable, 1=var-var) · 4–7 left X · 8–11 top Y · 12–15 width · 16–19 height (each 0/1/2) |
| 1 | tile spec (0=single ID 単体ID, 1=range of IDs 範囲ID — IDs read from a variable array) |
| 2 | layer (0=lower 下層, 1=upper 上層) |
| 3 | tile ID value, or var-array start for range mode |
| 4–7 | left / top / width / height |
| 8 | autotile handling (0=enabled 有効, **1=disabled** 無効) |

History quirks (all pre-211010 builds in the wild may exhibit them): 190920 — range application
order was bottom-right→top-left (fixed to normal order); 191007 — single-tile coordinates were
offset; 191020 — lower-layer range writes with autotile disabled were processed as upper-layer
(Maniacs 241029 `update.txt`). Tile-ID space is shared with 3028 Edit Picture (Tile).

### 4.16 3016 Control Global Save / 共有セーブの操作

Since 190904 (renewal: 文字列変数に対応 — string-variable support; Maniacs 211010
`command_modified.txt` renewal section) · 220325 handler `FUN_0044e420` (file `Save.lgs`,
header `LcfGlobalSave` — RE string scan) · BR uses 0 · EasyRPG **PARTIAL**
`src/game_interpreter.cpp:4972-5028` + `src/maniac_patch.cpp:881-989` — switch/variable
semantics FULL; the renewal **string-variable data type is missing** (Player handles
`type` 0/1 only, `:5003-5019`; the on-disk chunk a string copy would use is unknown —
`Save.lgs` chunk IDs other than 1/2 are skipped, see
[fileformats §7.3](maniac-patch-fileformats.md)).

Cross-savegame shared store; fixed filename `Save.lgs`. (Maniacs 211010 `command_new.txt`.)

| idx | Meaning |
|---:|---|
| 0 | operation: 0=開く open, 1=閉じる close (discards unsaved changes!), 2=保存する save, 3=保存して閉じる save+close, 4=共有セーブからコピー copy-from-global, 5=共有セーブへコピー copy-to-global |
| 1 | (copy ops only) mode word: bits 0–3 local start (0=literal, 1=variable 変数参照) · 4–7 global start (0=literal, 1=variable) · 8–11 size (0/1/2 full nibble) |
| 2 | data type (0=switch スイッチ, 1=variable 変数; renewal adds string-var — ID **(UNVERIFIED)**, presumably 2) |
| 3 | local (current-save) start index |
| 4 | global-save start index |
| 5 | size |

Semantics to replicate: *open* while already open is ignored; *close*/*save* while not open are
ignored; **copy ops auto-open when closed** (Player: unconditional `Load()` before copy,
`:4992-4993`, and `Load()` marks the store open even when the file doesn't exist yet,
`maniac_patch.cpp:889-894`). Byte format of `Save.lgs`: see
[maniac-patch-fileformats.md §7.3](maniac-patch-fileformats.md).

### 4.17 3017 Change Picture ID / ピクチャのID変更

Since 191020 (191021 hotfixed out-of-range handling) · 220325 handler `FUN_0044e930` · BR uses
0 · EasyRPG **STUB** `src/game_interpreter.cpp:5030-5037` (warns "Command ChangePictureId not
supported"; upstream open PR [#3500](https://github.com/EasyRPG/Player/pull/3500)).

| idx | Meaning |
|---:|---|
| 0 | operation: 0=移動 move, 1=交換 exchange, 2=スライド slide |
| 1 | mode word: bits 0–3 target-1 · 4–7 size · 8–11 target-2 (each indirection nibble 0/1/2) |
| 2 | target-1 (first source ID) |
| 3 | size (count of consecutive IDs) |
| 4 | target-2 (move/exchange: destination ID) **or** distance (slide) |
| 5 | out-of-range errors: **0=ignore する, 1=don't ignore しない** — note the inverted-feeling encoding |

Semantics: *move* relocates `size` pictures from target-1 to target-2, **erasing** anything
occupying the destination; *exchange* swaps the ranges; *slide* shifts the range by ±distance.
With ignore-errors on, a move from/to an out-of-range ID degrades to a plain erase
("単なる消去処理に置き換わります"). (Maniacs 211010 `command_new.txt`.)

### 4.18 3018 Set Game Option / ゲームのオプション設定 — RESOLVED 2026-06-09

Since 191020 (frame-skip op added 191103) · 220325 handler `FUN_0044ecd0`, **op jump table at
`0x44f364`** · BR uses 16 (incl. CE 16 `GameSpeed=====`, CE 9 `+GPConfiguration+`) · EasyRPG
**STUB** `src/game_interpreter.cpp:5039-5055` — reads `operation = parameters[1]`, implements
**only op 2 as a deliberate no-op** (arbitrary picture count is native), warns on everything
else (`:5051`).

Parameter skeleton:

| idx | Meaning |
|---:|---|
| 0 | indirection nibble (0/1/2) for the op value in p2 |
| 1 | **op selector** (table below) |
| 2 | op-specific value |
| 3 | op-specific secondary value/flags (op 1 only) |

> #### ✅ RESOLVED op map (consumption-site adjudication, 2026-06-09)
>
> The earlier conflict between the official 211010 docs (= PR
> [#3487](https://github.com/EasyRPG/Player/pull/3487)) and a first-pass RE reading of the
> 220325 binary was settled by tracing each op's globals to their **consumption sites** in
> **both** the 220325 (`BelovedRapture.exe`) and the public **211010 reference**
> (`maniacs-PE211010-en-im`, sha `49c33cb9`) binaries. **The official docs / PR #3487 are
> correct.** The first-pass RE reading (op 0 = frame-limiter, op 3 = key-config, op 4 = render
> flag, op 7 = glyph cell) was **wrong** and is corrected below. The adjudicated table:
>
> | op (p1) | Meaning | p2 | p3 | Consumer evidence (220325 / 211010) | Build |
> |---:|---|---|---|---|---|---|
> | 0 | **Run-when-inactive** — behavior when the window is unfocused (default *wait*; **saved**) | 0=wait ウェイト, 1=run 実行 | — | `DAT_0056c834` tested in WndProc `FUN_004a50f0` WM_ACTIVATE (~`0x4a5228`) + inactive pause-loop `FUN_00469f60:54` / 211010 `FUN_004a36d0`+`FUN_00469420` | both |
> | 1 | **Fatal Mix** (debug group) | FPS (≥1; **not** saved) | bits 0–3 TestPlay (0=keep, 1=off, 2=on → `DAT_0055ec19`); bit 4 right-Shift message-skip (`DAT_0056c836`) | pacer `FUN_00469f60`; `GetAsyncKeyState(0xA1)` `FUN_004ce000:107` / 211010 `FUN_004c70d0:64` | both |
> | 2 | **Picture max** (max picture ID; saved) | limit | — | bounds `FUN_0048f7f0` / 211010 `FUN_0048e700` | both |
> | 3 | **Frame skip** — render every n-th frame (saved) | 0=none, 1=1/5, 2=1/3, 3=1/2 (mask LUT `{0,4,2,1}`, `mask & frameCtr` skips the render switch) | — | `FUN_00489c20:78` (`DAT_0056d1e0` = render frame counter) / 211010 `FUN_00488ac0:79`; LUT recomputed on save-load `FUN_00495f40:376` | both |
> | 4 | **Message mouse-interaction gate** (polarity: 1=off *default*, 0=on) | 0/1 | — | mask `0xf0` vs `0x30`, `GetCursorPos` path, click bit `0x80`: `FUN_004ce000:57/351/408` / 211010 `FUN_004c70d0:55/391/448` | both |
> | 5 | **Battle/non-map UI anchor** — 3×3 placement of the 320×240 UI in custom windows | 0=center,1=TL,2=BL,3=TR,4=BR,5=TC,6=BC,7=CL,8=CR | — | `FUN_004a8000:31`, `FUN_004cc250:66`, getter `FUN_004a7f90` / 211010 `FUN_004a6570`/`FUN_004a65d0` | both |
> | 6 | **Hard picture-array rebuild** to clamp(value,1,1000) (distinct from op 2) | value | — | in-handler | both |
> | 7 | **FaceSet cell W×H** (default 48×48; persisted chunks `0x40`/`0x41`) | w | h | face grid `FUN_004cefd0:93-101`, face-X `FUN_004cc250:58`, indent `FUN_004ce000:253`, name-entry `FUN_004c2080:68` | **220325 only** |
>
> **Build delta:** 211010's switch has ops 0–6 (op 3 is the default-case target there; op ≥7
> falls through to it); 220325 adds **op 7** (FaceSet cell size) and nothing else. This is part
> of the one-command Discord delta (the other being command 3029) — see
> [maniac-patch.md §3.4](maniac-patch.md). Save persistence: ops with "saved" map onto the LSD
> "FatalMix"/option chunks — see [maniac-patch-fileformats.md §7.2](maniac-patch-fileformats.md).
>
> **EasyRPG action:** PR #3487 implements ops 0–5 with these exact semantics (op-4 polarity
> nonzero=disable, default 1; op-5 = battle-UI origin). It is the correct basis to adopt; op 6
> (rare) and op 7 (FaceSet cell, 220325-only) can follow. BR's `GameSpeed=====` event is **op 1
> FPS** → `Game_Clock::SetGameSpeedFactor(fps/60)`; this is BR's highest-impact 3018 gap.

Defaults observed in 220325 state-reset (`FUN_00491fb0`): picture limit 1000, FaceSet cell
48×48, others 0/1; FPS default 60 set at startup `FUN_004881a0` (RE 2026-06-09).

### 4.19 3019 Call Command / コマンドの呼び出し

Since 191103 · 220325: inline handler in the dispatcher (case stub `0x450432`) · BR uses 0 ·
EasyRPG **FULL with implementation difference** `src/game_interpreter.cpp:5469-5549`
(upstream PRs [#3140](https://github.com/EasyRPG/Player/pull/3140),
[#3319](https://github.com/EasyRPG/Player/pull/3319)).

"このコマンドは上級者向けです" — expert-only: invokes an arbitrary command code with arbitrary
arguments. Unknown codes either do nothing or alias an existing command (a direct consequence
of the dispatcher's selector hash — see [maniac-patch.md §9](maniac-patch.md)).
(Maniacs 211010 `command_new.txt`.)

| idx | Official 211010 | Player (dev-build extended layout) |
|---:|---|---|
| 0 | mode word: bits 0–3 code · 4–7 args-start · 8–11 argc (indirection nibbles) | same word, but bits 4–7 are a **processing mode**: 0=const / 1=variable (args fetched from the variable array), 2=*unimplemented even by Maniacs* (`:5477`), 3=**inline** (args follow in p5+, with mode nibbles chained 4-per-word from p2), 4=**expression** (p5+ holds expression bytecode, `ManiacPatch::ParseExpressions` `:5522`); bits 12–15 string-source selector |
| 1 | command code | `:5486` |
| 2 | first variable of the argument block | const/var: args start `:5496`; inline: first mode word `:5506` |
| 3 | argument count | `:5497/:5507` |
| 4 | — | string source (literal command-string vs string-var, via `CommandStringOrVariableBitfield(com,0,3,4)` `:5488`) |
| string | passed through to the invoked command (コード依存) | same |

The inline/expression processing modes are renewal/dev-era extensions not described in the
211010 doc; their presence in 220325 is **(UNVERIFIED — RE mapped only the dispatch stub, not
the argument decoding)**. 241028 fixed direct-value argument mis-interpretation ("引数の値を
直接指定するとき正しく解釈されないことがあった", Maniacs 241029 `update.txt`) — i.e. the inline
mode existed and was buggy before 241028.

**Official do-not-call list (replicate the limits):** unusable: Call Command itself.
Non-functional: Label. Mis-behaving: Show Choices, Battle/Shop/Inn Processing (branching
forms), Conditional Branch (map and battle), Loop, Show Message lines 2+ (20110), Comment
lines 2+ (22410) — all of these rely on indent/lookahead context the synthesized command lacks.

**EasyRPG implementation difference:** Player **pushes a single-command frame**
(`Push<ExecutionType::Eval>` `:5546`) instead of invoking the handler in place — "incompatible
to Maniacs but better compatibility with our code" (comment `:5544-5545`). Observable
difference: the synthesized command sees its own stack frame (affects 3021 op 4 nest queries
and `This Event` resolution in edge cases).

### 4.20 3020 Control Strings / 文字列変数の操作

Since 210519 · 220325 handler `FUN_00426720`, outer op switch at ~`0x4268bc` on
`param[3] & 0xff` · BR uses 1518 · EasyRPG **FULL** `src/game_interpreter.cpp:5057-5350` —
RE: "op numbering and sub-fn numbering identical" (semantics analysis 2026-06-09). Flagship
upstream PR [#3051](https://github.com/EasyRPG/Player/pull/3051); storage in
`src/game_strings.h/.cpp` (`Game_Strings`, persisted via LSD `SaveSystem 0x24`, see
[fileformats §7.2](maniac-patch-fileformats.md)).

The string-variable (`t[n]`) workhorse. Targets: `T[x]`, `T[x..y]`, `T[V[x]]`,
`T[V[x]..V[y]]`. (Maniacs 211010 `command_new.txt` — documented TPC-style; the editor had no
form for it through 241028.)

| idx | Meaning (Player comment block `:5061-5085`, RE-confirmed) |
|---:|---|
| 0 | mode word: bits 0–3 target string mode (single/range × literal/variable) · 4–7 / 8–11 / 12–15 / 16–19 mode nibbles for args 1–4 (numeric args: 0/1/2 indirection; string args: 0=literal, 1=`t[x]`, 2=`t[v[x]]`) |
| 1 | string index 0 |
| 2 | string index 1 (range upper bound) |
| 3 | packed op: **byte0 = operation** (table below) · **byte1 = sub-function** (asg/cat builders) · **byte2 = flags** (bit 1 hex `0x20000`, bit 2 extract `0x40000`, bit 3 exRep-"first"; *edge case:* exRep's `first` arg lives at `(p3 >> 19) & 1` — Player comment "Wtf BingShan" `:5250`) |
| 4..n | arguments (exMatch is the only op generating 8 params — 4 args) |

Outer op table (byte0 of p3) — official mnemonic / binary helper / Player case (all three
agree on every ID):

| op | Mnemonic | Meaning | 220325 helper (RE) | Player |
|---:|---|---|---|---|
| 0 | `.asg` | assign (empty string clears) | builder `FUN_00422d80` + `FUN_00493170` | case 0 |
| 1 | `.cat` | concatenate | `FUN_00422d80` + `FUN_004992c0` | case 1 |
| 2 | `.toNum` | string → number into variables (hex via flag; extract-digits via `0x40000`) | `_strtol` | case 2 |
| 3 | `.getLen` | character count (multibyte-aware, `__mbclen`) → variables | — | case 3 |
| 4 | `.inStr` | find substring → variable (-1 on failure); arg3 = start pos (start-pos handling buggy until 211010) | `FUN_004990c0` | case 4 |
| 5 | `.split` | split by delimiter → string array + count var | `FUN_004229f0` | case 5 |
| 6 | — | reserved / no-op | — | absent (correct) |
| 7 | `.toFile` | write string to `Text/<name>.txt` (encoding arg: 0=ANSI, 1=UTF-8; subfolders + auto-create since 210809; folder restriction removed 211010; **241028 bug: shift_jis selection wrote UTF-16** — fixed there) | `FUN_00426250` | case 7 |
| 8 | `.popLine` | destructively pop first line → other string. **Range quirk:** `t[a..b].popLine` does *not* operate per-string — it pops `(b-a)+1` lines and stores the **last** popped line (Player comment `:5318-5321`; range support fixed in Maniacs 241028) | `FUN_004227c0` | case 8 |
| 9 | `.exInStr` | regex search → variables (211010) | `FUN_004985e0` | case 9 |
| 10 | `.exMatch` | regex match-list extraction → string array (211010; 8th param edge case; **241028 fixed null-terminator leakage into captures** — 220325 has that bug) | `FUN_004989a0` | case 10 |

Inner sub-function table for asg/cat (byte1 of p3) — `FUN_00422d80` ↔ Player, IDs identical:

| sub | Mnemonic | Meaning |
|---:|---|---|
| 0 | (string) | literal/string operand, with min-size pad arg |
| 1 | (number) | number with zero-pad min-size (`\D` analogue) |
| 2 | (switch) | switch as ON/OFF text |
| 3 | `.name` | DB entity name — 19 tables: actor, skill, item, enemy, troop, terrain, element, state, anim, tileset, s[], v[], t[], cev, class, anim2, map, mev, member (full official list, Maniacs 211010 `command_new.txt`) |
| 4 | `.desc` | DB description (dynamic flag) — "説明" operand officially added 241028; present earlier per Player/TPC **(exact since-build UNVERIFIED)** |
| 6 | `.cat` | concat 2–3 operands |
| 7 | `.ins` | insert at index |
| 8 | `.rep` | replace first occurrence |
| 9 | `.subs` | substring (index, size) |
| 10 | `.join` | join `size` variables/strings with delimiter |
| 12 | `.file` | read `Text/<name>.txt` into the string (encoding arg; Player async-waits for the file `:5228-5241`) |
| 13 | `.rem` | remove range (211010) |
| 14 | `.exRep` | regex replace (211010; `first` flag in p3 bit 19) |

(Sub-IDs 5 and 11 are unassigned in every source — **(UNVERIFIED whether reserved)**.)

**RPG_RT quirks/history.** 210809: paths containing 2-byte characters were treated as invalid
(fixed); 211010: split with overlapping source/dest mis-behaved (fixed), large file reads
errored (fixed). The `Encoding=` ini key (EN build, 211010) governs file-IO code pages —
[fileformats §8](maniac-patch-fileformats.md). The rapid-write rate-limit warning string
("Outputting multiple files in a short time…") exists in 220325 (RE string scan) and is the
behavior behind Conditional Branch's "file output possible" check (§6.6).

### 4.21 3021 Get Game Info / ゲーム情報の取得

Since 210519 (op-set grew at 210530, 210809, 241028) · 220325 handler `FUN_004146c0` · BR uses
0 · EasyRPG **PARTIAL** `src/game_interpreter.cpp:4204-4401` — ops 0–10 dispatched, **op 3
(pixel info) unimplemented** (warn `:4243`).

Reads runtime values. Op selector = `parameters[1]`; common dest var = `parameters[2]`
(op-specific layouts below; the per-op parameter shapes are Player's decode, which is the only
index-level source for this command — official docs describe it via TPC mnemonics only —
**(layout RE-pending against `FUN_004146c0`)**).

| op | Since | Meaning | Player |
|---:|---|---|---|
| 0 | 210519 | map size in tiles → var, var+1 (`@sys.getInfo.mapSize`) | `:4213` |
| 1 | 210519 | tile IDs of a rect (upper/lower layer) → var array; p2=layer, p0 nibbles for x/y/w/h in p3–p6, dest in p7 (`@sys.getInfo.tiles`) — arg handling buggy until 210809 (fixed; Maniacs 241029 `update.txt`) | `:4217` |
| 2 | 210519 | window size px → var, var+1 (`@sys.getInfo.winSize`) | `:4237` |
| 3 | 210519 | screen pixels of a rect (`@sys.getInfo.pixel`) | **missing** `:4241-4243` |
| 4 | 210530 | executing-event info ("実行中のイベント情報"): with a "Nest" walk-back arg (p0-nibble2/p4), writes event type, event ID, page ID, execution type, current line (+1) → var..var+4; backed by the `SaveEventExecFrame` Maniacs chunks `0x0E–0x12` ([fileformats §7.2](maniac-patch-fileformats.md)). In battle, RPG_RT only reports the current line (Player comment `:4258-4261`). Stack walk-up was broken until 211010 (fixed) | `:4245` |
| 5 | 210809 | current chipset/tileset ID | `:4271` |
| 6 | 210809 | face graphic (actor or message face): name → `t[var]`, index → `v[p3]`; static/dynamic flag p6 | `:4274` |
| 7 | 210809 | walk/body graphic (actor or map event); **RPG_RT bugs replicated by Player** (comments `:4309-4310`): `.static 10001` returns the *current* player sprite while `.dynamic 10001` returns nothing; `.static 10005` (self) returns nothing while `.dynamic 10005` works | `:4300` |
| 8 | 210809 | camera position of screen origin in px ("カメラ位置") | `:4376` |
| 9 | 241028 | screen shake offsets x/y | `:4381` (present in 220325? **UNVERIFIED**) |
| 10 | 241028 | current BGM: name → `t[var]`, fadein/volume/tempo/balance → `v[p3..p3+3]` | `:4386` (220325 presence **UNVERIFIED**) |

**241028 quirk note:** "顔グラ / 歩行グラ の出力先が、それぞれtとvで入れ替わっていた" — ops 6/7
had their `t`/`v` outputs **swapped** until fixed in 241028 (Maniacs 241029 `update.txt`). A
faithful 220325 emulation must decide whether to reproduce the swap — **(RE-pending: check
`FUN_004146c0`'s ops 6/7 write order; Player currently implements the *fixed* orientation).**

The "battle-anim concurrency cap" (`@sys.gameOpt.animLimit`, 211010) documented in the same
official file is a *Set Game Option* concern, not a Get Game Info op — see §4.18.

### 4.25 3025 Edit Picture / ピクチャの編集

Since 210519 (210809: "透明状態の値を無視するオプション" ignore-alpha option) · 220325 handler
`FUN_0044af50` · BR uses 0 · EasyRPG **MISSING** — no dispatch case in this tree
(`src/game_interpreter.cpp:789-824` has no `Maniac_EditPicture`); liblcf models the
persistence chunk only (`SavePicture 0x1C maniac_image_data`, deflate-compressed —
[fileformats §7.2](maniac-patch-fileformats.md)).

Directly rewrites picture pixels (normal or string pictures): TPC
`@pic[1].setPixel .xywh x, y, w, h .src V[n]` — write a w×h rect from a variable array.
Edits persist in saves. (Maniacs 211010 `command_new.txt`.) Pixel format of the variable-array
encoding **(UNVERIFIED / RE-pending — likely 32-bit ARGB per variable, cf. op 3 of 3008 and
3021 op 3)**. 241028 fixed spritesheet-target corruption and save-restore of edited sheets;
220325 carries those bugs.

### 4.26 3026 Output Image (WritePicture) / 画像の出力

Since 210519 · 220325 handler `FUN_0044b330` · BR uses 0 · EasyRPG **FULL**
`src/game_interpreter.cpp:5351-5467`. Naming: official EN "Output Image" (211010
`command_new.txt` renewal section; 241029 `command_list.txt`); liblcf calls it
`Maniac_WritePicture`.

Writes the game screen or a picture as **32-bit PNG** (official: "32bitPNG形式"). TPC:
`@img.save.screen.dst "name"` / `@img.save.pic n .static/.dynamic .opaq .dst "name"`.
(Maniacs 211010 `command_new.txt`.)

| idx | Meaning (Player decode `:5356-5371`; official docs give TPC form only — index layout **(RE-pending vs `FUN_0044b330`)**) |
|---:|---|
| 0 | mode word: bits 0–3 picture-ID nibble · 4–7 filename source (0=literal string field, 1=string-var) |
| 1 | target (0=screen, 1=picture) |
| 2 | picture ID |
| 3 | filename string-var ID (when non-literal) |
| 4 | flags: bit0 dynamic (apply tone/flash/flip effects) vs static; bit1 opaque |
| string | output filename (`.png` appended when missing) |

Quirks (Player-observed Maniacs behavior): the opaque flag is ignored for string pictures
(`:5413-5415`); static mode crops the current spritesheet cell (`:5435-5437`). Official output
folder is `Picture/`; Player writes into the **save** filesystem instead
(`FileFinder::Save()`, `:5454-5456`) — sandbox-driven divergence, flagged.

### 4.27 3027 Add Move Route / キャラの動作追加

Since 210519 · 220325: **dispatcher no-op** — shared "return true" epilogue `0x451864`; the
command is consumed by **lookahead inside the MoveEvent (11330) handler `FUN_004140c0`**, which
loops `while (cmd->code == 0xbd3 /*3027*/)` collecting continuation route data (RE: dispatch
analysis 2026-06-09) · BR uses 0 (BR's 33,046 MoveEvents carry no 3027 continuations — census)
· EasyRPG **MISSING** in this tree (no dispatch case; `CommandMoveEvent`
`src/game_interpreter.cpp:3164` performs no 3027 lookahead). Upstream implementation exists as
PR [#3488](https://github.com/EasyRPG/Player/pull/3488) (not in this tree).

Appends **one move-route action per command line** immediately after a Set Move Route command;
exists for editor readability and to allow variable-specified action arguments. The official
action list covers the full standard move-route opcode set (moveUp…unfixDir, speed/freq/trans,
switch, setBody, se, through, anim pause/resume — Maniacs 211010 `command_new.txt`). Parameter
encoding of the action + args **(RE-pending — extract from `FUN_004140c0`'s consumption
loop)**. 241028 fixed switch/graphic/SE actions mis-functioning for some values; 220325 carries
those bugs.

**Implementation requirement:** because the dispatcher treats stray 3027 as a no-op, a 3027
that does **not** directly follow 11330 (or another 3027) is silently inert — replicate that
(do not error).

### 4.28 3028 Edit Picture (Tile) / ピクチャの編集(チップ)

Since 210809 ("リニューアル版から実装") · 220325 handler `FUN_0041cfb0` · BR uses 0 · EasyRPG
**MISSING** (no dispatch case in this tree).

Draws map tiles into a picture: TPC `@pic[n].drawTile` with `.xywh x,y,w,h` target rect,
`.lower`/`.upper` layer, `.single n` or `.range n` (IDs from variable array), `.disableAutoTile`
(lower layer), `.wipe` (clear rect first), `.tilesetId n` (0 = current map's tileset),
`.pattern n` (animation pattern; **-1 = mirror the map's current pattern**). Tile-ID space is
identical to Rewrite Map (3015). (Maniacs 211010 `command_new.txt`.) Per-index layout
**(RE-pending vs `FUN_0041cfb0`)**. 211010 fixed: single-ID lower-layer draws used the wrong
chip ID, and edits after wait-type commands were dropped — 220325 is post-fix.

### 4.29 3029 Control Message / 文章処理の制御

Officially new in 241028 ("新規追加") — **but already dispatched by the 220325 dev build**
(RE: handler `FUN_0043e700`; dispatch analysis 2026-06-09) · BR uses 0 · EasyRPG **MISSING**
— liblcf names the code (`Maniac_ControlTextProcessing` = 3029) and models the LSD hook state
(`SaveSystem 0x32` flags, `0x42–0x46` callback IDs — [fileformats §7.2](maniac-patch-fileformats.md));
Player implements neither the command nor the hooks.

Hooks message-window events to a common event. Hookable events (= the `0x32` flag bits):
**user event** (a `\E[…]` control char appears in text — `\E` invokes the CE with arbitrary
numeric/string args), **window created**, **window destroyed**, **character drawn** (文字描画).
Arguments are passed via numbered variable / string-variable blocks — a *system* callback
block and a *user* block (0 = skip), matching chunks `0x43–0x46`. (Official site 追加コマンド
section, via the web dossier; Maniacs 241029 `update.txt` 2024/10/28.) Per-index parameter
layout **(RE-pending — decompile `FUN_0043e700`; nothing index-level is published).**

### 4.30 3030/3031 Script / スクリプト

241028 only · **not dispatched in 220325** (RE) · BR uses 0 · EasyRPG **MISSING** (not in
liblcf; commands would survive load as raw codes and be silently skipped).

Executes **JavaScript**. **3030 carries line 1, 3031 carries lines 2+** (multi-line
continuation pattern, like 10110/20110) — the split is explicit in the official code list:
`03030 スクリプト(1行目)` / `03031 スクリプト(2行目~)` (Maniacs 241029 `command_code.txt`).
The bridge surface is deliberately minimal: switches/variables/string-vars only, e.g.
`s[1] = 1; v[1] = 100; t[1] = 'qwe';` (official site, via web dossier). Engine/runtime details
(JS engine identity, sandboxing, synchronicity) **(UNVERIFIED)**.

### 4.32 3032 Zoom Screen / 画面のズーム

241028 only ("新規追加") · **not dispatched in 220325** — liblcf's `Maniac_Zoom = 3032` is
**not** part of the 220325 surface (RE: no code path; the lone `0xBCF` byte hit is a JZ rel32
displacement; dispatch analysis 2026-06-09) · BR uses 0 · EasyRPG **MISSING** (enum exists;
upstream open PR [#3499](https://github.com/EasyRPG/Player/pull/3499)).

Zooms the screen image centered on a coordinate; **zoom-in only** (no zoom-out); purely
image-space — the camera/logical coordinates are unaffected; the layer parameter shares the
picture-layer space (0 = cancel the zoom); duration in frames. (Official site 追加コマンド via
web dossier.) Per-index layout **(UNVERIFIED — needs a 241028 binary or editor dump).**

### 4.33 3033 Console / コンソール

241028 only · not dispatched in 220325 · BR uses 0 · EasyRPG **MISSING** (not in liblcf).

Opens/closes a console window; writes strings; reads input into a string variable; fg/bg color
set with bits 1=blue, 2=green, 4=red, 8=intensify; options: expand control characters, hex
number output, suppress trailing newline, change fg, change bg. (Official site 追加コマンド via
web dossier.) Per-index layout **(UNVERIFIED).**

---

## 5. Undocumented command codes (3022 / 3023 / 3024)

None of these appear in any official document — the official 241029 `command_code.txt` jumps
`03021 → 03025` (Maniacs 241029 `command_code.txt`). All three facts below are RE
(BelovedRapture.exe; dispatch analysis 2026-06-09).

### 5.1 3022 — expression statement

Dispatched in 220325 (selector 0x510, case stub `0x450ced` → handler `FUN_00448a50`). The
handler copies the command's raw parameter array pointer + byte length into globals
(`DAT_0055f1ec`/`DAT_0055f1f4`) and invokes **`FUN_00445140`, the recursive expression-bytecode
VM** — i.e. 3022 = *execute inline expression bytecode as a statement*. It is the same VM that
backs the expression operand modes of Control Variables, Conditional Branch and Call Command.
The **full opcode set (0–78), builtin-function table (Fn 0–18) and wire encoding live in
[maniac-patch-fileformats.md §9](maniac-patch-fileformats.md)** — not duplicated here. Result
register `DAT_0055f1f8` (discarded for a statement).

Producer: TPC's backtick expression statements (hypothesis — no editor form exists). BR never
emits it (census: 0).

**EasyRPG:** the VM itself is **FULL** (`ManiacPatch::ParseExpression`,
`src/maniac_patch.cpp:51-122` opcode enum, RE-verified identical numbering — including the
in-place assignment opcodes 34–45, which Player supports in Maniac mode but deliberately
rejects when EasyRPG extensions are simultaneously active, `src/maniac_patch.cpp:151`).
The *command code* 3022 is **MISSING**: liblcf has no enum entry and Player never dispatches
it; on load it survives as a raw command and is silently skipped — harmless unless a game
relies on its side effects (in-place assignments **do** have side effects, so a TPC-authored
game using 3022 would silently lose writes — detectable via census).

### 5.2 3023 — not a command (in 220325)

**Not dispatched** by 220325 — no code path, no immediate reference. Not in liblcf, not in any
official code list. Whether it was ever assigned (reserved? removed? skipped to keep Script's
two codes adjacent later?) is an open question (§9). Treat as a gap in the code space; an
implementation must simply skip it like any unknown code.

### 5.3 3024 — recognized no-op

Dispatched in 220325 (selector 0x512) straight to the shared "return true" epilogue
`0x451864` — a **deliberate, recognized no-op**, distinct from the unknown-code default path:
recognized no-ops *return* from ExecuteCommand (yield point), unknown codes just continue the
fetch loop. Not in liblcf, not in official lists; possibly a removed command whose code the
editor may still emit (hypothesis). **EasyRPG: MISSING** — Player's silent-skip of unknown
codes is behaviorally near-identical; the only conceivable difference is the yield/continue
distinction inside a single frame **(practical impact: none observed; UNVERIFIED whether any
game depends on it)**.

---

## 6. Modified baseline commands

Maniacs extends many stock RM2k3 commands in place — same codes, extra parameters/modes. The
official source is `command_modified.txt` (211010 bundle; "リニューアル版" marks renewal-era
additions); since-builds from `update.txt`. Every code below is dispatched by 220325's
`FUN_004500d0` (RE), so the extensions read from the same `parameters[]` array. EasyRPG gates
most of them on `Player::IsPatchManiac()`.

### 6.1 10220 Control Variables / 変数の操作

The most-extended command (BR: 48,100 uses). Additions, with since-builds (Maniacs 211010
`command_modified.txt`; 241029 `update.txt`):

- **Target modes** (param[0]): range bounds by *variable value* (≤181209); auto-normalization
  `a > b ⇒ b..a` for batch ops; **expression target** — on-wire `arg0 = 4`, `arg1` = start
  index of the serialized target expression (200126; encoding in
  [fileformats §9.1](maniac-patch-fileformats.md)). Exact mode IDs for range-by-variable
  **(UNVERIFIED at index level)**; Player decodes them in `DecodeTargetEvaluationMode`.
- **Operations**: Or / And / Xor / Shl / Shr added to the assign/add/sub/mul/div/mod set
  (≤181209).
- **Operand IDs** (param[4]) — the Maniac additions as decoded by Player
  (`src/game_interpreter.cpp:1107-1280`, guard `operand >= 9 && !IsPatchManiac()` at `:1111`):

  | operand | Meaning | Since |
  |---:|---|---|
  | 9 | Party member by index (パーティメンバー; otherwise like actor) | ≤181209 |
  | 10 | Switch as 0/1 (`:1191`) | ≤181209 |
  | 11–18 | math fns Pow, Sqrt(×c), Sin((a/b)°×c), Cos, Atan2(×c), Min, Max, Abs (`:1201-1253`) | 190217 era (Sin/Cos gained the multiplier arg 190217) |
  | 19 | Binary op a∘b (Add…Shr family as an operand; incl. Random a..b) (`:1259`) | ≤181209 |
  | 20 | Ternary 三項演算 (`:1266`) | 190904 |
  | 21 | **Expression** — `arg4 = 21`, `arg5` = length, `arg6…` = bytecode (`:1279-1280`, `ManiacPatch::ParseExpression`) | 200126 |

  Plus item/value extensions inside existing operands: actor & enemy **ID** and **ATB gauge**
  items (≤181209); misc date/time/elapsed-frames (≤181209) and the **patch-version** item
  (190920; low 20 bits = YYMMDD, MSB = im 0 / pf 1, see [maniac-patch.md §4.3](maniac-patch.md));
  item/actor/event/enemy selectable **by variable** (≤181209); event operand "event ID" item
  (200126); actor/member "EXP for next level" (210809); **Lerp / Sum / Amin / Amax** operands
  (210530 — operand IDs **(UNVERIFIED)**; **MISSING in Player** — neither this tree nor
  upstream master implements any operand > 21; unknown operands hit the warning default
  `src/game_interpreter.cpp:1278` (verified 2026-06-09)). The expression VM's
  `misc(type)` table (0=gold … 13=patch version) is in `vexpr.txt` and
  [fileformats §9](maniac-patch-fileformats.md) — note its order ≠ the UI operand order.
- **EasyRPG status: FULL** for the documented pre-renewal surface (operands 9–21, all target
  modes, all operations); the 210530 **Lerp/Sum/Amin/Amax operands are MISSING** (also missing
  upstream — no Player PR mentions them; GitHub search 2026-06-09).

### 6.2 11110 Show Picture / 11120 Move Picture / 11130 Erase Picture

Additions (Maniacs 211010 `command_modified.txt`): angle effect (190526, as effect mode +
value-type nibbles in p13 bits 24–31 — cf. §4.7 idx 13); blend modes multiply/add/overlay +
flip H/V (190526, p12 — see §4.7 idx 12 for the bit layout, identical for 11110); ID / position
/ zoom / opacity by var-of-var (191103); **origin** nibble (191103; upper bits of p1 — Player
masks with `ManiacBitmask(p1, 0xFF)` `src/game_interpreter.cpp:2786-2787` (Show) /
`:2932-2933` (Move)); Move duration: **negative value = |value| frames** (vs normal value×6
frames, i.e. tenths) (since-build **(UNVERIFIED)** — it predates 191103, whose changelog fixes
the editor display of negative durations; Player clamps to −10000..10000 when Maniac `:3056`) and
duration by var/var-of-var (191103, "wait is variable" upper bits of p17, `:2987-2993`);
relative-move modes + RGBS by variable + "don't change" options for Move (renewal); separate
X/Y zoom (211010, normal draw only; Player `:2842-2848`, with the *effects*-picture W/H
scaling additionally gated on Maniacs **build ≥240423** per upstream — `:2844`); spritesheet
range animation (211010); Show defaults when arguments are missing changed at 190625 (erase on
map change + flash/shake affected — replicate per build). Erase Picture: ID by var-of-var,
**variable range**, and "all pictures" (191020; Player `:3107-3125`).
**EasyRPG status: FULL** (PRs [#2628](https://github.com/EasyRPG/Player/pull/2628) "Support
most Maniac Patch Picture enhancements", [#2741](https://github.com/EasyRPG/Player/pull/2741)
"Maniac Pictures: Support Origin, Fixed Angle and Frames for duration"), with save-state in
`SavePicture` Maniac chunks ([fileformats §7.2](maniac-patch-fileformats.md)).

### 6.3 12010 Conditional Branch / 条件分岐 (and 13310 in battle)

Branch-type space (param[0]) gains five Maniac values, decoded by Player at
`src/game_interpreter.cpp:3711-3778`:

| p0 | Meaning | Sub-encoding | Since | Player |
|---:|---|---|---|---|
| 12 | "Other" sub-ops | p1: 0=ロード直後 just-loaded · 1=ジョイパッド有無 joypad present · 2=ウィンドウアクティブ window active · 3=ファイル出力可能 file-output possible | ≤181209 / 190920 / 191020 / 211010 | `:3713-3741` (window-active hardcoded true; file-output = save-fs usable) |
| 13 | switch via variable (`s[v[x]]`) | p1 var, p2 ON/OFF | ≤181209 | `:3744-3748` |
| 14 | variable-indirect LHS (`v[v[x]]`) | p1 var, p2 RHS mode, p3 RHS, p4 operator | ≤181209 | `:3750-3756` |
| 15 | string comparison | p1 mode nibbles (LHS direct/indirect; RHS literal/direct/indirect), p2/p3 ids, p4 = op (bits 0–1) + ignore-case (bit 8); literal RHS in the string field | renewal (210519, with strings) | `:3758-3773`, `ManiacPatch::CheckString` `src/maniac_patch.cpp:689` |
| 16 | **expression** | bytecode from p6 onward | renewal | `:3775-3777` |

Plus in-place extensions to existing branch types: item/actor/event ID by variable (p3/p4
appendix params, `:3561-3581`, `:3638-3640`); "指定のマップイベントが存在" map-event-exists as a
flag on the orientation branch (210809; p4==1 ⇒ existence check, `:3642-3644`).
**EasyRPG status: FULL** (window-active is approximated — Player suspends on focus loss
anyway, comment `:3727-3729`).

### 6.4 12210 Loop / 12220 Break Loop / 22210 End Loop

Typed loops (≤181209 for the type system, per `command_modified.txt`): param[0] type —
0=∞ infinite (vanilla), 1=回数指定 X-times, 2=カウントアップ count-up a→b, 3=カウントダウン
count-down, 4=While, 5=Do-While; param[1] = mode word (begin/end value nibbles + comparison op
in bits 8+); p2/p3 = begin/end values; **p4 = optional index variable** (0-based for X-times /
While / Do-While; the running index for count-up/down). Loop state persists per frame in the
`SaveEventExecFrame` Maniac chunks `0x11/0x12` ([fileformats §7.2](maniac-patch-fileformats.md)).
Player: `CommandLoop` `src/game_interpreter.cpp:3821-3881`, `CommandEndLoop` `:3910` —
**FULL for types 0–5**; the 210809 **map-event enumeration** loop type (`@foreach`) is *not*
implemented — unknown types skip the loop body entirely (`default:` `:3855-3858`)
**(its type ID UNVERIFIED; presumed 6)**. Jumping into a loop via Label leaves count/index
unset (official caveat — replicate).
**Break Loop** (≤181209 "fixed behavior inside nested loops"): vanilla RPG_RT has a bug where
Break jumps to the *next* EndLoop regardless of nesting; **Maniacs fixes it** to jump to the
end of the enclosing loop. Player implements both: bug emulated when Maniac is off, correct
scoped jump when on (`:3883-3908`, `has_bug = !Player::IsPatchManiac()`). Renewal added
multi-level break + one-iteration skip ("抜け出すブロック数を指定 / 1ループ処理をスキップ",
`command_modified.txt`) — param layout **(UNVERIFIED)**, not in Player.

### 6.5 11610 Key Input Processing / キー入力の処理

Mouse support (≤181209): left/right/middle click + wheel up/down as additional checkable
inputs; **wheel is only valid with "wait until pressed"** (no held state). Results piggy-back
on the standard keyinput result chunks via bit 1 overloads
([fileformats §7.2](maniac-patch-fileformats.md)). Player: `CommandKeyInputProc`
`src/game_interpreter.cpp:3274` ff. — mouse decode at `:3346-3380`, with a deliberate
ordering note: Player checks mouse buttons first whereas "Maniac checks them last"
(`:3208-3210`) — a tie-break divergence when multiple inputs land on the same frame
**(impact: cosmetic; flagged)**. **Status: FULL** (platform-gated).

### 6.6 12330 Call Event / イベントの呼び出し

Common event callable **by variable** and **by var-of-var** (≤181209). Player:
`CommandCallEvent` `src/game_interpreter.cpp:4026` — **FULL**.

### 6.7 10710 Battle Processing / 戦闘の処理

Option to disable the pre-battle flash (190217). Parameter index **(UNVERIFIED — RE-pending;
nothing index-level published)**. **EasyRPG: MISSING** (#1818 checklist; the flash is always
played). Relevant to BR (573 EnemyEncounter uses).

### 6.8 11330 Move Event / キャラクターの動作指定 (+ 3027)

Renewal: target event and frequency by variable (`command_modified.txt` 変数指定 list); and the
**3027 Add Move Route continuation protocol** — the handler look-ahead-consumes consecutive
3027 lines (§4.27). Player: `CommandMoveEvent` `src/game_interpreter.cpp:3164` — var-target
supported; **3027 lookahead MISSING in this tree**.

### 6.9 Renewal-era variable-argument sweep (210519+)

`command_modified.txt`'s closing list — items that gained variable/var-of-var argument modes in
the renewal rebuild: System BGM/SE change (file, fade, volume, tempo, balance), Show/Move
Picture (file name, sheet split, tone), Play BGM / Play SE (file + numeric args), Move-route
target/frequency, Character Flash args (210809), Wait value (plus **frame-unit wait**), Face
Graphics (target, file, index), Show Battle Animation (target, ID), Change Tileset, Set Event
Location target, Hero Name/Title (target, text), Fade BGM time, Parallax (file, scroll
speeds), System Graphics file, Hide/Show Screen transition type. Player implements these
piecemeal (e.g. Wait frames `src/game_interpreter.cpp:1994-2011`, frame unit = `wait_type 256`;
upstream PR
[#2914](https://github.com/EasyRPG/Player/pull/2914) for BGM/SE/Wait) — **PARTIAL as a group;
per-item status (UNVERIFIED audit, tracked in #1818)**. Also renewal: Open Save Menu (11910)
became a general "system function call" and Change Save Access (11930) controls those
functions (Player: `src/game_interpreter_map.cpp:653,781`, with the Maniacs fullscreen-mode
arg noted as "Broken in Maniac" `:795`); Scroll Screen (11060) gained pixel-unit ops (Player:
double-precision pan speeds via `SavePartyLocation 0x8D/0x8E`); Shop (10720) products by
variable; Message Options (10120) window size/font (Player: model-only — TODO at
`src/game_interpreter.cpp:980`); Show Battle Animation (11210) playback buffers / reverse /
position binding (211010) — **MISSING** in Player.

### 6.10 Multi-line continuation codes (for completeness)

The Maniacs additions follow the stock continuation conventions; the 220325 dispatcher
consumes them by lookahead exactly like vanilla: 20110 (ShowMessage_2), 20141, 22011/23311,
22410, 20713/20722/20732 are *not* dispatched but consumed by their parent handlers; 3031
would be the same pattern for 3030 in 241028 (RE: dispatch analysis 2026-06-09; §4.30).

---

## 7. Battle-trigger common events (triggers 6 / 7)

Not commands, but command-adjacent: Maniacs extends the common-event `trigger` value space —
**6 = 戦闘開始 battle start**, **7 = 戦闘時に並列処理 battle parallel** (vanilla 3=auto,
4=parallel, 5=call). On disk this is a *value* of the existing trigger chunk
(liblcf `Trigger_maniac_battle_start=6` / `_battle_parallel=7`,
`lib/liblcf/src/generated/lcf/rpg/commonevent.h`; confirmed by Ghabry on PR
[#3545](https://github.com/EasyRPG/Player/pull/3545)). Since 190217. These drive every Maniacs
custom battle system, including BR's (its `BeginBattle=====` CE registers 3009 hooks from a
trigger-6 event).

**RE-verified firing model (220325)** — the authoritative spec
(RE: BelovedRapture.exe `FUN_004ac5c0` / `FUN_004abf00`; CE struct: trigger byte at +0x14,
condition-switch flag +0x15, switch ID +0x16; semantics analysis 2026-06-09):

| Trigger | When | How | Ordering |
|---|---|---|---|
| **6** battle start | **once**, during battle initialization | iterate spawned CE instances where `trigger == 6` and the condition switch passes; run each **synchronously** (blocking, like an auto-start event) via `FUN_004ab7c0` | after troop/actor setup (`FUN_004597b0`), **before** the first frame / ATB loop begins; official caveat: the CE must not contain waiting commands |
| **7** battle parallel | **top of every battle frame** | iterate CE instances where `trigger == 7` + condition switch; run each as a **parallel (non-blocking)** interpreter via `FUN_004ab8d0`; when idle it **re-triggers** the next frame | strictly **before** ATB / battle-state advancement (`FUN_00481660` / `FUN_004599b0`) — a trigger-7 event can therefore mutate gauges the same frame they would fire |

**EasyRPG status: MISSING — the #1 BR combat blocker.** No code in `src/` references the
trigger-6/7 enum values (tree-wide grep, 2026-06-09); `Game_Interpreter_Battle`'s
`ManiacBattleHook` machinery (§4.9) exists but nothing scans common events for triggers 6/7.
Open PR [#3545](https://github.com/EasyRPG/Player/pull/3545) implements them upstream; it is
unmerged and gated on tester reports ("will never be merged" without them — maintainer
position, see [maniac-patch.md §9](maniac-patch.md)). Player's `GetGameInfo` op 4 already
*decodes* execution types 6/7 for reporting (`src/game_interpreter.cpp:4258-4261` notes the
battle limitation), and the `maniac_event_info` save bitfield reserves them
(`battle_start=6, battle_parallel=7` execution types —
[fileformats §7.2](maniac-patch-fileformats.md)).

---

## 8. EasyRPG support rollup

Status per §2 (judged against the 220325 surface; anchors are this tree, branch RISKY —
upstream master may be ahead, e.g. PRs #3488/#3499/#3500/#3545 listed open as of 2026-06-09):

| Status | Count | Codes |
|---|---:|---|
| **FULL** | 10 | 3002, 3004, 3005, 3007, 3009, 3010, 3011, 3019*, 3020, 3026 |
| **PARTIAL** | 9 | 3001, 3003, 3008, 3012, 3013, 3014, 3015, 3016, 3021 |
| **STUB** | 2 | 3017, 3018 |
| **WONTFIX** | 1 | 3006 |
| **MISSING** | 10 | 3022, 3024, 3025, 3027, 3028, 3029, 3030, 3031, 3032, 3033 |
| **N/A** | 1 | 3023 |

\* 3019 is FULL-with-implementation-difference (frame push, §4.19).

**Known divergences in implemented commands** (the concrete fix-list, from the RE
discrepancy analysis 2026-06-09 — each with its section):
1. 3018 ops other than 2 unimplemented; **op 1 (FPS/GameSpeed) is the highest-impact gap**
   (BR `GameSpeed=====`) but its semantics sit inside the §4.18 conflict.
2. 3008 GetPictureInfo **op 3** missing (§4.8).
3. 3012 GetBattleInfo **field 1 (states)** length/content may differ (§4.12).
4. 3012 GetBattleInfo **field 2 (attributes)** trailing-value handling differs (§4.12).
5. Battle CE **trigger 6** not scanned (§7).
6. Battle CE **trigger 7** not scanned (§7).
7. 3022 in-place assignment opcodes blocked in EasyRpg-extensions mode (policy, §5.1).
8. 3001 face-picture settings reset vs preserved (§4.1, UNVERIFIED which is faithful).
9. 3013 shuffle: RPG_RT off-by-one bug not replicated (§4.13).
10. 3026 output directory: save-fs vs `Picture/` (§4.26).
11. 11610 mouse-vs-key check ordering (§6.5).

BR-weighted view (what actually blocks Beloved Rapture): its 4,450 Maniac-command instances
are 51% 3007 + 34% 3020 (both FULL), and the remainder concentrates in the battle quartet
3009–3012 (FULL/FULL/FULL/PARTIAL) + 3018 (STUB) — so the effective blockers are **triggers
6/7 (§7)**, **3018 op 1**, and the 3012 field divergences, in that order.

---

## 9. Open questions

1. ~~**3018 op-map adjudication**~~ — **RESOLVED 2026-06-09** (§4.18): consumption-site tracing
   in both the 220325 and 211010 binaries confirms the official-docs/PR-#3487 reading; ops 0–6
   exist in 211010, op 7 (FaceSet cell) is the only 220325 addition. Remaining sub-question:
   confirm the op-1 p3 bit-4 right-Shift-skip flag's exact save-chunk mapping (§4.18 vs
   fileformats §7.2).
2. **3023** — reserved or removed? (§5.2). Ask upstream/BingShan; check whether any TPC build
   emits it.
3. **3024 producer** — what (if anything) emits it; whether any editor build writes it (§5.3).
4. **3012 battler-array sizing** — does RPG_RT size the per-battler state array to the DB
   count at battle start (which would reconcile RE and the official doc)? (§4.12.)
5. **3012 group-target index-list write** — confirm `FUN_00412860` writes member indices, not
   just the count (§4.12).
6. **3021 ops 6/7 t/v output orientation in 220325** — pre-241028 swap bug (§4.21); also
   whether ops 9/10 exist in 220325.
7. **3001 renewal hero-name output channel** and the face-picture settings-preservation
   question (§4.1).
8. **3016 string-variable copy** — data-type ID and the `Save.lgs` chunk it implies (§4.16;
   cross-ref fileformats open question 6).
9. **Index-level layouts still missing**: 3025 pixel encoding, 3027 action encoding, 3028,
   3029, 3032, 3033; Battle Processing flash-disable param (§6.7); Loop foreach type ID and
   renewal Break params (§6.4); Lerp/Sum/Amin/Amax operand IDs (§6.1); 3013 dereference /
   linked-sort / 241028 op IDs (§4.13).
10. **3005/3006 coordinate basis under custom resolution** (§4.5).
11. **3008 op 0 w/h semantics** — zeroed (RE) vs sprite size (Player) (§4.8).

---

## 10. References

Primary:
- **Official Maniacs documentation bundles** (BingShan): build 211010 — `command_new.txt`
  (per-command コード/文字列引数/数値引数 layouts), `command_modified.txt`, `command_code.txt`,
  `cmdl_en.txt`, `vexpr.txt`; build 241029 — `command_list.txt`, `command_code.txt`,
  `battle_command_list.txt`, `update.txt` (dated changelog 2018-08-09→2024-10-28), `vexpr.txt`.
  Corpus copies: `c:\rg\easyrpg_library\maniacs_ref_211010\docs\`,
  `c:\rg\easyrpg_library\maniacs_ref_241029\docs\`. Live equivalents:
  <https://bingshan1024.github.io/steam2003_maniacs/> and
  <https://github.com/BingShan1024/steam2003_maniacs> (`vexpr.txt`, `vexpr_bytecode.txt`).
- **Binary RE of build 220325** — BelovedRapture.exe dispatch census (`FUN_004500d0`, 147
  IDs, jump tables, lookahead consumers) and command-semantics analysis (3001–3021 priority
  layouts, ControlStrings op tables, ShowStringPicture packing, ATB scaling, trigger-6/7
  firing model, 3018 op table `0x44f364`), 2026-06-09. RE notes corpus:
  `br_dispatch_findings.md`, `br_dispatch_ids.tsv`, `br_command_semantics.md`/`.tsv`
  (outside the repo; see [README.md Provenance](README.md)).
- **liblcf** — `lib/liblcf/src/generated/lcf/rpg/eventcommand.h:170-196` (Maniac codes),
  `lcf/rpg/commonevent.h` (triggers 6/7).
- **EasyRPG Player (this tree)** — `src/game_interpreter.cpp:789-824` (map/shared Maniac
  dispatch), `:4204-5549` (handlers), `src/game_interpreter_battle.cpp:252-259, 631-968`,
  `src/maniac_patch.{h,cpp}` (expression VM, GlobalSave, key range, CheckString),
  `src/game_strings.*`.
- **Beloved Rapture census** — corpus `c:\rg\easyrpg_library\beloved_rapture\command_usage.md`
  (liblcf scan, 533,201 commands, 2026-06).

Secondary:
- EasyRPG metabug [#1818](https://github.com/EasyRPG/Player/issues/1818); upstream PRs cited
  inline — merged: #2623, #2628, #2741, #2914, #3051, #3140, #3306, #3319; open (states
  verified via GitHub API 2026-06-09): #3487, #3488, #3499, #3500, #3502, #3545. Issues #3098,
  #3439; BR-specific upstream work: PR #3293 / issue #3128 (see
  [../games/beloved-rapture.md](../games/beloved-rapture.md)).
- jetrotal's CSA / TPC reference: <https://jetrotal.github.io/CSA/> ·
  <https://github.com/jetrotal/CSA> (TPC mnemonics used in §4.20–§4.28).
- Web dossier (official-site extraction with quotes): research corpus `maniac_patch_web.md`
  (2026-06-09).

Sibling specs: [maniac-patch.md](maniac-patch.md) ·
[maniac-patch-fileformats.md](maniac-patch-fileformats.md) ·
[../games/beloved-rapture.md](../games/beloved-rapture.md) ·
[easyrpg-extensions.md](easyrpg-extensions.md) · [README.md](README.md)
