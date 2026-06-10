<!--
Spec: DynRPG (RM2k3 Plugin SDK). Part of the docs/forks/* series.
Follows TEMPLATE.md. Anchors point into /home/john/Player (this tree) unless noted.
-->

# DynRPG ("The RM2k3 Plugin SDK")

> DynRPG is a binary fork of RPG Maker 2003's runtime, `RPG_RT.exe` **v1.08**, created by
> David **"Cherry" Trapp**. It turns the closed runtime into a plugin host: a patched
> `RPG_RT.exe` loads `dynloader.dll` at startup, which in turn loads community-written **native
> C++ plugin DLLs** from a `DynPlugins/` folder. Plugins receive engine callbacks and can
> implement **"comment commands"** — event `Comment` lines beginning with `@command` — plus the
> patch ships **QuickPatches** (text-format memory pokes in `DynRPG.ini`) and a bundle of engine
> bugfixes. A game uses DynRPG to gain scripting and capabilities (custom text, alternate ATB,
> OpenGL, Lua, key input, etc.) that vanilla RM2k3 1.08 cannot express. Because plugins are
> arbitrary x86 DLLs, EasyRPG **cannot load them**; it instead **High-Level-Emulates (HLE)** a
> small fixed set of plugins re-coded in C++.

**Status at a glance:** EasyRPG support = **PARTIAL (HLE-only)**. The comment-command parser and
sidecar save format are implemented; native DLLs are never loaded; exactly **two** plugins are
re-implemented (`EasyRpgPlugin`, `TextPlugin`). Detection = **`dynloader.dll` present in the game
folder** (`src/player.cpp:848`). Spec confidence = **MEDIUM** (Player side HIGH from source; native
SDK / plugin-internal behavior MEDIUM, from Cherry's docs and not re-verified against the binary).

---

## 1. Identity

- **Names / aliases:** DynRPG; "The RM2k3 Plugin SDK"; "DynRPG patch". No established Japanese name
  (a German/English-scene project). `dyn` = *dynamic*.
- **Author:** David **"Cherry" Trapp** (German RM scene). Community continuation by **PepsiOtaku**
  and Andrew **"rewking" King**.
- **License / redistribution:** The SDK is open-source (GitHub, below). Plugins set their own
  licenses. DynRPG patches the **official** RM2k3 1.08 `RPG_RT.exe`; the patcher ships only the
  diff and offers to fetch 1.08 if missing — it does **not** redistribute the copyrighted runtime.
- **Distribution URLs:**
  - Cherry site / docs: <https://rpg-maker.cherrytree.at/dynrpg/> (alive) and patch page
    <https://rpg-maker.cherrytree.at/dynrpg/patch.html> (alive).
  - Cherry's source: <https://github.com/CherryDT/DynRPG> (alive).
  - Community fork (PepsiOtaku/rewking) site: <https://www.rewking.com/dynrpg/> (alive); source
    <https://github.com/rewrking/DynRPG> (alive).
  - Archived installer: <https://archive.org/details/rm2k3_dynrpg_patch> (archived).
- **Liveness:** Cherry's last release is **v0.20** (docs regenerated 2021). The community line
  reached **v0.32 (2017-02-25)**. Effectively in maintenance; the plugin ecosystem is the active
  part.

## 2. Target engine builds

DynRPG applies to **RPG Maker 2003 with `RPG_RT.exe` version 1.08 only.** Cherry's patch page is
explicit: "The patch requires RPG Maker 2003 with RPG_RT.exe version 1.08. If users lack this
version, the patcher offers to install it"
(<https://rpg-maker.cherrytree.at/dynrpg/patch.html>). 1.08 (build 2003-09-22) is the last Japanese
RM2k3 crash-fix build and the de-facto Western modding baseline; see
[official-versions.md](official-versions.md) §RM2k3.

DynRPG **patches an existing `RPG_RT.exe`** in place (via `dynrpg_patcher.exe`) — it does **not**
ship a full replacement runtime. The patch inserts a startup hook that `LoadLibrary`s
`dynloader.dll`, and (being a Cherry tool) marks the exe with Cherry's `CHERRY` PE section. The
community fork targets the same 1.08 base. It does **not** target the official English/Steam line
(1.10–1.12a) — those games use Maniac Patch instead; see [maniac-patch.md](maniac-patch.md).

## 3. Version lineage

| Build | Date | Headline changes | Published |
|---|---|---|---|
| DynRPG 0.x (early) | ~2011–2013 | Initial SDK, comment commands, QuickPatches, bugfixes | Yes (cherrytree) |
| **DynRPG v0.20** | docs regen 2021 | Cherry's last published version; reference SDK | Yes |
| **DynRPG v0.32** | 2017-02-25 | Community continuation by PepsiOtaku / rewking; SDK build-system modernization, new plugin support | Yes (rewking.com / GitHub) |

Notes: version numbers above 0.20/0.32 advance the **SDK**, not the on-disk patch protocol; the
comment-command syntax, `DynRPG.ini` format, and `dynloader.dll` loader behavior are stable across
the line. (RE-pending: precise dated changelog of 0.x point releases — Cherry's blog is the source.)

## 4. Detection

DynRPG games are recognized **purely by the loader DLL**, not by any data-file signature.

| Evidence | Meaning | Notes |
|---|---|---|
| **`dynloader.dll`** in game folder | DynRPG loader — definitive | The bootstrap DLL injected at startup. EasyRPG keys on exactly this filename (`src/player.cpp:848`). |
| `DynPlugins/` folder | Holds the native plugin DLLs | Corroborating; one `.dll` per plugin. EasyRPG does not scan it. |
| `DynRPG.ini` | Plugin config + `[QuickPatches]` | Corroborating; see §7. |
| `CHERRY` PE section in `RPG_RT.exe` | Exe was patched by a Cherry tool | Detected by `EXEReader` into `file_info.cherry_size` (`src/exe_reader.cpp:144-145`) but **not** used to flag DynRPG specifically — many Cherry patches carry it. Treat as "Cherry-patched", not "DynRPG". |

**EasyRPG mapping.**

| Aspect | Value | Anchor |
|---|---|---|
| `Game_ConfigGame` flag | `patch_dynrpg` (bool, default false) | `src/game_config_game.h:44` — ini `[Patch] DynRPG`, key constructed `{ "DynRPG", "", "Patch", "DynRPG", false }` |
| CLI flag | `--patch-dynrpg` / `--no-patch-dynrpg`; legacy multi-form `--patch dynrpg` | `src/game_config_game.cpp:111-112,183-184`; help text `src/player.cpp:1500` ("Enable support of DynRPG patch by Cherry (very limited).") |
| ini parse | `patch_dynrpg.FromIni(ini)` | `src/game_config_game.cpp:223` |
| Autodetect anchor | `dynloader.dll` → `patch_dynrpg.Set(true)` + debug warning | `src/player.cpp:848-851` |
| Predicate | `Player::IsPatchDynRpg()` | `src/player.h:510-516` (also honors per-savegame runtime flag, §8) |

**Precedence.** Setting any patch explicitly (ini or CLI) sets `game_config.patch_override`, which
**disables all DLL auto-detection** including the `dynloader.dll` sniff (the detection block is
guarded by `if (!game_config.patch_override)`, `src/player.cpp:843`). `--no-patch` locks
`patch_dynrpg` off entirely (`src/game_config_game.cpp:92`). `RPG_RT.ini` is **not** consulted. See
[easyrpg-extensions.md](easyrpg-extensions.md) for the override mechanics.

On detection EasyRPG emits a deliberate caveat to the **debug log** (`Output::Debug`):
*"This game uses DynRPG. Depending on the plugins used it will not run properly."*
(`src/player.cpp:850`). Upstream PR <https://github.com/EasyRPG/Player/pull/2283> moved toward
surfacing this to the user rather than only the log.

## 5. Event commands

DynRPG adds **no numeric event-command codes.** Unlike Maniac Patch (codes 3001–3032,
[maniac-patch-commands.md](maniac-patch-commands.md)) and EasyRPG's own 2002–2058, DynRPG dispatches
entirely through the standard **`Comment` (12410) / `Comment_2` (22410)** commands whose string
begins with `@`. There is therefore exactly one logical "command": **the comment-command call.**

### 5.1 Comment-command syntax (`@command`)

A comment line of the form:

```
@function_name arg1, arg2, "a string arg", token42, $1
```

invokes the plugin function `function_name` with the listed arguments. Rules (verified against
EasyRPG's parser `DynRpg::ParseCommand`, `src/game_dynrpg.cpp:193-368`, which mirrors Cherry's spec):

| Element | Rule | Anchor |
|---|---|---|
| Leading sigil | The comment **must** start with `@`; otherwise it is a normal comment and ignored. | `game_dynrpg.cpp:205-208` |
| Function name | Everything from `@` up to the first space; **lower-cased**. Empty name → warning, abort. | `game_dynrpg.cpp:234-239,263-270` |
| Argument separator | Comma. Consecutive/leading commas yield empty-string args. | `game_dynrpg.cpp:286-318` |
| **String arg** | Wrapped in `"`. A literal `"` is written as `""` (doubled). Whitespace preserved inside. | `game_dynrpg.cpp:338-353` |
| **Number arg** | A bare float/int token. (Parsed lazily per-function as `int`/`float`; all args are stored as strings and converted on demand — `ParseArgs<T...>`, `game_dynrpg.h:110-124`.) | — |
| **Token arg** | Any unquoted run; **whitespace stripped**; lower-cased unless it is a substitutable reference (below). | `game_dynrpg.cpp:319-361`, `ParseToken` `game_dynrpg.cpp:96-175` |
| Multi-line concatenation | If the next command is another `Comment_2` (22410) whose string does **not** start with `@`, its text is appended — letting one logical call span several comment lines. | `game_interpreter.cpp:2078-2086` |

### 5.2 Token substitution

Within a **token** argument (regex `[NT]?V+[0-9]+`), prefixes are resolved before the function sees
the value (`ParseToken`, `game_dynrpg.cpp:110-175`):

| Token form | Resolves to | Notes |
|---|---|---|
| `V<n>` | value of game **variable** `n` | Chains right-to-left: `VV5` = `Var[Var[5]]`. |
| `N<n>` | **actor name** of actor id `n` | Must be the outermost prefix; invalid id → warning, empty. |
| `T<n>` | **Maniac string variable** `n` | **EasyRPG extension, not original DynRPG.** Only active when `Player::IsPatchManiac()` is also on (`game_dynrpg.cpp:137-139,159-163`). See [maniac-patch.md](maniac-patch.md). |

### 5.3 Vararg references (`$n`)

Inside a function's *vararg* string (resolved by `DynRpg::ParseVarArg`, `game_dynrpg.cpp:43-93`),
`$1`…`$9` interpolate the 1st…9th argument **after** the vararg's own index, and `$$` is a literal
`$`. Out-of-range `$`-refs warn and abort. This is how e.g. `@easyrpg_output` builds a message from
later args.

**RPG_RT quirks to replicate.** (1) Function names and bare tokens are **case-insensitive**
(lower-cased). (2) Whitespace inside unquoted tokens is silently removed. (3) An unterminated `"`
string is treated as if terminated at end-of-line (`game_dynrpg.cpp:250-253`). (4) On the native
patch, an unknown `@command` is simply ignored (no plugin claims it); EasyRPG instead emits
`Output::Warning("Unsupported DynRPG function: {}")` (`game_dynrpg.cpp:394`) — louder than the
original, which matters only for log noise.

## 6. Modified baseline commands

DynRPG does **not** alter the parameter layout of any standard RM2k3 event command. Its only
interaction with the baseline command set is **hijacking `Comment`/`Comment_2`** when the string
starts with `@` (§5). Everything else a DynRPG game does to baseline commands is done by an
individual plugin's runtime hooks (e.g. a plugin can intercept picture or battle behavior from
native code), which is plugin-specific and out of scope for the command table. This section is
retained per template; the substantive content lives in §5 (comment commands) and §7
(QuickPatches, which *can* poke command-handler bytes directly).

## 7. File-format changes

DynRPG changes **no LCF chunks.** It adds standalone files and an ini section.

| Artifact | Container | Purpose | liblcf coverage |
|---|---|---|---|
| `dynloader.dll` | DLL | Startup loader injected into `RPG_RT.exe` | N/A (not a data file) |
| `DynPlugins/*.dll` | native DLLs | The plugins themselves | N/A — **EasyRPG never reads these** |
| `DynRPG.ini` | INI text | Per-plugin config + `[QuickPatches]` | Not parsed by liblcf or EasyRPG (no QuickPatch support; see §9) |
| `SaveXX.dyn` | binary sidecar | Per-plugin save state, written next to `SaveXX.lsd` | **Implemented by EasyRPG** (`game_dynrpg.cpp`), see §7.2 |

### 7.1 `DynRPG.ini` and QuickPatches

`DynRPG.ini` configures plugins and hosts a **`[QuickPatches]`** section: a text format for
applying memory pokes to the running `RPG_RT.exe` without a separate IPS. Each entry is
`Name=address,bytes…`, where each value is interpreted by sigil
(<https://rpg-maker.cherrytree.at/dynrpg/patch.html>):

| Sigil | Meaning |
|---|---|
| (hex pair, e.g. `4A`) | raw byte written verbatim |
| `%` prefix (e.g. `%50`) | 8-bit **decimal** value |
| `#` prefix (e.g. `#1000`) | 32-bit **decimal** value (little-endian) |

QuickPatches became a de-facto micro-patch distribution format in the RM2k3 1.08 scene — many of the
behavioral patches catalogued in [runtime-micro-patches.md](runtime-micro-patches.md) circulate as
QuickPatch snippets. **EasyRPG does not implement QuickPatches** (they are raw x86 pokes against a
binary EasyRPG does not run); a game relying on one needs the equivalent behavior re-coded as a
`Game_ConfigGame` flag instead. (UNVERIFIED: exact ini key grammar edge-cases — taken from Cherry's
patch page, not re-derived from `dynloader.dll`.)

### 7.2 `SaveXX.dyn` sidecar save

When a DynRPG game saves to slot *XX*, plugins persist state to a sibling file `SaveXX.dyn`
(EasyRPG: `Save` + zero-padded slot + `.dyn`, `game_dynrpg.cpp:398-410`). EasyRPG's faithful
reimplementation of the container format:

| Field | Bytes | Meaning |
|---|---|---|
| Magic | 8 | ASCII `DYNSAVE1` (`game_dynrpg.cpp:432,495-497`) |
| Per plugin: id length | 4 | uint32, **byte-swapped** (stored big-endian on disk via `SwapByteOrder`) — length of identifier |
| Per plugin: identifier | *len* | plugin identifier string (e.g. `EasyRpgPlugin`, `DynTextPlugin`) |
| Per plugin: chunk length | 4 | uint32 byte-swapped — length of the plugin's blob |
| Per plugin: chunk data | *len* | plugin-defined payload |

On load, each chunk is routed to the plugin whose `GetIdentifier()` matches; chunks with no matching
plugin are **skipped** (`game_dynrpg.cpp:437-477`). EasyRPG writes/reads `.dyn` alongside the LSD:
saved from `Scene_Save` (`src/scene_save.cpp:159`), loaded from `Scene_Map` on game load
(`src/scene_map.cpp:81`). Both are no-ops unless `Player::IsPatchDynRpg()` (`game_dynrpg.cpp:413,480`).
(Note: the original native `.dyn` layout is plugin-defined; EasyRPG's two HLE plugins use their own
payloads — TextPlugin serializes its text objects, EasyRpgPlugin stores a 4-byte Player savegame
version. Cross-engine `.dyn` compatibility with the native patch is therefore **not** guaranteed and
not a goal.)

## 8. Runtime behavior changes

DynRPG itself imposes no new engine limits, resolution, or escape codes — all such changes come from
**specific plugins**. The structural facts EasyRPG reproduces:

- **Comment-command dispatch** runs inside the normal interpreter, so commands execute synchronously
  in event order (`Game_Interpreter::CommandComment` → `HandleDynRpgScript`,
  `game_interpreter.cpp:2106-2115,2057-2091`).
- **Per-frame plugin tick.** Registered HLE plugins get an `Update()` each frame, driven from the
  map spriteset (`src/spriteset_map.cpp:118` → `Game_DynRpg::Update` → each plugin's `Update()`,
  `game_dynrpg.cpp:515-519`). TextPlugin uses this to keep its text z-ordered above its bound
  picture.
- **Plugin registration is lazy and gated** (`Game_DynRpg::InitPlugins`, `game_dynrpg.cpp:177-191`):
  `EasyRpgPlugin` registers when DynRPG **or** EasyRPG extensions are active; `TextPlugin` registers
  **only** under DynRPG. Under EasyRPG-extensions-without-DynRPG, only `@easyrpg_`-prefixed commands
  are accepted (`game_interpreter.cpp:2064-2069`).
- **Per-savegame override.** `IsPatchDynRpg()` first consults the savegame runtime flags
  (`patch_dynrpg_on/off`, toggled by `EasyRpg_SetInterpreterFlag` cmd 2053,
  `game_interpreter.cpp:5595-5597`), so DynRPG can be switched mid-game; see
  [easyrpg-extensions.md](easyrpg-extensions.md).
- **Async preload (web).** `dynloader.dll` is in the Emscripten preload list so detection works on
  the web build (`src/async_handler.cpp:194`); note `SaveXX.dyn` is **not** preloaded, so `.dyn`
  loading may miss on the web async path (minor).

The "very limited" caveat is fundamental: **native plugin DLLs are never executed.** Any game logic
living inside a non-HLE'd plugin's C++ simply does not run.

## 9. EasyRPG support matrix

Architecture: comment commands are intercepted in `Game_Interpreter::HandleDynRpgScript`
(`game_interpreter.cpp:2057-2091`); parsing in `DynRpg::ParseCommand`/`ParseToken`
(`game_dynrpg.cpp:96-368`); dispatch walks the registered plugin list, first plugin to claim the
function wins (`Game_DynRpg::Invoke`, `game_dynrpg.cpp:383-396`). The base class is `DynRpgPlugin`
(`game_dynrpg.h:155-172`; virtuals `Invoke`/`Update`/`Load`/`Save` at `game_dynrpg.h:162-165`).

| Feature | Status | Implementing `file:line` | Notes / release |
|---|---|---|---|
| Comment-command parser (`@func`, strings, tokens, `V/N/T`, `$n` varargs) | **FULL** | `game_dynrpg.cpp:43-368` | `T` token = EasyRPG add-on (needs Maniac) |
| Multi-line `@command` concat | FULL | `game_interpreter.cpp:2078-2086` | |
| `SaveXX.dyn` container (`DYNSAVE1`) read/write | FULL | `game_dynrpg.cpp:398-513` | |
| Native DLL plugin loading | **WONTFIX (by design)** | — | "plugins cannot be executed directly and must be reimplemented" `game_dynrpg.h:127-129` |
| QuickPatches (`DynRPG.ini [QuickPatches]`) | **MISSING** | — | raw x86 pokes; use a `Game_ConfigGame` flag instead |
| Engine bugfixes bundled by the patch | **N/A** | — | EasyRPG's runtime is independent; it doesn't carry RPG_RT 1.08 bugs to fix |
| **Plugin: DynText (Cherry)** → `TextPlugin` | **PARTIAL** (HLE) | `dynrpg_textplugin.cpp` | See §9.1 |
| **Plugin: `EasyRpgPlugin`** (EasyRPG-native helpers) | **FULL** (HLE) | `dynrpg_easyrpg.cpp` | See §9.2 |
| All other plugins (Kazesui Text*, PepsiOtaku Emporium, RPGSS, system_opengl, DynParams, …) | **MISSING** | — | not re-implemented; §9.3 |

*Kazesui's "Text Plugin" and Cherry's "DynText" are commonly conflated; EasyRPG's `TextPlugin`
implements the **DynText** function set (`write_text`, …). See §9.3.

### 9.1 `TextPlugin` (HLE of DynText) — functions

Identifier `DynTextPlugin` (`dynrpg_textplugin.h:26`). Draws arbitrary text bound to a picture id;
text floats one z-level above its picture (`SetPictureId` → `SetZ(sprite z + 1)`,
`dynrpg_textplugin.cpp:91-104`). Functions (`Invoke`, `dynrpg_textplugin.cpp:402-419`):

| Function | Args | Behavior | Anchor |
|---|---|---|---|
| `write_text` | `id, x, y, text [, fixed][, color][, pic_id]` | Create a text object keyed by `id`; `fixed`="fixed" pins to map coords; optional color index & bound picture id | `:262-299` |
| `append_line` | `id, text` | Add a new line | `:301-317` |
| `append_text` | `id, text` | Append to the last line | `:319-335` |
| `change_text` | `id, text, color` | Clear then set; `color`≠"end" sets color index | `:337-358` |
| `change_position` | `id, x, y` | Move | `:360-377` |
| `remove_text` | `id` | Clear that object's text (silent if absent) | `:379-395` |
| `remove_all` | — | Drop all text objects | `:397-400` |

Text supports inline command codes via `CommandCodeInserter` (`:182-218`): `\I[n]`/`\i[n]` item
name/desc, `\T[n]`/`\t[n]` skill name/desc, `\X[id]`/`\x[id]` interpolate another DynText object's
first line. Persists to `.dyn` as a comma-delimited 8-field record per object; **commas inside text are escaped
to byte `0x01`** and multiple lines joined with `\n` (per-object `Save`, `:138-164`; plugin `Load`
`:431-492`, plugin `Save` `:494-508`). Note the on-disk field *order differs between write and read*:
`Save` writes `x,y,texts,color,id,255,fixed,pic_id` (`:140-153`) but `Load` parses positionally as
`x,y,texts,color,[transparency:ignored],fixed,pic_id,id` (`:446-491`) — i.e. EasyRPG's own writer and
reader disagree on where `id` and the trailing fields sit (see §11).
**Known fidelity gaps:** a hardcoded `+2` y-offset copies an observed quirk of the official plugin
("For unknown reasons the official plugin has an y-offset of 2", `:124-128`); font is always
`Font::Default()` (`:230`); only one color per object.

### 9.2 `EasyRpgPlugin` — functions

Identifier `EasyRpgPlugin` (`dynrpg_easyrpg.h:32`). EasyRPG-native helpers, *not* part of the
original DynRPG SDK; intended for testing and for `@easyrpg_*` use without DynRPG
(dispatch `Invoke`, `dynrpg_easyrpg.cpp:91-100`). The comment sigil `@` is consumed by the parser,
so dispatch matches the **bare** function name (`func == "call"`, etc.), not `@call`:

| Comment form | Dispatch name | Args | Behavior | Anchor |
|---|---|---|---|---|
| `@call` | `call` | `func_name, …` | RPGSS-style **indirection**: invoke another plugin function named at runtime, forwarding `args.subspan(1)` | `:51-68` |
| `@easyrpg_output` | `easyrpg_output` | `mode, msg…` | Log the `msg` vararg (`ParseVarArg` from index 1) at level `debug`/`info`/`warning`/`error`; an unknown `mode` is silently dropped | `:27-49` |
| `@easyrpg_add` | `easyrpg_add` | `var, a, b, …` | `Var[var] = a + b + …` (sums args from index 1) | `:70-89` |

Save chunk = 4-byte `PLAYER_SAVEGAME_VERSION`, byte-swapped (`Save`, `:113-122`); `Load` only logs the
stored version (`:102-111`).

### 9.3 Plugins NOT re-implemented (native-only in the wild)

Each of these is a native DLL EasyRPG cannot run; a game depending on its **logic** will misbehave
under EasyRPG (graphics/text-only plugins degrade more gracefully than ones with engine hooks):

| Plugin | Author | Provides | HLE'd? |
|---|---|---|---|
| **DynText** | Cherry | on-screen text bound to pictures | **Yes** → `TextPlugin` (§9.1) |
| **Text Plugin** | Kazesui | draw arbitrary text on screen | No (function set overlaps DynText; not separately implemented). <https://rpgmaker.net/engines/rt2k3/utilities/12/> |
| **DynRPG Plugin Emporium** (Advanced/Faster ATB, ATB Overhaul, Order/Quit Switch, Extra Keys, Save Detector/Delete/Cleanup, Menu Transition Tweaks, Global Save Data, Playtime Clock, Battle Layout Changer, Hero Rename, Display Options, Provoke Skill, MP/Stat Popup, Store IDs in Battle, Auto Switches w/ Var Import/Export, Sound & Music Player, Epic Run Key) | PepsiOtaku | battle/menu/input/audio overhauls | No. <https://rpgmaker.net/forums/topics/13468/> |
| **system_opengl** | rewking | OpenGL renderer + video options | No. <https://github.com/andrew-r-king/system_opengl> |
| **RPGSS** (RPG Script System) | — | **Lua** scripting via DynRPG | No (note `@call` mimics RPGSS-style indirection only). <https://rpgmaker.net/engines/rt2k3/utilities/204/> |
| **DynParams** | — | extra command parameters | No. <https://rpgmaker.net/engines/rt2k3/utilities/79/> |
| **RPG Hacker's DynPlugins** (e.g. DynRunningScript) | RPG Hacker | misc | No. <https://www.rpg-hacker.de/?page_id=221> |

Adding another plugin = subclass `DynRpgPlugin`, implement `Invoke`/`Update`/`Load`/`Save`, and
register it in `Game_DynRpg::InitPlugins` (`game_dynrpg.cpp:177-191`) — the same pattern the two
existing HLE plugins follow. **Policy:** EasyRPG will only HLE a plugin whose behavior is understood
and verifiable; arbitrary-DLL execution is out of scope (security + portability + the project's
no-binary-execution stance).

## 10. Test assets

- (RE-pending) A minimal DynRPG 1.08 project bundling `dynloader.dll` + a `DynPlugins/DynText.dll`
  exercising `write_text`/`append_line` — stresses the comment-command parser and `TextPlugin`.
- Games shipping the Kazesui/DynText plugin are the most common HLE-positive corpus; mine the
  library at `c:\rg\easyrpg_library` for folders containing `dynloader.dll` + `DynPlugins/`.
- Cross-ref: any [docs/games/](../games/) entry whose engine is RM2k3 **1.08** with a `CHERRY`
  section and `dynloader.dll`. (None currently catalogued; Beloved Rapture uses Maniac Patch, not
  DynRPG — see [../games/beloved-rapture.md](../games/beloved-rapture.md).)

## 11. Open questions

- (RE-pending) Exact `DynRPG.ini`/`[QuickPatches]` grammar and the full list of plugins that ship
  QuickPatch snippets — needed if EasyRPG ever maps common QuickPatches to config flags.
- (RE-pending) Native `.dyn` per-plugin payload layouts for DynText/Emporium plugins, to decide
  whether cross-engine save interop is feasible (currently EasyRPG's `.dyn` is self-consistent only).
- **Apparent bug:** `TextPlugin`'s `.dyn` writer (`dynrpg_textplugin.cpp:140-153`) and reader
  (`:446-491`) disagree on the field order of a text record — `Save` puts `id` in field 4 and
  `pic_id` last; `Load` expects `id` last and `pic_id` in field 6. Re-saved DynText state may not
  round-trip its `id`/`pic_id` correctly under EasyRPG. Needs a test asset to confirm and, if real,
  an upstream issue.
- Which Emporium/RPGSS-dependent games exist in the wild and warrant additional HLE plugins
  (prioritize by corpus frequency).
- Detection robustness: should a `CHERRY` section + RM2k3 1.08 without `dynloader.dll` be flagged at
  all? Currently no. (Cf. EasyRPG issue <https://github.com/EasyRPG/Player/issues/1182> on
  byte-signature patch detection.)
- Whether to surface the "DynRPG plugins may not run" warning to the player UI rather than the debug
  log (upstream PR <https://github.com/EasyRPG/Player/pull/2283> moved in this direction).

## 12. References

Primary:
- DynRPG patch page (target version, mechanism, QuickPatches, bundled bugfixes) —
  <https://rpg-maker.cherrytree.at/dynrpg/patch.html>
- DynRPG SDK / docs hub — <https://rpg-maker.cherrytree.at/dynrpg/>
- Cherry's source — <https://github.com/CherryDT/DynRPG>; community fork —
  <https://github.com/rewrking/DynRPG>, <https://www.rewking.com/dynrpg/>
- Archived installer — <https://archive.org/details/rm2k3_dynrpg_patch>

EasyRPG source (this tree, `/home/john/Player`):
- `src/game_dynrpg.{h,cpp}` — comment-command parser, plugin host, `.dyn` save format
- `src/dynrpg_easyrpg.{h,cpp}` — `EasyRpgPlugin` (`@call`, `@easyrpg_output`, `@easyrpg_add`)
- `src/dynrpg_textplugin.{h,cpp}` — `TextPlugin` (DynText HLE)
- `src/game_interpreter.cpp:2057-2115` — `HandleDynRpgScript`, `CommandComment`
- `src/player.cpp:848-851`, `src/player.h:510-516` — detection + predicate
- `src/game_config_game.{h,cpp}` — `patch_dynrpg` config/CLI/ini
- `src/exe_reader.cpp:144-145` — `CHERRY` section sizing

Plugin ecosystem:
- Kazesui Text Plugin — <https://rpgmaker.net/engines/rt2k3/utilities/12/>,
  thread <https://rpgmaker.net/forums/topics/11004/>
- PepsiOtaku DynRPG Plugin Emporium — <https://rpgmaker.net/forums/topics/13468/>
- system_opengl — <https://github.com/andrew-r-king/system_opengl>
- RPGSS — <https://rpgmaker.net/engines/rt2k3/utilities/204/>; DynParams —
  <https://rpgmaker.net/engines/rt2k3/utilities/79/>; RPG Hacker plugins —
  <https://www.rpg-hacker.de/?page_id=221>

Cross-links: [README.md](README.md) (registry) · [official-versions.md](official-versions.md)
(RM2k3 1.08 baseline) · [maniac-patch.md](maniac-patch.md) (the 2k3-fork EasyRPG supports deeply) ·
[runtime-micro-patches.md](runtime-micro-patches.md) (QuickPatch-distributed behaviors) ·
[easyrpg-extensions.md](easyrpg-extensions.md) (`@easyrpg_*`, override precedence, runtime flags).
