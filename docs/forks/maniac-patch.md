# Maniac Patch (マニアクスパッチ / "Maniacs" / "まにぱ" manipa) — overview & version lineage

> Maniac Patch is the most ambitious community binary fork of the **Steam release of RPG Maker 2003
> (v1.12a)**. Authored by 氷山羊 (BingShan / BingShan1024), it patches both the editor (`RPG2003.exe`)
> and the runtime (`RPG_RT.exe`) to add ~30 new event commands (codes 3001–3033), a third
> "string variable" type, an integer expression VM (`式`), a cross-savegame global save, custom
> window resolution, runtime font loading, deep battle/picture hooks, and (in the newest builds)
> JavaScript scripting and asset encryption. Games adopt it to build custom menus, battle systems
> and HUDs that stock 2k3 cannot express. **Beloved Rapture** (the immediate porting target,
> [`../games/beloved-rapture.md`](../games/beloved-rapture.md)) ships an `RPG_RT.exe` whose version
> resource reads `RPG Maker 2003 Runtime (Maniacs, v220325, en, im)`.

This document is the **overview**. The exhaustive event-command surface is *intended* to live in
**maniac-patch-commands.md** — **that companion file does not yet exist** (planned; see §11). Until
it is written, the per-code summary in §5 below is the most detailed command reference in this tree,
the **expression bytecode + per-chunk layouts** live in
[maniac-patch-fileformats.md](maniac-patch-fileformats.md) (§9 expression VM, §7 chunks), and the
**RE ground truth for build 220325** is [`/home/john/re/br_dispatch_findings.md`](/home/john/re/br_dispatch_findings.md).
Cross-links to `maniac-patch-commands.md` are retained as forward references to that planned doc.

**Status at a glance:** EasyRPG support = **PARTIAL (deep)** — the "legacy" (assembler-patched
v1.12a) command set up to ~v210414 is near-complete in master; the main remaining gap is
parallel/battle common events (open PR [#3545](https://github.com/EasyRPG/Player/pull/3545)).
Detection = `accord.dll` **or** PE VERSIONINFO `Maniacs, vNNNNNN` + PE section heuristics.
Spec confidence = **HIGH** for identity/lineage/detection/runtime model; the precise command
surface of build **220325** is **RE-confirmed** in [`/home/john/re/br_dispatch_findings.md`](/home/john/re/br_dispatch_findings.md)
(see §3.4, §9).

---

## 1. Identity

| Field | Value |
|---|---|
| Names | **Maniac Patch** / **Maniacs** / マニアクス / マニアクスパッチ / "まにぱ" (manipa) |
| Community nickname for patched build | **S2k3MP** ([VIPRPG@総合制作技術Wiki](https://wikiwiki.jp/viprpg-dev/2003/%E6%8B%A1%E5%BC%B5%E3%83%91%E3%83%83%E3%83%81)) |
| Author | **氷山羊 (BingShan / "BingShan1024")** — mail `bingshanyang at gmail dot com`; X [@BingShan1024](https://x.com/bingshan1024); Misskey.io `@BingShan1024`; Bluesky `@bingshan1024.bsky.social` ([official site, 作者 "author"](https://bingshan1024.github.io/steam2003_maniacs/)) |
| Official site (alive) | <https://bingshan1024.github.io/steam2003_maniacs/> (Japanese single-page app; all "official site" quotes below come from its embedded `index.html`). Source repo: <https://github.com/BingShan1024/steam2003_maniacs> |
| Distribution (alive) | Password-protected ZIPs on uploader.jp, account `xingqier`: <https://ux.getuploader.com/xingqier/>. The ZIP password is printed in the filename on the download page, e.g. `patch_maniacs_241029(pass=***).zip` ([Steam discussion](https://steamcommunity.com/app/362870/discussions/0/591762563949193002/)) |
| Support channels | 開発室 ("dev room") Discord, permanent invite <http://discord.gg/5NnbMtQ> (a 2026 EasyRPG comment also cites `discord.gg/aSskA7fcxW`, [#1818](https://github.com/EasyRPG/Player/issues/1818)); Shitaraba BBS <https://jbbs.shitaraba.net/game/60858/> |
| Liveness | **Actively maintained.** Latest published engine `RPG_RT.exe` = **241028** (241029 fixed only the JP initial database); editor `RPG2003.exe` = **240822** ([official site, トップ "Top"](https://bingshan1024.github.io/steam2003_maniacs/)) |

**License / redistribution.** Two layered conditions ([official site, リンク "links"](https://bingshan1024.github.io/steam2003_maniacs/)):
1. The general **Degica/Enterbrain Patch EULA** for Steam RM2k3, which explicitly permits creating
   and using binary patches "including commercial use, with no fee or royalty/revenue share"
   ([rpgmaker.net #24260](https://rpgmaker.net/forums/topics/24260/?p=1); EULA link
   [steamcommunity](http://steamcommunity.com/app/362870/discussions/0/541906348059148217/)).
2. A Maniacs-specific term: 自己責任での使用 — "use at your own risk."

Practical consequence for this project: shipping **detection data / behavior specs** is fine
(facts about a freely-distributed patch); shipping the patched `RPG_RT.exe`, `accord.dll`, or
extracted bundled text files is governed by the EULA and is not done here.

---

## 2. Target engine builds

**Steam RPG Maker 2003 v1.12a only.** The patch does not apply to Don Miguel / RPG Advocate /
other international RM2k3 builds; the VIPRPG wiki and the EasyRPG "known patches" wiki both stress
this ([VIPRPG wiki](https://wikiwiki.jp/viprpg-dev/2003/%E6%8B%A1%E5%BC%B5%E3%83%91%E3%83%83%E3%83%81);
[EasyRPG known-patches](https://wiki.easyrpg.org/development/technical-details/known-patches)).

The patch ships a **whole replacement `RPG_RT.exe`** (plus `ultimate_rt_eb.dll` and `accord.dll`),
not a delta against the user's binary. How that EXE is produced differs by era and matters for
detection (§4):

- **"old" builds (≤210414)** — the v1.12a binary is **directly assembler-hacked**. Internally
  still version `1.1.2.1`, retaining the stock logos / `CODE` segment / `CHERRY` segment; the
  feature code is grafted on (early builds add a **`GEEP`** PE section, later ones drop `GEEP` and
  enlarge `CHERRY`). EasyRPG calls these **"legacy" Maniac Patch**
  ([0.8.1 notes](https://blog.easyrpg.org/2025/04/easyrpg-player-0-8-1-stun/);
  [src/exe_reader.cpp:553-558](/home/john/Player/src/exe_reader.cpp)).
- **"renewal" builds (210519–211010)** — "改変範囲を全体にして1からコンパイルしたもの" (whole-program
  recompile). Still 32-bit; still internally `1.1.2.1`. **Beloved Rapture's 220325 sits in this
  era** and RE confirms it is a **C++/MSVC interpreter grafted onto the Delphi binary**
  ([`/home/john/re/br_dispatch_findings.md`](/home/john/re/br_dispatch_findings.md) §Methodology).
- **"64bit" builds (241028–241029)** — a 64-bit engine reverse-compiled from RPG_RT source; "no
  logos, no `CODE` segment, no `CHERRY` segment" ([src/exe_reader.cpp:522-524](/home/john/Player/src/exe_reader.cpp)).

---

## 3. Version lineage

Build IDs are **date codes `YYMMDD`** (e.g. `220325` = 2022-03-25). Era labels are the official
download page's own grouping. "Published" = appeared on the official download page / uploader. The
two **2022 dev builds are NOT published** — this is critical and RE-confirmed (§3.4). Headline
deltas are condensed from the official 更新履歴 ("changelog"); the per-command "Since" column in
[maniac-patch-commands.md](maniac-patch-commands.md) is authoritative for command introduction.

> **Build identity & naming.** The `YYMMDD` code equals the binary's PE `TimeDateStamp` date, so
> builds are named **`maniacs-PE<YYMMDD>-<lang>-<variant>`** (e.g. `maniacs-PE220325-en-im`) with a
> `sha12` disambiguator. The naming convention, the fingerprint scheme, and the registry of every
> build we hold (with hashes, timestamps, and witness games) live in
> [maniac-patch-builds.md](maniac-patch-builds.md) (+ machine-readable
> [`maniac-patch-builds.tsv`](maniac-patch-builds.tsv)).

| Build | Date | Era | Headline changes | Published |
|---|---|---|---|---|
| 180809 | 2018-08-09 | old | 動作テスト版 — first operational test release | ✅ |
| 181209 | 2018-12-09 | old | wave-effect picture fix; **origin of the im/pf split** (variant that doesn't change map-event update timing); faster common-event calls; locale/mojibake fix; ControlVariables / ConditionalBranch / CallEvent / Loop / BreakLoop extensions | ✅ |
| 190118–190122 | 2019-01 | old | AV-false-positive mitigation (section layout change); **Show String Picture (3007)** added; string-picture + save/load fixes | ✅ |
| 190217 | 2019-02-17 | old | **Get Picture Info (3008), Control Battle (3009), Control ATB Gauge (3010), Change Battle Command EX (3011)** added; battle "disable pre-battle flash"; common-event triggers **6=battle start, 7=battle parallel**; per-enemy normal-attack battle anim | ✅ |
| 190220 | 2019-02-20 | old | default battle type-B command-selection fixes | ✅ |
| 190526 | 2019-05-26 | old | **Get Battle Info (3012), Control Var Array (3013)** added; **pf per-frame command cap raised 10,000 → 200,000**; font loading (`Font/`); editor command-list editing; `accord.dll` Google-Drive flag fixed | ✅ |
| 190625 | 2019-06-25 | old | Show Picture default-arg behavior changes; speedups | ✅ |
| 190630 | 2019-06-30 | old | editor font config; map size lower bound 1×1; parallel-CE wait >2 s fix | ✅ |
| 190904 | 2019-09-04 | old | **Key Input Proc EX (3014), Rewrite Map (3015), Control Global Save (3016)** added; truecolor (16.77M) rendering; OTF fonts; ternary operand | ✅ |
| 190920 | 2019-09-20 | old | **ControlVariables "patch version" operand** (§4.3); ConditionalBranch "joypad present" | ✅ |
| 191007 | 2019-10-07 | old | editor cancel-button modes; numeric-form limits raised | ✅ |
| 191020 / 191021 | 2019-10-20/21 | old | **Change Picture ID (3017), Set Game Option (3018)** added; ConditionalBranch "window active"; Erase Picture range/all; default-battle crash fix | ✅ |
| 191103 | 2019-11-03 | old | **Call Command (3019)** added; Set Game Option frame-skip; Show/Move Picture origin + var-of-var; OGG battle-anim SE | ✅ |
| 200126 / 200128 | 2020-01-26/28 | old | **expression (`式`) operands for Control Variables** (target & operand); expression bugfixes | ✅ |
| 210414 | 2021-04-14 | old | editor-side prep for external command-creation integration (→ TPC). **EasyRPG's "legacy" reference ceiling** | ✅ |
| 210519 | 2021-05-19 | renewal | **"全内容更新" full rebuild.** Added **Control Strings/String Variables (3020), Get Game Info (3021), Edit Picture (3025), Output Picture (3026), Add Move Route (3027)**; editor↔TPC integration | ✅ |
| 210530 | 2021-05-30 | renewal | Get Game Info "executing event info"; ControlVariables **Lerp/Sum/Amin/Amax**; Var Array dereference + linked-array sort | ✅ |
| 210809 | 2021-08-09 | renewal | **Edit Picture (Tile) (3028)** added; partial EasyRPG extended Terms (ID 203–216); string-var file IO into `Text/`; Loop map-event enumeration; ConditionalBranch "map event exists" | ✅ |
| 211010 | 2021-10-10 | renewal | battle-anim playback control; regex on string vars; **independent X/Y picture scaling**; spritesheet range animation; ConditionalBranch "file output possible"; **(EN build) `Encoding=` ini key**; TPC Footy2 editor component | ✅ |
| **220205** | **2022-02-05** | (renewal-era) | **UNPUBLISHED Discord/dev build.** No public changelog. Between published 211010 and 241028 | ❌ |
| **220325** | **2022-03-25** | (renewal-era) | **UNPUBLISHED Discord/dev build — Beloved Rapture's runtime.** No public changelog; command surface is **RE-pinned** (§3.4). Between published 211010 and 241028 | ❌ |
| 241028 | 2024-10-28 | 64bit | **64-bit engine**; im/pf executables **merged** (ini `pf=1`); **asset encryption `RPG_RT.rs1`**; editor plugin system (`cmdcs.dll`); added **Control Message (3029), Script (3030/3031), Zoom Screen (3032), Console (3033)**; `\c[x,y]`/`\$[x,y]` 2-D color/EXFONT indexing; System2 scene control; folder relocation (`SaveData/`, `Map/`); mp3 streaming; vanilla-based patch form | ✅ |
| 241029 | 2024-10-29 | 64bit | JP initial-database fix only | ✅ |

Also on the uploader but not on the site: debug builds `patch_maniacs_190819_dbg`, `190820_dbg`,
`engine_190821/23/25_dbg`, and a save-error hotfix `稀に発生するセーブ時エラー対応.zip` (2019-08-11)
([uploader page 2](https://ux.getuploader.com/xingqier/index/date/desc/2)).

### 3.4 The unpublished 2022 dev builds (220205 / 220325) — why this matters

Three independent lines of evidence establish that **220205 and 220325 were never published** and
exist only as Discord/dev distributions between public 211010 and public 241028:

1. The Wayback snapshot of the official page dated **2022-11-16** still lists **211010** as the
   newest download, and its changelog ends 2021/10/10
   ([web.archive.org/web/20221116205904/…](https://web.archive.org/web/20221116205904/https://bingshan1024.github.io/steam2003_maniacs/)).
2. The site repo's git history has **no commits at all between 2021-10-10 and 2024-10-28**
   ([commit list](https://github.com/BingShan1024/steam2003_maniacs/commits/master)).
3. Community evidence shows new features circulating on the Discord during that window (jetrotal,
   2023-05-06: "Some time ago, I saw this on a discord Channel" with extended Display-Text /
   Show-Choices forms absent from 211010 — [#1818 comment](https://github.com/EasyRPG/Player/issues/1818)).

**The published changelog therefore has no entry for 220325.** The 2024/10/28 changelog
retroactively covers the entire 211010→241028 span as one cumulative list, so it cannot tell you
which features were present at 220325. The authoritative description of the **220325 command
surface is the reverse engineering** in
[`/home/john/re/br_dispatch_findings.md`](/home/john/re/br_dispatch_findings.md): it dispatches
**147 distinct command codes** (118 baseline, 26 Maniac, 3 not in liblcf). The Maniac codes
present are **3001–3022 and 3024–3029** — notably it **dispatches 3029** (Control Message) but
**does NOT dispatch 3023, 3030, 3031, or 3032** (liblcf's `Maniac_Zoom`=3032 is not in the 220325
surface). Per-code detail is tabulated in [maniac-patch-commands.md](maniac-patch-commands.md);
the dispatcher mechanics are summarized in §9 below.

Safe assumption for any 220325-class game: **everything documented through 211010 is present;
241028-list features may or may not be present and must be confirmed against the binary.**

---

## 4. Detection

### 4.1 Bundled DLLs

| File | Meaning |
|---|---|
| `accord.dll` | The Maniacs runtime helper. Historically the primary Maniacs marker (was flagged by Google Drive AV until the 190526 fix). **Not present in the newest builds** ([0.8.1 notes](https://blog.easyrpg.org/2025/04/easyrpg-player-0-8-1-stun/)) |
| `ultimate_rt_eb.dll` | Ships alongside but is the **stock English RM2k3 runtime DLL**, not Maniacs-specific (also keys EasyRPG's English-engine fallback, [src/player.cpp:823](/home/john/Player/src/player.cpp)) |

### 4.2 PE characteristics (the durable signal)

| Era | `logos` | `CODE` seg | `CHERRY` seg | `GEEP` seg | Internal version | EasyRPG `mp_version` |
|---|---|---|---|---|---|---|
| old, first builds | 1 | present | normal | **present** | 1.1.2.1 | 1 |
| old, later builds | 1 | present | **> 0x10000** | absent | 1.1.2.1 | 1 |
| renewal (incl. **220325**) | 1 | present | (per build) | absent | 1.1.2.1 | 1 |
| 64bit (241028+) | **0** | **absent** | **absent** | absent | 1.1.2.1 | 1 |

EasyRPG computes `mp_version = (geep_size > 0 || cherry_size > 0x10000) ? 1 : 0` only as a
**fallback** when the exact build number is unknown ([src/exe_reader.cpp:556-559](/home/john/Player/src/exe_reader.cpp)).
The 64-bit case is matched by `logos == 0 && version == 1.1.2.1 && code_size == 0 && cherry_size == 0`
([src/exe_reader.cpp:521-530](/home/john/Player/src/exe_reader.cpp)).

### 4.3 VERSIONINFO build-number string (the exact build)

Maniacs builds embed a UTF-16 VERSIONINFO `FileDescription`-style string of the form
**`RPG Maker 2003 Runtime (Maniacs, vNNNNNN, <lang>, <im|pf>)`**, e.g. Beloved Rapture's
`Maniacs, v220325, en, im`. EasyRPG parses the build number as follows
([src/exe_reader.cpp:389-398](/home/john/Player/src/exe_reader.cpp)):

1. After locating the `VS_FIXEDFILEINFO` block, search for the literal UTF-16 sequence
   **`"Maniacs, v"`** (`{'M',0,'a',0,'n',0,'i',0,'a',0,'c',0,'s',0,',',0,' ',0,'v',0}`).
2. Take the next **12 chars** (`version_length = 12`), strip embedded NULs, `atoi()` →
   `file_info.maniac_patch_version` (so `220325` → integer `220325`).

The result feeds `game_config.patch_maniac.Set(maniac_patch_version)` unless an explicit patch
flag set `patch_override` ([src/player.cpp:798-802](/home/john/Player/src/player.cpp)). The
`en`/`im` tail tokens are **not** parsed by EasyRPG; their meaning is documented in §8.1 and
they are runtime-detectable at game level via the ControlVariables "patch version" operand
(low 20 bits = `YYMMDD`, MSB = im(0)/pf(1); added in build 190920 — [official site, 変更コマンド
"changed commands"](https://bingshan1024.github.io/steam2003_maniacs/)).

### 4.4 EasyRPG mapping (flag, CLI, autodetect anchor)

| Aspect | Value / anchor |
|---|---|
| Config flag | `Game_ConfigGame::patch_maniac` — `ConfigParam<int> patch_maniac{ "Maniac Patch", "", "Patch", "Maniac", 0 }` ([src/game_config_game.h:45](/home/john/Player/src/game_config_game.h)) |
| Query helper | `Player::IsPatchManiac()` → `patch_maniac.Get() > 0` (also honors a temporary runtime override via command 2053 SetInterpreterFlag when `ENABLE_DYNAMIC_INTERPRETER_CONFIG`) ([src/player.h:519-526](/home/john/Player/src/player.h)) |
| CLI / ini | `--patch-maniac [N]` / `[Patch] Maniac=N`. **`N=1`** enable; **`N=2`** enable but **do not widen variable ranges to 32-bit** (for retro-fitting the patch onto an existing 2k/2k3 game) ([src/player.cpp:1503-1506](/home/john/Player/src/player.cpp)) |
| EXE autodetect | `src/player.cpp:795-802` (version-resource path) |
| DLL autodetect | `accord.dll` present **and** not already Maniac → `patch_maniac.Set(1)` ([src/player.cpp:853-855](/home/john/Player/src/player.cpp)) |
| Precedence | Any explicit patch flag sets `patch_override` ([src/game_config_game.h:81](/home/john/Player/src/game_config_game.h)), which disables **all** autodetection — including the version-resource and `accord.dll` paths (both guarded by `if (!game_config.patch_override)`). See [easyrpg-extensions.md](easyrpg-extensions.md). |

---

## 5. Event commands

The full **per-index** parameter layout (codes **3001–3033** plus the widened baseline commands
10220/11110/11120/11130/11610/12010/12210/12330 etc.) belongs in **maniac-patch-commands.md**, which
**is not yet written** (planned; §11). Until then this is the per-code reference: the table below
gives name, first-published build, and a one-line semantic from the official 追加コマンド ("added
commands") section, with the **liblcf enum name** and the **220325 RE handler address** where known.
"Since" = first *public* build per the official changelog; **dev builds (incl. 220325) may predate it**.
For the BR-relevant build, "✓220325" marks codes whose handler is dispatched by `FUN_004500d0`
(RE; §9), and the per-command parameter bitfields/op-selectors remain (RE-pending) until the
commands file lands.

Code-range eras: **3001–3019** are "old"-era; **3020–3028** renewal-era; **3029–3033** are 241028
additions (with **3029 already dispatched in the 220325 dev build**, §9).

| Code | Name (EN / JP) | Since | 220325 | Semantics (one-line) | liblcf enum / 220325 handler |
|---:|---|---|:--:|---|---|
| 3001 | Get Save Info / セーブ情報の取得 | ≤181209 | ✓ | Reads load-screen fields (date/level/HP) of slot N into vars; face → 4 pictures | `Maniac_GetSaveInfo` / `FUN_00436ce0` |
| 3002 | Save / セーブの実行 | ≤181209 | ✓ | Default-equivalent save to slot N; optional result var (1/0) | `Maniac_Save` / `FUN_00437390` |
| 3003 | Load / ロードの実行 | ≤181209 | ✓ | Default-equivalent load of slot N; optional pre-validate; 211010 fade-disable | `Maniac_Load` / `FUN_00437510` |
| 3004 | End Load Process / ロード処理の終了 | ≤181209 | ✓ | No-op (legacy; "現在このコマンドは意味を持ちません") | `Maniac_EndLoadProcess` / inline |
| 3005 | Get Mouse Position / マウス座標の取得 | ≤181209 | ✓ | Mouse pos → 2 vars, normalized to 320×240 | `Maniac_GetMousePosition` / `FUN_00437100` |
| 3006 | Set Mouse Position / マウス座標の設定 | ≤181209 | ✓ | Sets mouse pos (320×240 basis), clamped | `Maniac_SetMousePosition` / `FUN_00437240` |
| 3007 | Show String Picture / 文字列ピクチャの表示 | 190119 | ✓ | Generates a picture from a string (control chars, window/frame/gradient/shadow) | `Maniac_ShowStringPicture` / `FUN_00443b50` |
| 3008 | Get Picture Info / ピクチャ情報の取得 | 190217 | ✓ | Picture pos/size → vars (no rotation/effects) | `Maniac_GetPictureInfo` / `FUN_0044a850` |
| 3009 | Control Battle / 戦闘処理の制御 | 190217 | ✓ | Hook/override 5 default-battle processes (ATB, damage-pop, targeting, +state/+stat) | `Maniac_ControlBattle` / `FUN_00412f00` |
| 3010 | Control ATB Gauge / ATBゲージの操作 | 190217 | ✓ | Read/write ATB gauge (range 0–300000) | `Maniac_ControlAtbGauge` / `FUN_00412bf0` |
| 3011 | Change Battle Command EX / 戦闘コマンドの変更EX | 190217 | ✓ | Enable/disable Row; edit party commands | `Maniac_ChangeBattleCommandEx` / `FUN_00412b90` |
| 3012 | Get Battle Info / 戦闘情報の取得 | 190526 | ✓ | Reads in-battle values (mods/states/attrs/positions) | `Maniac_GetBattleInfo` / `FUN_00412860` |
| 3013 | Control Var Array / 変数配列の操作 | 190526 | ✓ | Array ops on consecutive vars (copy/swap/sort/shuffle/enumerate/deref/reverse) | `Maniac_ControlVarArray` / `FUN_0044bad0` |
| 3014 | Key Input Proc EX / キー入力の処理EX | 190904 | ✓ | Extended key/joypad input; 0/1 per key, never waits | `Maniac_KeyInputProcEx` / `FUN_0044c420` |
| 3015 | Rewrite Map / マップの書き換え | 190904 | ✓ | Rewrites tiles of current map (single/rect, layer, autotile); not saved | `Maniac_RewriteMap` / `FUN_0044d330` |
| 3016 | Control Global Save / 共有セーブの操作 | 190904 | ✓ | Cross-savegame store of switches+vars in `Save.lgs` (§8.6) | `Maniac_ControlGlobalSave` / `FUN_0044e420` |
| 3017 | Change Picture ID / ピクチャのID変更 | 191020 | ✓ | Move/exchange/slide ranges of picture IDs | `Maniac_ChangePictureId` / `FUN_0044e930` |
| 3018 | Set Game Option / ゲームのオプション設定 | 191020 | ✓ | Global runtime settings (inactive behavior, Fatal-Mix FPS/TestPlay/MsgSkip, pic-limit, frameskip) | `Maniac_SetGameOption` / `FUN_0044ecd0` |
| 3019 | Call Command / コマンドの呼び出し | 191103 | ✓ | Invoke an arbitrary event command code with arbitrary args | `Maniac_CallCommand` / inline |
| 3020 | Control Strings / 文字列変数の操作 | 210519 | ✓ | Third var type `t[n]`: assign/concat/to-num/length/search/extract/split/file-IO | `Maniac_ControlStrings` / `FUN_00426720` |
| 3021 | Get Game Info / ゲーム情報の取得 | 210519 | ✓ | Reads map size, tile IDs, window size, exec-event state, chipset/face/walk/camera | `Maniac_GetGameInfo` / `FUN_004146c0` |
| **3022** | *(expression statement)* | dev (≤220325) | ✓ | **Execute inline expression bytecode** (recursive VM `FUN_00445140`); **not in liblcf** | — / `FUN_00448a50` |
| 3023 | *(reserved/absent)* | — | ✗ | Not dispatched by 220325; not in liblcf (open Q, §11) | — |
| 3024 | *(recognized no-op)* | dev (≤220325) | ✓ | Explicit recognized no-op (returns true); **not in liblcf** | — / inline |
| 3025 | Edit Picture / ピクチャの編集 | 210519 | ✓ | Rewrites picture pixels in an arbitrary rect; persists in save (deflate) | `Maniac_EditPicture` / `FUN_0044af50` |
| 3026 | Output Picture (WritePicture) / 画像の出力 | 210519 | ✓ | Writes screen or a picture to a file in `Picture/` | `Maniac_WritePicture` / `FUN_0044b330` |
| 3027 | Add Move Route / キャラの動作追加 | 210519 | ✓ | Appends move-route actions; **dispatcher no-op** — consumed by MoveEvent (11330) lookahead | `Maniac_AddMoveRoute` / inline |
| 3028 | Edit Picture (Tile) / ピクチャの編集(チップ) | 210809 | ✓ | Draws map tiles into a picture, tile-by-tile | `Maniac_EditTile` / `FUN_0041cfb0` |
| 3029 | Control Message / 文章処理の制御 | 241028 (dev earlier) | ✓ | Hooks message-window events (user-event/create/destroy/char-draw) to a common event | `Maniac_ControlTextProcessing` / `FUN_0043e700` |
| 3030/3031 | Script / スクリプト | 241028 | ✗ | Executes JavaScript (bridges to switches/vars/string-vars only) | — (not dispatched in 220325) |
| 3032 | Zoom Screen / 画面のズーム | 241028 | ✗ | Zooms screen on a coordinate; **`Maniac_Zoom` in liblcf but NOT in 220325 surface** | `Maniac_Zoom` (liblcf) / not dispatched |
| 3033 | Console / コンソール | 241028 | ✗ | Opens/closes a console window for string IO; not in liblcf | — |

Notes: codes **3001–3022 and 3024–3029** are confirmed dispatched in 220325 (26 Maniac codes); **3023,
3030, 3031, 3032 are absent** (§9). Per-index parameter layouts, op-selector tables and the
ShowStringPicture 5-bit/arg-14 packing are (RE-pending) here and belong in the planned
`maniac-patch-commands.md`. The expression bytecode that backs 3022 and the expression operand modes
is fully specified in [maniac-patch-fileformats.md §9](maniac-patch-fileformats.md).

## 6. Modified baseline commands

Maniacs widens/extends many stock commands. Per-index detail belongs in the planned
`maniac-patch-commands.md` (§11); the summary below is from the official 変更コマンド ("changed
commands") section. Every one of these baseline codes is dispatched by 220325's `FUN_004500d0` (§9),
so the *extra parameters* are read from the same `parameters[]` array — the layout is (RE-pending).

| Code | Command | Maniacs additions (summary) |
|---:|---|---|
| 10220 | Control Variables / 変数の操作 | range target by variable; ops Or/And/Xor/Shl/Shr; operands actor/enemy ATB, date/time/frame, item/actor/event/enemy-by-var, party-by-index, math fns (Add…Atan2, Min/Max/Abs/Random), ternary, **patch-version operand** (§4.3), Lerp/Sum/Amin/Amax (210530), "EXP for next level" (210809), **expression operands** (§5/§9) |
| 11110 | Show Picture / ピクチャの表示 | angle effect; blend multiply/add/overlay; flip H/V; ID/pos/zoom/opacity by var-of-var; origin (原点); spritesheet range anim (211010); separate X/Y zoom (211010) |
| 11120 | Move Picture / ピクチャの移動 | same effect set; negative duration = \|value\| **frames** (normal = value×6 frames); duration by var-of-var |
| 11130 | Erase Picture / ピクチャの消去 | ID by var-of-var, variable range, or "all pictures" |
| 11610 | Key Input Processing / キー入力の処理 | mouse input (L/R/M click, wheel up/down); wheel only with "wait until pressed" |
| 12010 | Conditional Branch / 条件分岐 | switch/var by var-of-var; new checks: ロード直後 "just loaded", ジョイパッドの有無 "joypad present", ウィンドウがアクティブ "window active" (191020), マップイベントが存在 "map event exists" (210809), ファイル出力可能 "file output possible" (211010) |
| 12210 | Loop / ループ | typed loops: ∞, X-times, count up/down, While, Do-While; optional index var; map-event enumeration `@foreach` (210809) |
| 12220 | Break Loop / ループの中断 | fixed behavior inside nested loops |
| 12330 | Call Event / イベントの呼び出し | common event by variable and by var-of-var |
| 10710 | Battle Processing / 戦闘の開始 | option to disable the pre-battle flash (EasyRPG: still MISSING, §9) |

241028-era baseline extensions (Screen Scroll instant-execute, Erase Event restore/target,
Swap Event Position var targets, Face Graphics animation, Play BGM/Sound variable support) are
**not relevant to 220325** and are summarized in the dossier (`maniac_patch_web.md §4.1`).

## 7. File-format changes

New/changed LDB/LMU/LMT/LSD chunks (e.g. LDB `0x21 maniac_string_variables`, Enemy `0x0F`,
Terms `0xA1–0xA6`; SaveSystem `0x24 maniac_strings`, `0x88/0x89/0x8A/0x8B`, message-hook
`0x2D–0x46`; SavePicture `0x1C maniac_image_data`), the **`Save.lgs`** global-save file, the
**`RPG_RT.rs1`** encrypted asset pack (241028+), and the `RPG_RT.ini [RPG_RT]` keys are
documented in [maniac-patch-fileformats.md](maniac-patch-fileformats.md), with liblcf coverage
notes. One runtime-relevant fact repeated here for the lineage: **`RPG_RT.rs1` asset encryption is
241028-only** and therefore **irrelevant to a 220325 game**.

## 8. Runtime behavior changes

These are the engine-wide behaviors that are not expressible as a single command or chunk. A
faithful reimplementation must reproduce them.

### 8.1 im vs pf — event-page-condition refresh timing

The single most important runtime variant. From the official **im版とpf版 ("im edition and pf
edition")** section ([official site](https://bingshan1024.github.io/steam2003_maniacs/)):

| Variant | Tail token | Refresh model | Notes |
|---|---|---|---|
| **im** = 即時 ("Immediately (Default)") | `im` | Like classic RM200X: **every** switch/variable change immediately re-checks the page conditions of all events on the current map | Compatibility-focused. **Beloved Rapture is `im`.** |
| **pf** = 毎フレーム ("Per frame (Incompatible)") | `pf` | Page conditions checked **once per frame**, drastically lowering variable-op cost | Recommended for heavy custom systems; event page switching needs care — "putting a Wait 0.0 at the end of the event fixes problems in most cases" |

Until 241028 these were **two separate executables**; from 241028 they were merged and selected by
`pf=1` (vs absent/0) in `RPG_RT.ini [RPG_RT]`. EasyRPG note: the per-frame refresh optimization
the 0.8.1 engine performs ("determine which events depend on which variables… only refresh these")
is an internal performance measure, distinct from faithfully selecting im/pf semantics.

### 8.2 Execution limits

The per-event **per-frame command-execution cap** was raised from the stock **10,000** to
**200,000 in the pf edition** (since 190526). EasyRPG treats this as a general change (Ghabry on
[#1818](https://github.com/EasyRPG/Player/issues/1818)); exceeding it is what produces the
"Event N exceeded execution limit" black-screen failure (e.g. issue
[#1807 海賊ウォーク / "Pirate Walk"](https://github.com/EasyRPG/Player/issues/1807)).

### 8.3 Variable value range

Variables widen from the stock 2k3 ±9,999,999 to the **full signed int32**
(−2,147,483,648 … 2,147,483,647), still **saturating** (no wrap; "従来通りオーバーフロー/アンダー
フローはありません"). EasyRPG applies this in `Game_Constants::GetVariableLimits()` when Maniac is
active unless `--patch-maniac 2` is used: the guard `!Player::IsPatchManiac() || patch_maniac.Get()==2`
falls back to the stock 2k3 bounds, otherwise `min = std::numeric_limits<Var_t>::min()` /
`max = std::numeric_limits<Var_t>::max()`
([src/game_constants.cpp:24-44](/home/john/Player/src/game_constants.cpp); stock 2k3 bounds
`min_2k3 = -9'999'999`, `max_2k3 = 9'999'999` at [src/game_variables.h:39-40](/home/john/Player/src/game_variables.h)).

### 8.4 Custom resolution (Winw/Winh)

`Winw=` / `Winh=` in `RPG_RT.ini [RPG_RT]` set the window/logical resolution (since renewal build
210519); defaults are **320×240**. RE of 220325 confirms the reader (single `GetPrivateProfileIntA`
site in `FUN_004a3820`) clamps **Winw to 64..1920** and **Winh to 32..1440**
([`/home/john/re/br_dispatch_findings.md`](/home/john/re/br_dispatch_findings.md) §RPG_RT.ini).
Community guidance: best results when both exceed the default and are multiples of 16
([rpgmaker.net #24260 p5](https://rpgmaker.net/forums/topics/24260/?p=5)); games in the wild use
e.g. `Winw=426 / Winh=240` for 16:9. EasyRPG's equivalent is `--game-resolution`. (Beloved
Rapture's non-multiple-of-16 width triggered a map-scroll bug — issue
[#3128](https://github.com/EasyRPG/Player/issues/3128).)

### 8.5 Font loading (`Font/`)

A `Font/` folder in the project is scanned at startup and its **TTF** (and **OTF** since 190904)
files are registered (`AddFontResourceExA` / `AddFontMemResourceEx` in 220325); the fonts are then
usable in string pictures and message windows. The 220325 binary also carries embedded `RPG2000` /
`RPG2000G` FONTRES resources ([`/home/john/re/br_dispatch_findings.md`](/home/john/re/br_dispatch_findings.md)
§Referenced filenames). BR additionally appears to embed a custom font in its EXE, which is why
EasyRPG's text metrics differ (PR [#3293](https://github.com/EasyRPG/Player/pull/3293)).

### 8.6 Global save (`Save.lgs`)

Command **3016 Control Global Save** maintains a **cross-savegame shared store of switches and
variables only**, in a fixed-name file **`Save.lgs`** ("共有セーブ"; RE-confirmed filename
"LcfGlobalSave" string in 220325 — [`/home/john/re/br_dispatch_findings.md`](/home/john/re/br_dispatch_findings.md)
§Referenced filenames). EasyRPG implements it (`ManiacPatch::GlobalSave` at
[src/maniac_patch.h:52](/home/john/Player/src/maniac_patch.h), saved on shutdown at
[src/player.cpp:946](/home/john/Player/src/player.cpp); rewritten in PR
[#3435](https://github.com/EasyRPG/Player/pull/3435)). Since 241028 a `Save.lgs` inside the
optional `SaveData/` folder is also honored. Op semantics and byte layout: see
[maniac-patch-fileformats.md](maniac-patch-fileformats.md) and
[maniac-patch-commands.md](maniac-patch-commands.md).

### 8.7 Other engine-level behaviors

- **String variables (`t[n]`)** — a third variable type alongside switches/variables; values usable
  in control characters (`\T[n]`) and most DB term fields. File IO into a `Text/` subfolder
  (`\Text\` confirmed in 220325). See command **3020** in
  [maniac-patch-commands.md](maniac-patch-commands.md).
- **Expression VM (`式`)** — an integer-bytecode evaluator backing the expression operand modes of
  ControlVariables and other commands; in 220325 it is `FUN_00445140` (recursive opcode VM) and
  command **3022** is "execute inline expression bytecode"
  ([`/home/john/re/br_dispatch_findings.md`](/home/john/re/br_dispatch_findings.md) §UNKNOWN-TO-LIBLCF).
  EasyRPG: `ManiacPatch::ParseExpression/ParseExpressions` ([src/maniac_patch.h:31-32](/home/john/Player/src/maniac_patch.h)).
  Full opcode/grammar spec: [maniac-patch-commands.md](maniac-patch-commands.md).
- **Battle common-event triggers** — common-event `trigger` gains value **6 = battle start** and
  **7 = battle parallel** (vanilla 3=auto, 4=parallel, 5=call); confirmed by Ghabry on PR
  [#3545](https://github.com/EasyRPG/Player/pull/3545). These drive Maniacs custom battle systems.
- **Truecolor rendering, OGG (with VX-style `LOOPSTART`/`LOOPLENGTH`), mouse-advances-message,
  int32 default-battle damage cap** — all engine-wide (§7.1 of the dossier).
- **RPG_RT.ini keys in 220325** (RE-confirmed complete profile surface): `GameTitle`, `Winw`,
  `Winh`, `Encoding` (code page, EN-build text IO), `RuntimePackageKey` — **no** `FullPackageFlag`
  and **no** `Fps`/`pf` key in this build (`pf` is 241028+). The TestPlay/BattleTest/HideTitle/
  FullScreen switches are command-line, not ini ([`/home/john/re/br_dispatch_findings.md`](/home/john/re/br_dispatch_findings.md)).
- **Asset encryption (`RPG_RT.rs1`)** — **241028+ only**, irrelevant to 220325. See
  [maniac-patch-fileformats.md](maniac-patch-fileformats.md).

---

## 9. EasyRPG support matrix (feature level)

Per-command checkbox state lives in [maniac-patch-commands.md](maniac-patch-commands.md); the
table below is the **feature-level** rollup. Releases: there has been **no Player release since
0.8.1.1 (2025-06-02)**, so most 2026 Maniacs fixes are **master-only**
([blog.easyrpg.org](https://blog.easyrpg.org/)). Tracking metabug:
[#1818 "Support Maniac Patch (Metabug for all releases)"](https://github.com/EasyRPG/Player/issues/1818).

| Feature area | Status | Anchor / PR | Release-vs-master |
|---|---|---|---|
| **Legacy command set (≤v210414)** | **PARTIAL (near-complete)** | [#1818](https://github.com/EasyRPG/Player/issues/1818) | "almost everything of legacy v210414" by 0.8.1 (2025-04); residual gaps below mostly master/open |
| New commands 3001–3013 | FULL | per-command PRs (#2623/#2486/#2870/#2735/#3295/#3291/#3290/#3294/#2733) | mostly in 0.8.1 |
| Key Input Proc Ex (3014) | PARTIAL | #2797 | joypad-remap subfeatures **WONTFIX** (EasyRPG has own remapping); Shift/Ctrl/Alt bug [#3098](https://github.com/EasyRPG/Player/issues/3098) open |
| Rewrite Map (3015) | PARTIAL | #3306, open PR #3502 | A/B autotiles unsupported |
| Control Global Save (3016) | FULL | #2797, rewritten #3435 (2025-07) | rewrite master-only |
| Change Picture ID (3017) | MISSING (open PR) | open PR [#3500](https://github.com/EasyRPG/Player/pull/3500) | not merged |
| Set Game Option (3018) | PARTIAL | open PR [#3487](https://github.com/EasyRPG/Player/pull/3487) | picture-limit stubbed; "variable count" sub-op **does not exist** (PR #3544 rejected after asking Maniac devs) |
| Call Command (3019) | FULL (impl. difference) | #3140 | pushes a single-command frame rather than invoking directly |
| String Variables (3020) + expressions | FULL | #3051 (flagship 0.8.1), #3140 | |
| Get Game Info (3021) | PARTIAL | #3309/#3349 | |
| Edit/Output Picture, Add Move Route, Edit Tile (3025–3028) | FULL/PARTIAL | #3488 etc. | per [maniac-patch-commands.md](maniac-patch-commands.md) |
| Control Message / Script / Zoom / Console (3029–3033) | MOSTLY MISSING | Zoom open PR #3499 | 241028-era; **not needed for 220325** (220325 dispatches only 3029) |
| Modified baseline commands (pictures, ControlVars, CondBranch, Loop, KeyInput) | FULL | #2628/#2741/#2734/#2649/#2623 | "Just Loaded" cond. implemented in 0.8.1 despite stale issue note |
| **Battle / parallel common events** | **MISSING — main blocker** | open PR [#3545](https://github.com/EasyRPG/Player/pull/3545) | needs testers; "will never be merged" without reports. Blocks BR combat |
| Battle Processing flash-disable | MISSING | #1818 checklist | |
| Scoped / self variables | MISSING | rejected PR #3546 (semantics = per-event) | unimplemented |
| im/pf refresh semantics | INTERNAL (perf only) | 0.8.1 notes; #1818 | no explicit im/pf selection; engine has its own refresh optimizer |
| 32-bit variable widening | FULL | [src/game_constants.cpp:28-41](/home/john/Player/src/game_constants.cpp) | `--patch-maniac 2` opts out |
| Custom save/db chunks | MOSTLY FULL | liblcf `fields_easyrpg.csv` | faceset-anim + text-settings chunks pending ([liblcf #493](https://github.com/EasyRPG/liblcf/issues/493)); `maniac_frameskip` WONTFIX |
| 64-bit "rewrite" builds (241028+) beyond legacy | DETECT-ONLY | [src/exe_reader.cpp:521-530](/home/john/Player/src/exe_reader.cpp) | only the EXE is recognized; new-only features not tracked |

**Reference target.** EasyRPG's implementation tracks the **published 210414 / 211010** "legacy"
builds. **220325 is between published builds**; everything BR uses beyond 211010 (extra GetGameInfo
items, message/choice extensions, etc.) has no public changelog. **The authoritative command
surface for 220325 is the RE in [`/home/john/re/`](/home/john/re/br_dispatch_findings.md):**

- Single unified dispatcher **`FUN_004500d0`** (handles both map and battle interpreters).
- Selector hash `sel = (code & 0x3FF) - 8*((code >> 8) & 0xFF) + 0x19A` (valid iff `sel <= 0x573`),
  routed through a range-split + five two-level jump tables; unknown codes are **silently skipped**.
- **147 codes dispatched** (118 baseline, 26 Maniac, 3 not-in-liblcf). Maniac codes present:
  **3001–3022, 3024–3029**. **Absent: 3023, 3030, 3031, 3032** (liblcf `Maniac_Zoom`=3032 is
  NOT part of the 220325 surface). `3027 Add Move Route` is a dispatcher no-op consumed by the
  MoveEvent (11330) handler's lookahead; `3024` is an explicit recognized no-op; code `0` aliases
  EndEventProcessing.

---

## 10. Test assets

- **Beloved Rapture** — the primary 220325 / `en` / `im` target; commercial game
  ([`../games/beloved-rapture.md`](../games/beloved-rapture.md)). Stresses: custom menus
  (GetSaveInfo facesets, Set Game Option picture limit), expression operands, string variables,
  non-multiple-of-16 resolution, and the **battle/parallel common-event** system (the remaining
  blocker). EasyRPG history: issue [#3128](https://github.com/EasyRPG/Player/issues/3128), PR
  [#3293](https://github.com/EasyRPG/Player/pull/3293).
- **海賊ウォーク (Kaizoku Wōku / "Pirate Walk")** — JP freeware; long-standing
  execution-limit black-screen metabug [#1807](https://github.com/EasyRPG/Player/issues/1807).
- Broader Maniacs corpus: spirit's Place ([#3439](https://github.com/EasyRPG/Player/issues/3439),
  save-format incompatibility), "OFF Forgotten Dreams" ([#3459](https://github.com/EasyRPG/Player/issues/3459),
  file-condition features). Test-game corpus lives at `c:\rg\easyrpg_library`.

## 11. Open questions

0. **Companion file `maniac-patch-commands.md` is unwritten.** This overview, the README, the
   fileformats spec, `dynrpg.md`, and `bootlegs-translations.md` all forward-reference it for the
   per-index command/op-selector layout. §5/§6 here carry the per-code summary as a stopgap; the
   full bitfield-level parameter spec (incl. ShowStringPicture's arg-14 packing and 5-bit
   font/system-name encoding documented by Ghabry on [#1818](https://github.com/EasyRPG/Player/issues/1818))
   still needs to be authored. **(file-pending)**
1. **Exact 220325 feature delta vs 211010** — beyond the dispatched command set, which
   *parameter* extensions (message-window options, extra GetGameInfo items, choice extensions)
   exist? Confirm against the binary / Discord history. (Command-level: see
   [maniac-patch-commands.md](maniac-patch-commands.md).)
2. **Command codes 3022–3024 semantics** — 3022 is the expression-statement command (RE-mapped),
   3024 a recognized no-op; whether 3023 is reserved/removed is open
   ([`/home/john/re/br_dispatch_findings.md`](/home/john/re/br_dispatch_findings.md) §Open questions).
3. **`misc(type)` builtin type-value table** for the expression VM — explicitly noted by BingShan
   as NOT matching the UI operand order; needs RE or the bundled `vexpr.txt`.
4. **Save round-trip incompatibility** — EasyRPG stores the picture overlay flag in a different
   place than Maniacs RPG_RT ([#3439](https://github.com/EasyRPG/Player/issues/3439)); save
   timestamp timezone bug ([liblcf #500](https://github.com/EasyRPG/liblcf/issues/500)).
5. **`Save.lgs` byte format** — implemented in EasyRPG but undocumented upstream; derive from
   Player/liblcf (tracked in [maniac-patch-fileformats.md](maniac-patch-fileformats.md)).

## 12. References

Primary:
- **Official site (BingShan)** — <https://bingshan1024.github.io/steam2003_maniacs/> (sections:
  トップ/ダウンロード/im版とpf版/制御文字の拡張/素材暗号化/エディタプラグイン/その他追加機能/追加コマンド/変更コマンド/更新履歴/作者/リンク).
  Repo: <https://github.com/BingShan1024/steam2003_maniacs> (incl. `vexpr.txt`, `vexpr_bytecode.txt`).
- **Reverse engineering of 220325** — [`/home/john/re/br_dispatch_findings.md`](/home/john/re/br_dispatch_findings.md)
  (dispatcher `FUN_004500d0`, 147 codes, ini surface, filenames).
- **EasyRPG source** — `src/exe_reader.cpp`, `src/player.cpp`, `src/game_constants.cpp`,
  `src/game_variables.h`, `src/maniac_patch.{h,cpp}`, `src/game_config_game.h` (this tree).
- **EasyRPG metabug** — [Player #1818](https://github.com/EasyRPG/Player/issues/1818); liblcf
  Maniac chunks via `generator/csv/fields_easyrpg.csv` (PRs #271/#417/#434/#445/#473/#482/#499).

Secondary / community (archive links given where pages may rot):
- Wayback 2022-11-16 official page (proves 220325 unpublished):
  [web.archive.org/web/20221116205904/…](https://web.archive.org/web/20221116205904/https://bingshan1024.github.io/steam2003_maniacs/).
- rpgmaker.net thread #24260 (EN update notes, Winw/Winh): <https://rpgmaker.net/forums/topics/24260/>.
- RMteka "Maniacs Patch Won't Bite You" #1–#4: <https://www.rmteka.pl/maniacs-patch-wont-bite-you-1-basic-feature-changes/> (and /2/, /3/, /4/).
- jetrotal CSA (TPC command reference, `.EDB` round-trip): <https://jetrotal.github.io/CSA/> / <https://github.com/jetrotal/CSA>.
- VIPRPG@総合制作技術Wiki (S2k3MP, plugins, TPC): <https://wikiwiki.jp/viprpg-dev/2003/%E6%8B%A1%E5%BC%B5%E3%83%91%E3%83%83%E3%83%81>.
- Sibling specs: [maniac-patch-commands.md](maniac-patch-commands.md),
  [maniac-patch-fileformats.md](maniac-patch-fileformats.md),
  [../games/beloved-rapture.md](../games/beloved-rapture.md),
  [easyrpg-extensions.md](easyrpg-extensions.md).

---

### Appendix A — TPC / editor-plugin ecosystem

Maniacs is surrounded by a tooling ecosystem that a port must be *aware of* (games are authored
with it) but need not reimplement:

- **TPC** — BingShan's official **text-to-event compiler**: feed a source file, it emits RM2k3 +
  Maniacs event commands and common events ("direct programming through the TPC system",
  [EasyRPG known-patches wiki](https://wiki.easyrpg.org/development/technical-details/known-patches)).
  Distributed separately on the uploader (`tpc_210519` … `tpc_241028`) because it edits projects
  directly, is obfuscated, and triggers AV false positives ("再配布して使うものでもない" — not meant
  for redistribution). Editor integration since 210519 (F3/F4/F5 text-edit hooks; Footy2 component
  since 211010). Grammar: `@command` + `.subcommand`, `v[]/s[]/t[]`, backtick expressions,
  `def/defs/defv/deft` metaprogramming, `cev/mev/mep` common/map-event definitions
  ([CSA info.md](https://github.com/jetrotal/CSA/blob/main/info.md)).
- **CSA (Cold Spaghetti Analyzer)** — jetrotal's community companion: web docs of the whole TPC
  command set, reads commands from an XML `.EDB` dump, supports `//TPC_snippet` round-tripping
  (<https://jetrotal.github.io/CSA/>).
- **`cmdcs.dll` editor plugin API (241028+)** — `cmdcs.dll` next to the editor + a `Plugin/`
  folder; plugins are **C# + WinForms (.NET Framework 4.8.1)** that can override event-command edit
  forms and add menus; some built-in command forms are themselves plugins; config in
  `Plugin\cmdcs.xml`. Community plugin index on the
  [VIPRPG plugin page](https://wikiwiki.jp/viprpg-dev/2003/%E6%8B%A1%E5%BC%B5%E3%83%91%E3%83%83%E3%83%81/%E3%83%97%E3%83%A9%E3%82%B0%E3%82%A4%E3%83%B3).
  **Editor-only — does not affect runtime/game-file compatibility.**

Note on file artifacts seen next to Maniacs games: `RPG_RT.edb` is **not** a Maniacs file — it is
EasyRPG's `lcf2xml` XML dump of `RPG_RT.ldb` (consumed by tools like CSA/TPC), inert at runtime
(the Player only auto-loads an XML DB named `EASY_RT.edb`). `.r3proj` is the stock Steam RM2k3
project file. See [maniac-patch-fileformats.md](maniac-patch-fileformats.md) and
[../games/beloved-rapture.md](../games/beloved-rapture.md) for the per-game file inventory.
