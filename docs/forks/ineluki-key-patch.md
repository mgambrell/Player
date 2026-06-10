<!--
Spec: Ineluki's Key Patch (+ Disharmony / Force Harmony).
Part of the docs/forks/ RPG_RT fork-support series. Follows TEMPLATE.md section order.
Every nontrivial claim carries a source: external link, a file:line anchor into
/home/john/Player (src/…) or bundled liblcf (lib/liblcf/…), or an RE note. Unverified
claims are tagged (UNVERIFIED)/(RE-pending)/(hypothesis).
-->

# Ineluki's Key Patch (Tastenpatch / Ineluki Key Patch) + Disharmony / Force Harmony

> A German-scene patch by **Ineluki** that turns RPG Maker 2000/2003's audio DLL into a
> general-purpose scripting and input back-end. It ships a **replacement `Harmony.dll`**
> that intercepts the *Play Sound* event command: sound effects whose filenames end in
> `.script.wav` are not audio at all but **INI-format scripts** ("ini-script") that enable
> keyboard/mouse polling, run external programs, write to logs, and feed values back to the
> game through the **MIDI tick counter** (variable read mode "MIDI play position"). A
> companion mechanism plays real **MP3/OGG** music through `*.link.wav` alias files. Games
> used it to read the full keyboard (impossible in stock RM2k/2k3), build custom save/load
> menus, take screenshots, and stream compressed audio. The related **Disharmony** patch
> (Derula) generalizes the audio-aliasing half; **Force Harmony** (Bananen-Joe & Cherry)
> re-installs the *original* `Harmony.dll` on newer engines so the Key Patch keeps working
> at the cost of native MP3 playback.

**Status at a glance:** EasyRPG support = **PARTIAL** (most ini-script commands implemented;
`execProgram`/`mciCommand`, the `automatic` mouse auto-append, and the true save *count* in
`saves.script` are not). Detection = **`harmony.dll`
present in the game directory** → auto-enables `patch_key_patch`
(`src/player.cpp:844`). Spec confidence = **MEDIUM** (engine behavior is read from EasyRPG's
reimplementation and the original docs; the original binary `Harmony.dll` internals are
RE-pending).

---

## 1. Identity

- **Names / aliases:**
  - English: *Ineluki's Key Patch*, *Key Patch*.
  - German: *Tastenpatch* (literally "key/button patch"); *Inelukis Key Patch / Tastenpatch*.
  - Companion audio patches: *Disharmony Audio Patch* (Derula); *Force Harmony* (Bananen-Joe
    & Cherry); the original MP3 half is often just called the *"MP3 patch"* / *Ineluki MP3
    patch* (see `src/game_system.cpp:533`, `:538` "Ineluki MP3" log strings).
- **Author(s):** **Ineluki** (German RPG-Maker community). Disharmony by **Derula**; Force
  Harmony by **Bananen-Joe** (David Gausmann) **& Cherry** (David Trapp).
- **License / EULA / redistribution:** No formal license published with the original. The
  patch *replaces* Tkool's bundled `Harmony.dll`, which technically conflicts with the
  RPG Maker EULA (the German scene tolerated `.dll` swaps where the JP scene did not — see
  `other_patches_web.md` §7). Treat the binary `Harmony.dll` as **not redistributable**;
  detection only needs the *filename* and *file size*, not the bytes. (UNVERIFIED — no
  explicit license text located.)
- **Distribution URLs:**
  - Original (DEAD): `http://www.rpg2000.4players.de:1061/sonstiges/utils/InelukiKeyPatchv1-2.zip`
    (EasyRPG known-patches wiki) — host long offline.
  - Key Patch v1.2 mirror (ALIVE): <https://www.cherrytree.at/cms/download/2004/01/20/dl-3-inelukis-key-patch.html>
  - Disharmony Audio Patch (ALIVE): <https://cherrytree.at/cms/download/2009/12/29/dl-5-disharmony-audio-patch.html>
  - Force Harmony (DEAD download `forceharmony_english.rar`): documented on the EasyRPG
    known-patches wiki and <https://rpgmaker.net/forums/topics/689/>.
  - EasyRPG "Known patches": <https://wiki.easyrpg.org/development/technical-details/known-patches>
    (mirror <https://easyrpg.github.io/wiki/development/technical-details/known-patches/>).
- **Liveness:** Original Key Patch is **abandonware** (last release v1.2, ~2004). Disharmony
  remains downloadable at Cherry Tree (2009). No active maintenance; the community has
  largely moved to Maniac Patch (Steam 2k3) and DynRPG (2k3 1.08) for the same roles.

---

## 2. Target engine builds

The patch works against RPG_RT builds that still use the **original `Harmony.dll`** audio
layer:

| Engine | Versions the patch targets | Note |
|---|---|---|
| RPG Maker 2000 | up to **1.10** | Builds before the VALUE! (1.50) audio rework |
| RPG Maker 2003 | up to **1.04** | Builds before 1.05 changed the audio DLL |

Source: `other_patches_web.md` §4.4 and the EasyRPG known-patches wiki. On **RM2k VALUE!
(≥1.50)** and **RM2k3 ≥1.05**, Tkool replaced/extended the audio DLL and the original
`Harmony.dll` interface (used by the Key Patch) is gone — which is exactly why **Force
Harmony** exists: it "downgrades" those engines back to the original `Harmony.dll` so the Key
Patch's script hook works again, sacrificing native MP3 (`other_patches_web.md` §1, §4.4;
<https://rpgmaker.net/forums/topics/689/>).

The Key Patch does **not** ship a whole replacement `RPG_RT.exe` — it ships a replacement
`Harmony.dll` (plus optional helper EXEs). The engine binary is unmodified, so EasyRPG's
`RPG_RT.exe` version/logo detection is unaffected; detection is purely DLL-based (§4).

---

## 3. Version lineage

The Key Patch itself had few public builds; the lineage that matters is the **family** of
related audio/input patches that all hinge on `Harmony.dll`.

| Build / variant | Date | Headline | Published |
|---|---|---|---|
| Ineluki Key Patch v1.2 | ~2004-01-20 | keyboard/mouse, run EXEs, MP3 via `Harmony.dll` replacement; `*.script.wav`/`*.link.wav` | Yes (mirror at Cherry Tree) |
| Disharmony Audio Patch | 2009-12-29 | MP3/OGG with fades via `name.ext.wav` aliasing; generalizes the audio half | Yes (Cherry Tree dl-5) |
| Force Harmony (EN) | (date UNVERIFIED) | re-installs original `Harmony.dll` on RM2k VALUE! / RM2k3 ≥1.05 so Key Patch works again (loses MP3) | Download dead; doc on EasyRPG wiki |

Earlier Key Patch builds (<1.2) are referenced but not catalogued here (RE-pending). The
"few uncommon versions of the patch" that add `setMouseAsReturn` / `setMouseWheelAsKeys` are
noted in EasyRPG's code as rare variants (`src/game_ineluki.cpp:178`, `:197`).

---

## 4. Detection

How to recognize a Key-Patch game **without running it**:

- **Bundled DLL (primary):** a file named **`harmony.dll`** in the game directory. EasyRPG
  treats *any* `harmony.dll` presence as the Key Patch signal — it does not byte-compare the
  DLL here (`src/player.cpp:844`).
  - Note a subtlety: a *stock* (official) `Harmony.dll` also ships with many RM2k/2k3 games.
    EasyRPG's filename test will set `patch_key_patch` even for stock DLLs; the Key-Patch
    *scripts* simply never fire if no `*.script.wav` files exist, so the false positive is
    harmless (the script hook is only reached through *Play Sound*). The **official**
    `Harmony.dll` is **473600 bytes** — `KnownFileSize::OFFICIAL_HARMONY_DLL`
    (`src/filefinder.h:368`); a *different* size implies a replacement DLL (Key Patch /
    Disharmony / Force-Harmony-restored), which `FileFinder::IsMajorUpdatedTree()` uses to
    skip its MP3-based engine heuristic (`src/filefinder.cpp:531-538`). A size-based
    discriminator could distinguish "official" from "patched" Harmony, but EasyRPG does not
    currently use it to gate `patch_key_patch`.
- **Corroborating data-file signatures (no run needed):**
  - One or more **`*.script.wav`** files referenced from *Play Sound* commands (the ini-script
    files). Their content is INI text, not RIFF/WAVE.
  - **`autorun.script`** in the game root — a newline-delimited list of script files run at
    startup/title (`src/player.cpp:880-882`, `src/scene_title.cpp:94-96`).
  - **`*.link.wav`** files under `Music/` (or `*.link` more generally) — small (<500 byte)
    text files whose first line is a relative path to a real MP3/OGG. EasyRPG keys on the
    `.link` suffix and a `<500` byte size (`src/game_system.cpp:557`, `:602`).
  - `SaveCount.dat` / a `saves.script` reference — used by the save-existence helper script
    (`src/game_ineluki.cpp:60-66`; see §7.1 for EasyRPG's bool-vs-count limitation).
- **PE characteristics of `RPG_RT.exe`:** none specific to this patch — the engine binary is
  unmodified (§2). Do **not** rely on `RPG_RT.exe` signatures for Key-Patch detection.

**EasyRPG mapping:**

| Aspect | Value | Anchor |
|---|---|---|
| `Game_ConfigGame` flag | `patch_key_patch` (bool, default false) | `src/game_config_game.h:48` |
| `EasyRPG.ini [Patch]` key | `KeyPatch` | `src/game_config_game.cpp:239` (`FromIni`) |
| CLI flag | `--patch-key-patch` / `--no-patch-key-patch` | `src/game_config_game.cpp:136-138`; help text `src/player.cpp:1502` |
| Legacy multi-flag form | `--patch … key-patch` | `src/game_config_game.cpp:191-192` |
| Auto-detect anchor | `harmony.dll` present ⇒ `patch_key_patch.Set(true)` | `src/player.cpp:844-846` |
| Predicate | `Player::IsPatchKeyPatch()` | `src/player.h:528-535` |
| Per-savegame runtime flag | `keypatch` (`EasyRpgStateRuntime_Flags`) toggled by `EasyRpg_SetInterpreterFlag` (cmd 2053) | `src/window_interpreter.cpp:55`; `src/player.h:531` |

**Precedence:** setting any patch option explicitly (ini or CLI) sets `patch_override`, which
**disables all DLL auto-detection** including the `harmony.dll` check
(`src/player.cpp:843`; see [easyrpg-extensions.md](easyrpg-extensions.md)). `--no-patch`
locks every patch off (`src/game_config_game.cpp:96` calls `patch_key_patch.Lock(false)`).

---

## 5. Event commands

**This fork adds no new numeric event-command codes.** Unlike Maniac Patch (3001–3032) or
EasyRPG's own extensions (2002–2058), the Key Patch piggy-backs entirely on **existing**
RM2k/2k3 commands. Its "command set" is the **ini-script action vocabulary** (covered in §8.1)
plus a single re-purposed *variable read mode* (covered in §6). The heading is kept per
template; there is nothing to populate in the per-code table.

The trigger surface is:

| Stock command | How the Key Patch overloads it | EasyRPG anchor |
|---|---|---|
| **Play Sound (SE)** — `PlaySound` (11550) | A *Play Sound* whose filename ends in `.script.wav`/`.script` is dispatched to the ini-script interpreter instead of the audio engine. Real `Harmony.dll` distinguished by intercepting the SE play call. | `src/game_system.cpp:583-587` (routes `.script` to `Game_Ineluki::Execute`) |
| **Play Sound (SE)** with `*.link.wav` | Plays the aliased MP3/OGG named on the first line of the link file. | `src/game_system.cpp:602-611` |
| **Play BGM** — `PlayBGM` (11510) with `*.link.wav` | Same MP3/OGG aliasing for background music. | `src/game_system.cpp:557-567` |
| **Control Variables → "Other" → "MIDI play position"** (op 8) | Returns the script "output list" tail (LIFO pop) instead of real MIDI ticks while a script set output mode to `output` — the channel scripts use to return data to the game. | `src/game_interpreter_control_variables.cpp:244-251` |

---

## 6. Modified baseline commands

### 6.1 Control Variables — "MIDI play position" read mode (the return channel)

Stock RM2k/2k3 *Control Variables* can read **"Other → MIDI play position"** (the playback
position of the current MIDI in ticks). The Key Patch hijacks this read so scripts can return
integers to the game:

- An ini-script can switch the engine into **output mode** (`miditickfunction = output`,
  §8.1). While in output mode, "MIDI play position" pops and returns the **last** value of an
  internal **output list** (LIFO) instead of real MIDI ticks; `miditickfunction = original`
  restores normal MIDI-tick behavior; `miditickfunction = clear` empties the list.
- EasyRPG: `ControlVariables::Other` case 8 calls `Game_Ineluki::GetMidiTicks()` when
  `IsPatchKeyPatch()` (`src/game_interpreter_control_variables.cpp:244-251`); the pop-from-back
  logic is `src/game_ineluki.cpp:324-335`. In output mode an **empty** list returns **-1**
  (sentinel), so games typically push a known value before reading
  (`src/game_ineluki.cpp:328-333`).

This is the documented Key-Patch convention: *"executes scripts via the 'Play sound' command,
and returns data through the midi ticks"* (EasyRPG known-patches wiki;
`other_patches_web.md` §4.4).

### 6.2 Play BGM / Play Sound — MP3/OGG via `*.link.wav`

Stock RM2k ≤1.10 / RM2k3 ≤1.04 cannot play MP3 BGM. The Key Patch (and Disharmony)
add it by **aliasing**: a tiny text file `Something.link.wav` placed where the engine expects a
WAV contains, on its first line, a *relative path* to the real `.mp3`/`.ogg`; an optional
second line `loop` (and Disharmony fade parameters) controls looping. The replacement
`Harmony.dll` reads the link file and streams the real audio. See §7.2 for the file format and
§9 for EasyRPG's loop-support gap.

No other baseline commands are modified — the patch is a thin shim over audio + the MIDI-tick
read.

---

## 7. File-format changes

The Key Patch introduces **no LDB/LMU/LMT/LSD chunk changes** and therefore needs no liblcf
model changes. Everything is **standalone sidecar files** with conventional `.wav`/`.script`
extensions so the unmodified editor accepts them as ordinary Sound/Music resources.

### 7.1 `*.script.wav` / `*.script` — ini-script files

- **Container:** plain text, **INI format** (parsed via `lcf::INIReader`,
  `src/game_ineluki.cpp:256`).
- **Reference:** named in a *Play Sound* command (SE), so the editor treats it as a sound
  effect. The `.wav` second extension makes the editor's resource picker show it; the engine
  matches on the inner **`.script`** suffix (EasyRPG: `EndsWith(name, ".script")`,
  `src/game_system.cpp:583`, `:203`), not on `.wav`.
- **Structure:** sections form a **singly-linked chain**. Parsing starts at section
  `[execute]`; each section has an `action` key plus action-specific keys, and a **`next`**
  key naming the next section to run (empty `next` ends the chain)
  (`src/game_ineluki.cpp:264-317`). Per-action keys are in §8.1.
- **Special-cased name:** `saves.script` — synthesised handler that switches to output mode
  and pushes `FileFinder::GetSavegames()` onto the output list (the SaveCount.dat helper),
  without needing a real file (`src/game_ineluki.cpp:60-66`).
  - **EasyRPG limitation:** despite the in-code comment "It counts the amount of savegames",
    `FileFinder::GetSavegames()` (`src/filefinder.cpp:408-421`) actually returns a **bool as
    int** — it iterates `Save01.lsd`…`Save15.lsd` and `return true` on the **first** one found,
    so the value pushed is **1 if any save exists, else 0**, never the true count. The original
    SaveCount.dat helper returned the real number of saves; games that rely on a count > 1
    behave incorrectly under EasyRPG. (Engine bug — worth flagging upstream.)
- **liblcf coverage:** N/A — not an LCF structure; the *Play Sound* command that references it
  is a normal `lcf::rpg::Sound` and round-trips losslessly.

### 7.2 `*.link.wav` / `*.link` — MP3/OGG alias files

- **Container:** plain text, **< 500 bytes** (EasyRPG's size gate, `src/game_system.cpp:557`,
  `:602`).
- **Line 1:** relative path to the real audio file (MP3/OGG). EasyRPG re-encodes the line with
  the game's encoding and canonicalises it (`Game_System::InelukiReadLink`,
  `src/game_system.cpp:529-541`).
- **Line 2+ (original patch / Disharmony):** optional `loop` directive and (Disharmony) fade
  parameters. (UNVERIFIED exact syntax — RE-pending; EasyRPG currently reads **only line 1**.)
- **Folder convention:** under `Music/` for BGM links; SE links resolved by the normal sound
  search path.
- **liblcf coverage:** N/A — referenced by an ordinary BGM/SE command, round-trips losslessly.

### 7.3 `autorun.script`

- **Container:** plain text; **newline-delimited list of script filenames** to execute at
  startup and on every return to the title screen.
- **EasyRPG:** `Game_Ineluki::ExecuteScriptList` requests each listed file asynchronously as an
  *important file*, then executes them **in listed order** once all are fetched
  (`src/game_ineluki.cpp:219-246`, `:378-393`). It is preloaded by the logo scene
  (`src/scene_logo.cpp:262`), and run at `src/player.cpp:880-882` and
  `src/scene_title.cpp:94-96`. The list file itself is **not** fetched through the async path
  (it is opened directly).

### 7.4 Helper / companion files (informational)

- `SaveCount.dat` — a tiny program/data file in some distributions that counts saves; EasyRPG
  emulates its effect via the `saves.script` special case rather than executing it
  (`src/game_ineluki.cpp:60-66`), and an `execProgram` referencing `SaveCount.dat` is a no-op
  (`:96-98`). Note the count is reduced to a 0/1 existence flag under EasyRPG (§7.1).
- Companion utilities that *depend on* the Key Patch (not part of it): Miroku's **Screenshot
  Extension** (`Effect.exe`, writes 256-color screenshots into `Picture/`), and a
  **Save-/Load-Script** by Vampire of Eternal Chaos (AutoHotkey-based custom save/load UI)
  (EasyRPG known-patches wiki; `other_patches_web.md` §4.4). These run as external programs via
  `execProgram` (which EasyRPG does not execute — §8.1, §9).

---

## 8. Runtime behavior changes

### 8.1 ini-script action vocabulary (the real "command set")

Each `[section]` of a `*.script.wav` file has an `action` plus parameters. The table lists
every action EasyRPG recognises, its INI keys, behavior, and support status. Sourced from the
parser (`src/game_ineluki.cpp:266-317`) and executor (`:86-213`). German key-event names come
from `key_to_ineluki[]` (`src/game_ineluki.h:141-204`).

| `action` | INI keys (parsed) | Behavior | EasyRPG status | Anchor |
|---|---|---|---|---|
| `writeToLog` | `text` | Writes `text` to the engine log/console. | **FULL** | `game_ineluki.cpp:89-90` |
| `execProgram` | `command` | Original: runs an external program. | **PARTIAL** — only fakes known programs: `exitgame*`/`taskkill*` → set exit flag; `SaveCount.dat` → no-op; everything else warns "Not supported". | `game_ineluki.cpp:91-100` |
| `mciCommand` | `command` | Original: issues a Windows MCI command (e.g. CD/audio). | **MISSING** — warns "Not supported". | `game_ineluki.cpp:101-102` |
| `midiTickFunction` | `command` (else `value`) | Sets MIDI-tick return mode: `original` → real ticks; `output` → pop from output list; `clear` → empty list. | **FULL** | `game_ineluki.cpp:103-111` |
| `addOutput` | `value` | Pushes integer `value` onto the output list (data the game later reads via "MIDI play position", §6.1). | **FULL** | `game_ineluki.cpp:112-113` |
| `enableKeySupport` | `enable` (`true`/`false`) | Enables/disables per-frame keyboard polling. When enabling, masks WASD from the normal movement mapping so raw key events fire (`mask_kb`, `game_ineluki.cpp:33-48`). | **FULL** (no-op + warning on platforms without `SUPPORT_KEYBOARD`) | `game_ineluki.cpp:114-128` |
| `registerKeyDownEvent` | `key`, `value` | When `key` is *pressed* this frame, push `value` onto the output list. `key` is a German key name (table below). | **FULL** | `game_ineluki.cpp:129-136` |
| `registerKeyUpEvent` | `key`, `value` | When `key` is *released* this frame, push `value`. | **FULL** | `game_ineluki.cpp:137-144` |
| `enableMouseSupport` | `enable`, `id`, `automatic` | Enables mouse polling; `id` = a prefix value pushed with each mouse report; `automatic` (original: append mouse pos every 500 ms). | **PARTIAL** — `automatic` 500 ms auto-append is a TODO; mouse warns on unsupported platforms. | `game_ineluki.cpp:145-157` (TODO `:149`) |
| `getMousePosition` | *(none)* | No-op if `enableMouseSupport` was not set (`:159-161`). Pushes 4 ints in push order `button`, `y`, `x`, `id`; since the list is LIFO the game reads them back as `id`, `x`, `y`, `button` (button: 0 none, 1 left, 2 right, 3 both — `:165-167`). | **FULL** | `game_ineluki.cpp:158-172` |
| `setDebugLevel` | `level` | Original: sets a debug verbosity. | **STUB** — no-op. | `game_ineluki.cpp:173-174` |
| `registerCheatEvent` | `cheat`, `value` | Registers a key-sequence "cheat code"; when the player types the sequence, push `value`. Code chars map to keys via the first char of each `key_to_ineluki` name. | **FULL** | `game_ineluki.cpp:175-176`; cheat matching `:356-375`; ctor `:395-404` |
| `setMouseAsReturn` | `value` (`left`/`right`/`both`/`none`) | Binds mouse button(s) to the DECISION (confirm) input. Rare patch variant. **No-op unless `enableMouseSupport` was set first** (early-returns on `!mouse_support`, `:179-181`); invalid value warns and falls back to `none`. | **FULL** (routes to `RuntimePatches::mouse_bindings`, sets `enabled=true`) | `game_ineluki.cpp:177-195` |
| `setMouseWheelAsKeys` | `value` (`updown`/`leftright`/`none`) | Maps the mouse wheel to direction keys. Rare patch variant. **No-op unless `enableMouseSupport` was set first** (`:198-200`); invalid value warns and falls back to `none`. | **FULL** | `game_ineluki.cpp:196-212` |
| *(unknown action)* | — | Original: ignored. | Logged "Unknown command" and skipped; section still chains via `next`. | `game_ineluki.cpp:307-310` |

**Parser/chaining notes:**
- Section walk starts at `[execute]` and follows each section's `next` key; the loop ends when
  `next` is empty (`game_ineluki.cpp:264-317`). A script is **parsed once and cached** by file
  name (`functions` map, `game_ineluki.h:104`; cache check `game_ineluki.cpp:80-84`).
- Action names are matched **case-insensitively** (lower-cased on parse,
  `game_ineluki.cpp:268`).
- The **output list is LIFO** (`GetMidiTicks` pops the back, `game_ineluki.cpp:330-331`), so a
  script that pushes `a` then `b` returns `b` first. Scripts/games are written around this.

### 8.2 Key-name table (German → InputKey)

Key event scripts reference keys by **German names**. EasyRPG maps them in
`Game_Ineluki::key_to_ineluki` (`src/game_ineluki.h:141-204`). Selected mappings (Japanese not
applicable; these are German):

| Ineluki name | Meaning (EN) | InputKey |
|---|---|---|
| `(links)` / `(rechts)` / `(hoch)` / `(runter)` | left / right / up / down arrows | LEFT/RIGHT/UP/DOWN |
| `a` … `z` | letter keys | A…Z |
| `0` … `9` | number keys | N0…N9 |
| `.` | period | PERIOD |
| `(tab)` `(entf)` `(ende)` `(bildrunter)` `(bildhoch)` `(pos1)` `(einfg)` | Tab, Delete, End, PageDown, PageUp, Home, Insert | TAB/DEL/ENDS/PGDN/PGUP/HOME/INSERT |
| `(esc)` `(enter)` `(space)` `(backspace)` | Escape, Return, Space, Backspace | ESCAPE/RETURN/SPACE/BACKSPACE |
| `(strg)` `(alt)` `(capslock)` `(numlock)` `(scrolllock)` | Ctrl, Alt, Caps/Num/Scroll Lock | CTRL/ALT/CAPS_LOCK/NUM_LOCK/SCROLL_LOCK |
| `(lshift runter)` `(rshift runter)` `(lshift hoch)` `(rshift hoch)` | left/right Shift down/up | LSHIFT/RSHIFT |

Notes captured in the source: German umlaut keys (ä/ö/ü) are **not** mappable
(`game_ineluki.h:138-140`); the duplicated Shift "runter"/"hoch" (down/up) names are flagged
as an oddity in the original patch (`game_ineluki.h:199-203`).

### 8.3 Input model

- Key polling uses **raw** key state, bypassing the normal RM2k input remap:
  `Input::IsRawKeyTriggered` (down), `IsRawKeyReleased` (up), `IsRawKeyPressed` (cheats)
  (`game_ineluki.cpp:343-375`). Polled once per frame from `Game_Ineluki::Update()`
  (`:337-341`), driven by `Player::Update` (`src/player.cpp:380-382`).
- Enabling key support **masks WASD** out of the movement mapping so those keys deliver raw
  events to scripts instead of moving the hero (`mask_kb`, `game_ineluki.cpp:33-48`); the mask
  is removed on destruction (`~Game_Ineluki`, `:51-57`).
- Mouse-as-input is delegated to the shared `RuntimePatches::mouse_bindings` mechanism
  (`game_runtime_patches.h`), the same one PowerMode 2003 uses
  (`game_ineluki.cpp:184-212`).

### 8.4 Audio model

- MP3/OGG playback is achieved purely through `*.link.wav` aliasing (§6.2, §7.2). EasyRPG
  performs an extra async round-trip: read the link, then request and play the real file
  (`OnBgmReady`→`OnBgmInelukiReady`, `src/game_system.cpp:557-575`; SE path `:602-641`).
- **Loop handling:** the original link file may carry a `loop` directive; EasyRPG reads **only
  line 1** and applies the engine's default loop behavior (no per-link loop/fade override).
  (UNVERIFIED whether stock RM2k loop semantics match the patch — RE-pending.)
- **Force Harmony interaction:** if a game restored the original `Harmony.dll` (Force Harmony)
  to regain the Key Patch on a newer engine, native MP3 is lost in the *real* engine; under
  EasyRPG this is moot — EasyRPG plays MP3/OGG natively, and the link mechanism works
  regardless of which `Harmony.dll` filename is present.

### 8.5 Limits / resolution / save semantics

No changes. The Key Patch does not alter picture/variable limits, resolution, refresh timing,
text escape codes, or save-file layout. Custom save/load *menus* built on top of it (e.g. the
Save-/Load-Script) are external AutoHotkey programs invoked via `execProgram`, which EasyRPG
does not run (§9).

---

## 9. EasyRPG support matrix

Implementation lives in **`src/game_ineluki.{h,cpp}`** plus audio hooks in
`src/game_system.cpp` and the variable read hook in
`src/game_interpreter_control_variables.cpp`. No PR/issue is cited inline below where the code
is simply present in `master`; relevant tracking issues: RFC
<https://github.com/EasyRPG/Player/issues/939>, Key-Patch issue
<https://github.com/EasyRPG/Player/issues/1181>.

| Feature | Status | Anchor / note | Release annotation |
|---|---|---|---|
| Detection via `harmony.dll` → `patch_key_patch` | **FULL** | `player.cpp:844-846` | present in 0.8.x |
| ini-script parsing (INI chain via `next`) | **FULL** | `game_ineluki.cpp:248-322` | present in 0.8.x |
| `autorun.script` startup/title execution | **FULL** | `game_ineluki.cpp:219-246`; `player.cpp:880-882`; `scene_title.cpp:94-96` | present in 0.8.x |
| `writeToLog`, `addOutput`, `midiTickFunction` | **FULL** | `game_ineluki.cpp:89-113` | present in 0.8.x |
| MIDI-tick return channel (Control Variables "Other" op 8) | **FULL** | `game_interpreter_control_variables.cpp:244-251`; `game_ineluki.cpp:324-335` | present in 0.8.x |
| Keyboard support (`enableKeySupport`, `registerKeyDown/UpEvent`, `registerCheatEvent`) | **FULL** (no-op + warning where `SUPPORT_KEYBOARD` undefined) | `game_ineluki.cpp:114-144`, `:175-176`, `:343-375` | present in 0.8.x |
| WASD masking while key support on | **FULL** | `game_ineluki.cpp:33-48` | present in 0.8.x |
| Mouse position (`enableMouseSupport`, `getMousePosition`) | **PARTIAL** — `automatic` 500 ms auto-append not implemented; platform-gated | `game_ineluki.cpp:145-172` (TODO `:149`) | present in 0.8.x |
| `setMouseAsReturn`, `setMouseWheelAsKeys` (rare variants) | **FULL** | `game_ineluki.cpp:177-212` | present in 0.8.x |
| `saves.script` / SaveCount.dat emulation | **PARTIAL** — returns 0/1 existence flag, not the true save count (`FileFinder::GetSavegames` is bool-valued despite its name; §7.1) | `game_ineluki.cpp:60-66`; `filefinder.cpp:408-421` | present in 0.8.x |
| `*.link.wav` MP3/OGG aliasing (BGM + SE) | **PARTIAL** — line-1 path only; no per-link `loop`/fade parsing | `game_system.cpp:529-575`, `:602-641` | present in 0.8.x |
| `execProgram` | **PARTIAL** — only fakes `exitgame`/`taskkill` (quit) and `SaveCount.dat`; all other programs warn | `game_ineluki.cpp:91-100` | present in 0.8.x |
| `mciCommand` | **MISSING** — warns "Not supported" | `game_ineluki.cpp:101-102` | present in 0.8.x |
| `setDebugLevel` | **STUB** — no-op | `game_ineluki.cpp:173-174` | present in 0.8.x |
| Disharmony fade/loop directives | **MISSING** | not parsed (only `.link` line 1) | — |
| Force Harmony | **N/A** — handled implicitly (native audio; filename-based detection unaffected) | §8.4 | — |
| Per-savegame toggle (`EasyRpg_SetInterpreterFlag` "keypatch") | **FULL** | `window_interpreter.cpp:55`; `player.h:531` | present in 0.8.x |

**Summary:** EasyRPG covers the **scripting + input + audio-alias** core. The intentional gaps
are exactly the features that need a real OS (`execProgram` running arbitrary external EXEs,
`mciCommand` for Windows MCI). Games that lean on those (e.g. external screenshot/save GUIs)
degrade gracefully — the script still runs, the unsupported action warns and is skipped.

---

## 10. Test assets

- No dedicated minimal repro project is bundled in this tree. Candidate stressors from the
  corpus (`c:\rg\easyrpg_library`, see [README](README.md)):
  - Any game shipping a non-473600-byte `harmony.dll` + `*.script.wav` files exercises the
    ini-script path and the MIDI-tick return channel.
  - A game with `Music/*.link.wav` exercises the MP3/OGG alias path (§7.2).
  - A game using Miroku's Screenshot Extension or the Save-/Load-Script exercises the
    `execProgram` warning path (§9).
- Cross-reference: when a concrete corpus game is identified, link it under `docs/games/`
  (e.g. alongside [../games/beloved-rapture.md](../games/beloved-rapture.md)). (RE-pending —
  no Key-Patch test game has been pinned in this corpus yet.)

---

## 11. Open questions

1. **`*.link.wav` / Disharmony second-line syntax.** Exact `loop` + fade directive grammar is
   unverified; EasyRPG reads only line 1. (RE-pending — inspect a real Disharmony link file.)
2. **Original `Harmony.dll` script-hook ABI.** How the replacement DLL intercepts the SE play
   call and where it injects the output values vs. real MIDI ticks (the exact interface the
   engine calls into) is not reverse-engineered here. (RE-pending.)
3. **Stock vs. patched `Harmony.dll` discrimination.** Should `patch_key_patch` auto-enable be
   gated on `size != OFFICIAL_HARMONY_DLL` (473600) to avoid flagging stock-DLL games? Current
   code does not gate; harmless because scripts only fire via `*.script.wav`. Worth confirming
   with upstream. (Question for maintainers.)
4. **Key Patch sub-versions (<1.2).** Which actions existed in which build; `setMouseAsReturn`/
   `setMouseWheelAsKeys` are only in "a few uncommon versions" (`game_ineluki.cpp:178`, `:197`).
   (RE-pending — locate older builds.)
5. **`mciCommand` usage in real games.** Whether any shipping game depends on MCI (CD audio),
   to judge MISSING-status risk. (Corpus survey pending.)
6. **`saves.script` count regression.** `FileFinder::GetSavegames()` returns a 0/1 existence
   flag, not the save count its name and the in-code comment imply (`filefinder.cpp:408-421`).
   Confirm with upstream whether any Key-Patch game depends on the true count (custom save
   menus showing "N saves") and whether to fix `GetSavegames` to iterate all 15 slots.
   (Question for maintainers / corpus survey.)

---

## 12. References

Primary sources:

- EasyRPG Player source (this tree): `src/game_ineluki.{h,cpp}`, `src/game_system.cpp`
  (audio hooks `:529-642`), `src/game_interpreter_control_variables.cpp:244-251`,
  `src/player.cpp:844-846`, `:880-882`, `src/filefinder.cpp:531-538`, `:408`,
  `src/filefinder.h:368`, `src/game_config_game.{h,cpp}`, `src/player.h:528-535`,
  `src/scene_title.cpp:94-96`, `src/scene_logo.cpp:262`, `src/window_interpreter.cpp:55`.
- EasyRPG "Known patches" wiki: <https://wiki.easyrpg.org/development/technical-details/known-patches>
  (mirror <https://easyrpg.github.io/wiki/development/technical-details/known-patches/>).
- EasyRPG issues: RFC <https://github.com/EasyRPG/Player/issues/939>; Key Patch
  <https://github.com/EasyRPG/Player/issues/1181>.
- Ineluki Key Patch v1.2 mirror: <https://www.cherrytree.at/cms/download/2004/01/20/dl-3-inelukis-key-patch.html>.
- Disharmony Audio Patch: <https://cherrytree.at/cms/download/2009/12/29/dl-5-disharmony-audio-patch.html>.
- Force Harmony / patch list context: <https://rpgmaker.net/forums/topics/689/>.
- Original (DEAD): `http://www.rpg2000.4players.de:1061/sonstiges/utils/InelukiKeyPatchv1-2.zip`.

Research corpus (outside repo): `/home/john/research/other_patches_web.md` §1, §4.4, §7;
`/home/john/research/easyrpg_patch_inventory.md` §1.1, §1.2, §5.

Sibling specs: [README.md](README.md) (registry/conventions),
[maniac-patch.md](maniac-patch.md), [powermode2003.md](powermode2003.md) (shares
`RuntimePatches::mouse_bindings`), [destiny.md](destiny.md), [dynrpg.md](dynrpg.md),
[easyrpg-extensions.md](easyrpg-extensions.md) (override precedence,
`EasyRpg_SetInterpreterFlag`).
