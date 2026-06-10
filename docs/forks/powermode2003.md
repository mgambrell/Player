<!--
docs/forks/powermode2003.md — part of the RPG_RT fork-support spec series.
Follows docs/forks/TEMPLATE.md section order.
Evidence: EasyRPG Player source (our tree, branch RISKY), Makerpendium wiki, EasyRPG GitHub.
Anchors of the form src/foo.cpp:NN are into /home/john/Player.
-->

# PowerMode 2003 (パワーモード2003 / "Mega Patch 2003")

> PowerMode 2003 is a binary fork of the RPG Maker 2003 runtime (`RPG_RT.exe` v1.09) made by
> the Russian author **Firesta**, with the bundled audio DLL `hvm.dll` by **Ripp3er** and an
> Italian localization distributed as **"Mega Patch 2003"** by the translator **Calev**. It
> reserves the first eight in-game variables, `V[1]`–`V[8]`, as **control registers**: writing
> to one of them triggers an engine-side action (open the load menu, quit, read the mouse,
> poll the keyboard, do floating-point math, or rotate pictures) and the result is written back
> into those same variables. It also replaces the audio backend (FMOD/BASS via `warp.dll`,
> adding AAC+/MP4 and dropping WMA) and skips the publisher logo and title screen. A game uses
> it to get mouse/keyboard input, trig/sqrt math, and per-picture rotation that stock
> RM2k3 1.09 cannot do.

**Status at a glance:** EasyRPG support = **PARTIAL** (near feature-complete; AAC/`warp.dll`
audio not implemented). Detection = **`warp.dll` in the game directory**
(`src/player.cpp:861`). Spec confidence = **MEDIUM** — the EasyRPG re-implementation
(`src/game_runtime_patches.cpp`, PR [#3384](https://github.com/EasyRPG/Player/pull/3384)) is
authoritative for the control-register behavior and is fully cited below; several upstream
*facts* (exact CR0 opcodes vs. the patch's own docs, the V[5]/V[6] trig target ordering, the
audio-format list, the build dates) are sourced from the Makerpendium wiki and are marked
(UNVERIFIED) where not corroborated by RE of the original binary.

## 1. Identity

| Field | Value |
|---|---|
| Names | **PowerMode 2003**; Italian alias **"Mega Patch 2003"**; sometimes "Power Mode" |
| Embedded EXE string | `powermode 2k3` (used as a candidate detection signature, see §4) |
| Author | **Firesta** (Russian RPG Maker scene) |
| Audio DLL author | **Ripp3er** (`hvm.dll`; shipped as / alongside `warp.dll`) |
| Italian translation | **Calev** ("Mega Patch 2003") |
| Target engine | RPG Maker 2003, `RPG_RT.exe` **v1.09** (internal `1.0.9.1`) — see §2 |
| License / EULA | None published; binary modification of `RPG_RT.exe` violated the Tkool EULA (as with all such forks). No redistribution terms known. (UNVERIFIED) |
| Liveness | **Dead.** Original distribution site archived ~June 2008; no maintained release. |
| Primary doc | Makerpendium wiki: <https://www.makerpendium.de/wiki/PowerMode_2003> |

Authorship, the `hvm.dll`/Ripp3er and Calev/"Mega Patch 2003" attributions, and the v1.09
target are all from the Makerpendium page above and the header doc comment at
`src/game_runtime_patches.h:241`–256 (line 243: `*  (aka. "Mega Patch 2003" in Italian scene)`).
The `--patch-powermode` help text names the author directly: "Enable PowerMode 2003 Patch by
Firesta." (`src/player.cpp:1511`). The `Game_ConfigGame` flag's display name is literally
`"Power Mode 2003"` (`src/game_config_game.h:75`).

## 2. Target engine builds

PowerMode 2003 patches **RPG Maker 2003 `RPG_RT.exe` version 1.09** (the final Japanese 2k3
build, internal version resource `1.0.9.1`; see [official-versions.md](official-versions.md)).
It is delivered as a **patched `RPG_RT.exe`** (byte patch + bundled DLLs), not as a wholesale
replacement runtime distributed independently of a base install. (UNVERIFIED whether the
distribution shipped a pre-patched EXE or a patcher tool; the Makerpendium page describes a
"patch", which in this ecosystem is usually an IPS/patcher against a specific build.)

Because the base is 1.09, the host engine is the classic (non-Steam) 2k3 battle/picture model:
50 pictures, the "bottom transparency" picture gradient feature, and the legacy event-command
set. PowerMode's picture-rotation register only addresses **picture IDs 1–50**
(`src/game_runtime_patches.cpp:526`), consistent with the 1.09 50-picture limit.

EasyRPG does not gate `patch_powermode` on engine version: the flag is a plain
`ConfigParam<int>` (`src/game_config_game.h:75`) and is auto-enabled purely on `warp.dll`
presence (§4). It only interacts with the engine version through the picture "bottom
transparency" feature, which PowerMode repurposes (§6) and which EasyRPG disables when the flag
is set (`src/sprite_picture.cpp:34`).

## 3. Version lineage

Build history per the Makerpendium wiki (<https://www.makerpendium.de/wiki/PowerMode_2003>);
dates and labels (UNVERIFIED) — no RE of the original binaries has confirmed them.

| Build | Date | Headline changes | Published |
|---|---|---|---|
| Preview | autumn 2006 | First public preview | Yes (preview) |
| WIP-20070114 | 2007-01-14 | Work-in-progress build | Yes (WIP) |
| **v0.02** | 2007-06-17 | Last known build; the version most games shipped with | Yes |
| (site archive) | ~2008-06 | Original site captured by archive.org; no later builds | — |

There is no version-resource string carrying a PowerMode build number (unlike Maniac Patch's
`Maniacs, vNNNNNN`), so EasyRPG cannot distinguish PowerMode builds; it treats them uniformly.
(RE-pending: confirm whether `warp.dll`/`hvm.dll` carry a usable version string.)

## 4. Detection

How to recognize a PowerMode 2003 game from its files, without running it.

| Signal | Meaning | EasyRPG use |
|---|---|---|
| **`warp.dll`** in game dir | PowerMode's audio backend (FMOD→BASS wrapper; the bundled audio DLL, related to Ripp3er's `hvm.dll`) | **Primary autodetect.** `src/player.cpp:861` → `game_config.patch_powermode.Set(true)` |
| EXE byte signature `powermode 2k3` | ASCII string embedded in the patched `RPG_RT.exe` | Proposed but **not implemented** in EasyRPG; recorded in issue [#1182](https://github.com/EasyRPG/Player/issues/1182) |
| Logo/title skip behavior | Game boots straight past publisher logo + title (runtime, not a file signature) | Not used for detection |

**Bundled DLLs.** `warp.dll` is the audio replacement (the Makerpendium page describes the
audio backend migrating FMOD→BASS and adding AAC+/MP4 while dropping WMA). `hvm.dll`
(Ripp3er) is credited as the audio component; the relationship between `hvm.dll` and the
shipped `warp.dll` is (UNVERIFIED) — EasyRPG keys only on `warp.dll`.

**EasyRPG mapping.**

| Item | Value | Anchor |
|---|---|---|
| `Game_ConfigGame` flag | `patch_powermode` (`ConfigParam<int>`, default 0) | `src/game_config_game.h:75` |
| INI key | `[Patch] PowerMode2003` | `src/game_config_game.h:75`, `src/game_config_game.cpp:255` |
| CLI flag | `--patch-powermode` / `--no-patch-powermode` | `src/game_config_game.cpp:170`, help `src/player.cpp:1511` |
| Autodetect anchor | `warp.dll` present → set flag | `src/player.cpp:861` |

**Precedence.** As with every patch flag, setting `--patch-powermode` (or the INI key, or
`--no-patch`) marks `patch_override = true`, which disables `warp.dll`-based autodetection for
the run (CLI: `src/game_config_game.cpp:170`–173 sets the flag and `patch_override` at `:172`;
INI: `:255`–257 sets it at `:256`; `--no-patch` locks the flag off via `patch_powermode.Lock(0)`
at `src/game_config_game.cpp:100`). See [easyrpg-extensions.md](easyrpg-extensions.md) for the
shared override mechanism.

**Reporting gap.** Unlike most patches, `patch_powermode` is **not** listed in
`Game_ConfigGame::PrintActivePatches()` (`src/game_config_game.cpp:264`–) nor in
`RuntimePatches::DetermineActivePatches()` (`src/game_runtime_patches.cpp:151`), so an active
PowerMode patch is silently omitted from the startup "Patch configuration:" debug line. This is
a cosmetic logging omission, not a functional one. (Verified: no `patch_powermode` reference in
either function.)

## 5. Event commands

**PowerMode 2003 adds no new event-command codes.** Unlike Maniac Patch (3001–3033) or
EasyRPG's own 2001–2058, it has no command-dispatch entries; the `default:` arms of the three
interpreter switches are untouched by it (see [README](README.md) "Command dispatch"). All of
its functionality is driven by **writes to variables `V[1]`–`V[8]`** ("control registers"),
processed by a variable-change hook rather than by event commands.

This is the heart of the spec; §8.1 below details each register. Three entry points feed the
hook, all gated on `patch_powermode` (and marked `EP_UNLIKELY` so the check is near-free when
off): single-var `RuntimePatches::OnVariableChanged(int)` (`src/game_runtime_patches.cpp:214`),
the initializer-list overload (`:220`), and `OnVariableRangeChanged(start,end)` (`:228`, which
fans out to every id in the range). Each calls
`PowerMode2003::HandleVariableHooks(var_id)` (`src/game_runtime_patches.cpp:492`), which switches
on the **variable id** and calls the matching register handler:

| Written var | Handler | Source |
|---:|---|---|
| `V[1]` (CR0) | `HandleCommands()` | `src/game_runtime_patches.cpp:494,378` |
| `V[3]` (MCOORDY) | `HandleMouse()` | `src/game_runtime_patches.cpp:497,388` |
| `V[4]` (KEY) | `HandleKeyboard()` | `src/game_runtime_patches.cpp:501,399` |
| `V[7]` (FCODE) | `HandleFloatComputation()` | `src/game_runtime_patches.cpp:504,447` |
| any other | no-op (`default: return;`) | `src/game_runtime_patches.cpp:509` |

Note the trigger asymmetry: mouse reads fire on a write to **`V[3]` (Y)**, not `V[2]`; float
math fires on a write to **`V[7]` (the op code)**, after the operands `V[5]`/`V[6]` are already
set. A game therefore sets operands first and writes the op-code register last to execute. The
picture-rotation register `V[8]` is **not** handled here at all — it is read passively each
frame during picture animation (§6).

## 6. Modified baseline commands

PowerMode 2003 does not add parameters to existing event commands. It changes the **runtime
semantics** of two baseline picture features. They are documented here because they alter how a
stock command's data is interpreted.

### 6.1 Show/Move Picture — "bottom transparency" repurposed as rotation amount

Stock RM2k3 (non-English/Steam) pictures support a **bottom-transparency gradient**: a picture
can have a different transparency at its bottom edge than its top, producing a vertical fade.
EasyRPG models this as the `feature_bottom_trans` path
(`src/sprite_picture.cpp:34,157,161,163`).

When `patch_powermode` is active **and** the global picture-rotation register `V[8]` is in its
"off" state (value ≤ 10, see §8.1), PowerMode **reuses the picture's "finish bottom
transparency" field as a per-picture counter-clockwise rotation amount**, applied to any
picture that is in the standard rotation effect mode:

- EasyRPG disables the bottom-transparency gradient entirely while the patch is on:
  `feature_bottom_trans = IsRPG2k3() && !IsRPG2k3E() && !patch_powermode.Get()`
  (`src/sprite_picture.cpp:34`). With the patch active the gradient is suppressed so the field
  can be reused.
- Per animation frame, for a picture with `effect_mode == Effect_rotation` (the liblcf enum
  `SavePicture::Effect_rotation == 1`), `ApplyPictureRotation`
  (`src/game_runtime_patches.cpp:514`) is consulted *before* the normal rotation step
  (`src/game_pictures.cpp:568`–571). In the V[8]-off case (`var_start_pict_rotations <= 10`,
  `:517`): if the picture's `finish_bot_trans` (an `int32_t` field, reinterpreted via
  `static_cast<int8_t>`) **≥ 50**, the function returns `true` and the rotation advances
  **counter-clockwise**: `current_rotation -= current_effect_power`
  (`src/game_runtime_patches.cpp:520`–522). Otherwise it returns `false` and the engine falls
  through to its normal clockwise `current_rotation += current_effect_power`
  (`src/game_pictures.cpp:570`).

So a single bit of the bottom-transparency byte (≥ 50 vs < 50) selects rotation **direction**;
the rotation **speed** is still the picture's normal effect power. This is faithful replication
of the patch's bottom-transparency hijack. (UNVERIFIED: the exact original threshold; EasyRPG
uses `>= 50` on the signed reinterpretation, with a code comment that the value "is reused to
specify a counter-clock-wise rotation".)

### 6.2 Show/Move Picture — picture IDs 1–50 driven by the V[8] rotation table

When `V[8]` ("SPECIAL") holds a value **> 10**, it is treated as the **base variable id of a
rotation table**, and each picture `1..50` takes its absolute rotation directly from a
per-picture variable:

```
current_rotation = V[ V[8] + (pictureID - 1) ]      // for pictureID in 1..50
```

(`src/game_runtime_patches.cpp:526`–529). This overrides the picture's own rotation
accumulation entirely (the function returns `true`, so `game_pictures.cpp` skips its normal
`+= effect_power` step). This is how a PowerMode game animates up to 50 pictures' angles from a
contiguous block of variables — e.g. set `V[8] = 100` and pictures 1..50 read their angle from
`V[100..149]`.

## 7. File-format changes

**None known.** PowerMode 2003 introduces no new LDB/LMU/LMT/LSD chunks, no standalone data
files, and no new INI keys on the *game* side. All state lives in ordinary variables
(`V[1]`–`V[8]` and, for the V[8] table, an arbitrary block of variables) and ordinary picture
fields (bottom-transparency reused for rotation, §6.1).

liblcf coverage is therefore **not applicable**: there is nothing PowerMode-specific to model.
The picture rotation rides on the standard `SavePicture` fields already in the model
(`current_rotation`, `current_effect_power`, `finish_bot_trans`); see
`lib/liblcf` `SavePicture`. The one `[Patch]` key, `PowerMode2003`, is an **EasyRPG** INI key
in `EasyRPG.ini` (not in `RPG_RT.ini`), per `src/game_config_game.h:75`.

(RE-pending: confirm no PowerMode build embeds anything in `RPG_RT.ini`/`Save*.lsd` — none is
known, and EasyRPG models none.)

## 8. Runtime behavior changes

### 8.1 Control registers V[1]–V[8] (the core mechanism)

PowerMode reserves the first eight variables. The constants are defined at
`src/game_runtime_patches.h:258`–265. **A game must avoid using V[1]–V[8] for general
storage** when this patch is active.

| Var | Const | Role | Trigger | Behavior (EasyRPG) | Anchor |
|---:|---|---|---|---|---|
| `V[1]` | `PM_VAR_CR0` | Command register 0: load / quit / save-exists | write to V[1] | See CR0 table below | `:258`, handler `:378` |
| `V[2]` | `PM_VAR_MCOORDX` | Mouse cursor X (output) | written *by* engine on V[3] write | set to `mouse.x` | `:259`, `:394` |
| `V[3]` | `PM_VAR_MCOORDY` | Mouse cursor Y (output); **also the read trigger** | write to V[3] | reads mouse, sets V[2]=x, V[3]=y | `:260`, `:388,497` |
| `V[4]` | `PM_VAR_KEY` | Full-keyboard VK input (in/out) | write to V[4] | See keyboard semantics below | `:261`, `:399` |
| `V[5]` | `PM_VAR_FVALUE1` | Float operand / result A | (read on V[7] write) | input1 and a result slot | `:262`, `:447` |
| `V[6]` | `PM_VAR_FVALUE2` | Float operand / result B | (read on V[7] write) | input2 and a result slot | `:263`, `:447` |
| `V[7]` | `PM_VAR_FCODE` | Float op-code; **the math trigger** | write to V[7] | runs sin/cos / tan / sqrt / div | `:264`, `:447,504` |
| `V[8]` | `PM_VAR_SPECIAL` | Picture-rotation control | read per frame (not a write hook) | rotation table / direction (§6) | `:265`, `:514` |

#### CR0 — `V[1]` command register (`HandleCommands`, `src/game_runtime_patches.cpp:378`)

On any write to `V[1]`, EasyRPG reads the value and acts, then **rewrites V[1] to a
save-exists flag**:

| `V[1]` value written | Action | Anchor |
|---:|---|---|
| **255** | If a savegame exists, open the **Load menu** (`Scene_Load`) | `:380` |
| **254** | **Quit** the game (`Player::exit_flag = true`) | `:382` |
| any other | no action | — |
| *(always, after handling)* | `V[1]` is set to **1 if a savegame exists, else 0** | `:385` |

So `V[1]` doubles as a **savefile-existence probe**: after writing anything to it (or at game
start / after load, see below), reading `V[1]` yields 1 iff a save exists.

(UNVERIFIED — *important discrepancy*: the Makerpendium/`game_runtime_patches.h:250` prose
describes CR0 as "Calling up the Load menu, Exiting the game & Checking for the existence of
Savefiles", but does **not** publish the literal opcodes. EasyRPG implements **255 = load,
254 = quit**. The original patch's exact trigger values are RE-pending; the EasyRPG values are
what the re-implementation uses and should be treated as the reference until the binary is
disassembled.)

At new-game and after loading a save, EasyRPG seeds `V[1]` to the save-exists flag and **forces
the title screen to be skipped** (`new_game.Set(true)`) — this is PowerMode's "logo + title
skip" behavior:

- `OnResetGameObjects` (`src/game_runtime_patches.cpp:195`–206): sets `new_game = true`, seeds
  `V[1] = HasSavegame() ? 1 : 0`, and enables PowerMode's mouse button bindings (left =
  decision, right = cancel).
- `OnLoadSavegame` (`:208`–212): re-seeds `V[1]` to the save-exists flag.

#### Mouse — `V[2]`/`V[3]` (`HandleMouse`, `src/game_runtime_patches.cpp:388`)

Writing `V[3]` causes EasyRPG to read the current mouse position and write `V[2] = mouse.x`,
`V[3] = mouse.y` (`:393`–395). On platforms without mouse/touch support it warns and does
nothing (`:389`–391). After the write, the X register is flagged for event-refresh
(`Game_Map::SetNeedRefreshForVarChange(PM_VAR_MCOORDX)`, `:499`). Additionally, while the patch
is active, **mouse buttons are bound** as input: left → decision, right → cancel
(`src/game_runtime_patches.cpp:202`–204). (The original patch also exposes mouse buttons; the
left/right mapping here is EasyRPG's reproduction.)

#### Keyboard — `V[4]` (`HandleKeyboard`, `src/game_runtime_patches.cpp:399`)

`V[4]` polls the **entire keyboard** using Windows **virtual-key codes** (the VK↔InputKey map
is `RuntimePatches::VirtualKeys::VirtualKeyToInputKey` / `InputKeyToVirtualKey`,
`src/game_runtime_patches.h:273`/`:377`, covering mouse buttons 0x1–0x6, editing/navigation
keys, 0–9, A–Z, numpad, F-keys, modifiers, etc.):

- **Scan mode** (`V[4] == 0`): EasyRPG scans for any pressed key and writes its VK code back
  into `V[4]` (`:405`–422). Modifier precedence is explicit: Shift, then Ctrl, then Alt, then a
  full scan of `Input::Keys` returning the lowest matching VK code. L/R modifier variants both
  collapse to the generic VK (`0x10`/`0x11`/`0x12`).
- **Probe mode** (`1 ≤ V[4] ≤ 255`): the value is treated as a VK code to *test*; if that key
  is **not** currently pressed, `V[4]` is reset to 0 (`:423`–441). Unsupported codes log a
  debug message (`:427`). Modifier keys test both L and R physical variants.
- Any other value resets `V[4]` to 0 (`:442`–444).

On platforms without keyboard support it warns and returns (`:400`–402).

#### Floating-point math — `V[5]`/`V[6]`/`V[7]` (`HandleFloatComputation`, `:447`)

Floats are emulated in fixed point: results are scaled by **1,000,000** before storing in the
integer variables (so a fractional result of 0.5 is stored as 500000). Inputs are read as plain
integers and cast to `float`. Writing the **op-code** to `V[7]` triggers the computation:

| `V[7]` | Operation | Reads | Writes (×1,000,000 unless noted) | Anchor |
|---:|---|---|---|---|
| **1** | sin & cos of `V[5]` **in degrees** | `V[5]`=angle° | `V[6] = sin(°)`, `V[5] = cos(°)` | `:455`–461 |
| **2** | tan of `V[5]` in degrees | `V[5]`=angle° | `V[6] = tan(°)` | `:463`–467 |
| **3** | sqrt of `V[5]` | `V[5]` | `V[5] = floor(sqrt)` (integer part, no scale), `V[6] = frac × 1e6` | `:469`–475 |
| **4** | divide `V[5] / V[6]` | `V[5]`, `V[6]` | `V[5] = floor(quotient)` (no scale), `V[6] = frac × 1e6` | `:477`–485 |
| other | no-op | — | — | `:486` |

After the op, both result registers are flagged for event-refresh
(`SetNeedRefreshForVarChange(V[5])`, `V[6]`; `:506`–507). Angles for sin/cos/tan are converted
to radians internally (`input1 * M_PI / 180`), confirming the **degree** convention.

> **(UNVERIFIED — target-register ordering for op 1/2.)** The EasyRPG source carries an explicit
> warning comment (`src/game_runtime_patches.cpp:450`–453): *"Some versions of the documentation
> for this patch have the target ids swapped (SIN → FVALUE1, COS → FVALUE2, TAN → FVALUE1). But
> the only known RPG_RT versions of this patch actually save them like follows…"* — i.e. EasyRPG
> deliberately follows the **observed binary**, not the published docs, putting `sin/tan → V[6]`
> and `cos → V[5]`. A re-implementer must match the binary, not the Makerpendium text.

#### Picture rotation — `V[8]` (covered in §6.1/§6.2)

`V[8]` ≤ 10: "off" mode; bottom-transparency byte selects rotation direction (§6.1).
`V[8]` > 10: base variable id of a 50-entry absolute-rotation table for pictures 1–50 (§6.2).
(The exact ≤10 / >10 threshold is EasyRPG's; the original cut-off is RE-pending.)

### 8.2 Audio

PowerMode replaces the stock audio backend with `warp.dll` (the Makerpendium page: FMOD then
BASS, **adds AAC+/MP4, drops WMA**). **EasyRPG does not reimplement the `warp.dll` audio
formats**: PR [#3384](https://github.com/EasyRPG/Player/pull/3384) is "feature-complete" *except
AAC audio*. EasyRPG plays whatever standard formats its own audio stack supports; AAC+/MP4
content authored against `warp.dll` will not play. (See §9.)

### 8.3 Title / logo skip

The patch boots past the publisher logo and title screen. EasyRPG reproduces the title-screen
skip by forcing `new_game = true` in `OnResetGameObjects` when the patch is active
(`src/game_runtime_patches.cpp:199`). (The publisher-*logo* skip is a non-issue for EasyRPG,
which doesn't render the original RPG_RT startup logos for patched games.)

### 8.4 Limits, resolution, timing

No change. PowerMode targets 1.09 and keeps the 320×240 resolution, 50-picture limit, and the
standard refresh model. The only timing-adjacent behavior is that control-register writes run
synchronously inside `Game_Variables::Set`, and the affected output registers are pushed through
`SetNeedRefreshForVarChange` so parallel/auto-start event pages keyed on V[2]/V[5]/V[6] re-poll
in the same frame (`src/game_runtime_patches.cpp:499,506,507`).

## 9. EasyRPG support matrix

PR [#3384 "Patch compatibility: Power Mode 2003"](https://github.com/EasyRPG/Player/pull/3384)
by **florianessl**, merged **2026-03-13** (commit `1560cf41` on this tree, branch RISKY).
**Master-only: there has been no tagged Player release since 0.8.1.1 "Stun – Patch 1"
(2025-06-02), so PowerMode 2003 support is not in any release** — see the research corpus
`/home/john/research/easyrpg_status_web.md` §4.6 / [README](README.md).

| Feature | Status | Anchor / PR | Notes |
|---|---|---|---|
| Detection via `warp.dll` | **FULL** | `src/player.cpp:861` | Auto-enables the flag |
| `--patch-powermode` / `[Patch] PowerMode2003` | **FULL** | `src/game_config_game.cpp:170,255` | Override semantics standard |
| CR0 `V[1]`: open Load menu / quit / save-exists probe | **FULL** | `src/game_runtime_patches.cpp:378` | Opcodes 255/254 (see §8.1 caveat) |
| Title-screen skip | **FULL** | `src/game_runtime_patches.cpp:199` | via `new_game` |
| Mouse coords `V[2]`/`V[3]` | **FULL** (platform-gated) | `:388` | Warns where no mouse/touch support |
| Mouse buttons (L=decision, R=cancel) | **FULL** | `:202`–204 | |
| Full keyboard `V[4]` (scan + probe) | **FULL** (platform-gated) | `:399` | VK map at `:273`/`:377`; warns where no keyboard |
| Float ops `V[5]`/`V[6]`/`V[7]` (sin/cos/tan/sqrt/div) | **FULL** | `:447` | Follows the binary, not the docs (§8.1) |
| Picture rotation `V[8]` (table + direction reuse) | **FULL** | `:514`; `src/game_pictures.cpp:568` | Bottom-trans gradient disabled while active (`src/sprite_picture.cpp:34`) |
| `warp.dll` audio (AAC+/MP4) | **MISSING / WONTFIX** | PR #3384 body | "feature-complete except AAC audio" |
| Active-patch logging | **MISSING** (cosmetic) | `src/game_config_game.cpp:264`, `src/game_runtime_patches.cpp:151` | `patch_powermode` not added to the "Patch configuration:" line |

## 10. Test assets

No PowerMode 2003 game is currently checked into the corpus at `c:\rg\easyrpg_library`
(none identified during this research; UNVERIFIED). A minimal repro project for each register
would exercise:

| Asset (to create) | Stresses |
|---|---|
| `V[1]=255` after a save exists | CR0 load-menu jump + save-exists probe |
| `V[3]` write + parallel event reading `V[2]`/`V[3]` | mouse coords + `SetNeedRefreshForVarChange` |
| `V[4]=0` loop printing the returned VK | keyboard scan + VK map coverage |
| `V[5]=30; V[7]=1` then show `V[5]`,`V[6]` | sin/cos in degrees, ×1e6 scaling, target ordering |
| `V[5]=2; V[7]=3` | sqrt integer/fractional split |
| 50 pictures + `V[8]=100`, vary `V[100..149]` | absolute rotation table (§6.2) |
| 1 rotating picture with bottom-trans ≥50 vs <50 | direction hijack (§6.1) |

Cross-ref: a corpus PowerMode game, once located, should be added to `docs/games/` and noted
here. The Makerpendium page is the lead for finding the original distribution / sample games.

## 11. Open questions

1. **CR0 opcodes (RE-pending).** Confirm 255 = load and 254 = quit against the original
   `RPG_RT.exe`. EasyRPG implements these; the published docs don't state literal values.
2. **Trig target ordering (RE-pending).** Confirm `sin/tan → V[6]`, `cos → V[5]` against the
   binary — EasyRPG's source explicitly warns the docs disagree (§8.1).
3. **V[8] threshold (RE-pending).** Confirm the off/table cut-off (EasyRPG uses ≤10 vs >10) and
   the bottom-transparency direction threshold (EasyRPG uses signed `≥ 50`).
4. **`warp.dll` / `hvm.dll` relationship and version strings.** Are they one and the same? Do
   they carry a build identifier EasyRPG could use to distinguish PowerMode builds?
5. **Audio format set.** Verify the FMOD→BASS migration and the exact added/removed formats
   (AAC+/MP4 added, WMA dropped) against the DLLs; decide whether AAC HLE is worth adding.
6. **EXE signature detection.** Should EasyRPG add the `powermode 2k3` byte-signature check
   (issue [#1182](https://github.com/EasyRPG/Player/issues/1182)) so games that bundle no
   `warp.dll` (or renamed it) are still detected?
7. **Build dates / lineage.** RE the preview / WIP-20070114 / v0.02 builds to confirm dates and
   whether behavior differs across them.
8. **Logging gap.** Add `patch_powermode` to `PrintActivePatches` (trivial fix; currently
   omitted).

## 12. References

Primary:

- EasyRPG Player source (this tree, branch RISKY):
  - `src/game_runtime_patches.cpp:377`–533 (all PowerMode handlers), `:195`–234 (hooks)
  - `src/game_runtime_patches.h:241`–270 (namespace + register constants + doc comment),
    `:273`/`:377` (VK↔InputKey maps)
  - `src/player.cpp:861` (detection), `:1511` (help text)
  - `src/game_config_game.h:75` (flag), `src/game_config_game.cpp:100,170,255` (CLI/INI/lock)
  - `src/game_pictures.cpp:568` (rotation call site), `src/sprite_picture.cpp:34` (bottom-trans gate)
- PR [#3384 "Patch compatibility: Power Mode 2003"](https://github.com/EasyRPG/Player/pull/3384)
  (florianessl; merged 2026-03-13; commit `1560cf41`).
- EasyRPG issue [#1182](https://github.com/EasyRPG/Player/issues/1182) (EXE-signature detection,
  records the `powermode 2k3` string).

Secondary (archive where possible):

- Makerpendium wiki, **PowerMode 2003**:
  <https://www.makerpendium.de/wiki/PowerMode_2003> (author Firesta, `hvm.dll` by Ripp3er,
  Italian "Mega Patch 2003" by Calev, v1.09 target, build dates, audio/feature list).
- Research corpus (out of tree): `/home/john/research/other_patches_web.md` §4.3,
  `/home/john/research/easyrpg_status_web.md` §4.6.
- Cross-links: [README.md](README.md), [TEMPLATE.md](TEMPLATE.md),
  [official-versions.md](official-versions.md), [easyrpg-extensions.md](easyrpg-extensions.md),
  [runtime-micro-patches.md](runtime-micro-patches.md).
