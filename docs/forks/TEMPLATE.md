<!--
TEMPLATE for docs/forks/*.md and (adapted) docs/games/*.md.
Copy this file, fill every section, delete sections that genuinely don't apply
(but say WHY in one line rather than dropping them silently).

GOAL of these specs: a competent C++ developer with no prior knowledge of this fork
should be able to (re)implement support for it in an EasyRPG-class engine using ONLY
this document. That is the bar. Prefer tables. Cite a source for every nontrivial claim:
  - external facts: an inline link (archive.org link for dead pages);
  - engine-behavior facts: a `file:line` anchor into Player/ or lib/liblcf/ (our tree),
    or a binary handler address from the RE notes (e.g. `BelovedRapture.exe FUN_004500d0`).
Keep original Japanese terms alongside translations so claims can be re-searched.
Mark every unverified claim explicitly: (UNVERIFIED), (RE-pending), or (hypothesis).
-->

# <Fork name> (<JP name 改造パッチ> / romaji)

> One-paragraph elevator description: what this fork is, who made it, what it adds,
> and why a game would use it.

**Status at a glance:** EasyRPG support = FULL / PARTIAL / STUB / NONE.
Detection = <how>. Spec confidence = HIGH / MEDIUM / LOW.

## 1. Identity
- Names / aliases (EN + JP + romaji).
- Author(s).
- License / EULA / redistribution stance (matters for shipping detection data or test assets).
- Distribution URLs (mark alive / dead / archived; give archive.org link for dead).
- Liveness: actively maintained? last release date?

## 2. Target engine builds
Exact RPG_RT version(s) and build dates the patch applies to or derives from
(e.g. "Steam RPG Maker 2003 v1.12a only"). Note if it ships a whole replacement
RPG_RT.exe vs. patching an existing one.

## 3. Version lineage
Dated build table (build id | date | headline changes | published?). Mark unpublished /
dev-only / Discord-only builds explicitly — they are the ones RE has to pin down.

| Build | Date | Headline changes | Published |
|---|---|---|---|

## 4. Detection
How to recognize a game using this fork from its files, without running it.
- Bundled DLLs (name → meaning).
- PE characteristics: section names/sizes, VERSIONINFO strings, logo/asset CRCs, byte signatures
  (cite Makerpendium offsets where applicable).
- Data-file signatures (LDB/LMU chunk presence, ini keys).
- EasyRPG mapping: the `Game_ConfigGame` flag (`lib/liblcf`/`src/game_config_game.h`),
  the CLI flag, and the autodetect anchor (`src/player.cpp:NNN`). Note precedence
  (manual flag sets `patch_override` and disables autodetect — see easyrpg-extensions.md).

## 5. Event commands
The core of the spec. One row per command code. Parameter layout must be at the
per-index level (what each `parameters[i]` means, including bitfields and op-selectors),
plus how the command's string field is used.

| Code | Name (EN / JP) | Since | String field | Parameters (per index) | RPG_RT quirks to replicate | EasyRPG status + anchor |
|---:|---|---|---|---|---|---|

For multi-op commands (an op selector in `parameters[0]`), give a nested op table.
Record observed engine bugs explicitly — faithful emulation often means replicating them.

## 6. Modified baseline commands
Standard RM2k/2k3 commands this fork extends (extra params, new modes, widened ranges,
gating by a flag/version). One subsection per affected command.

## 7. File-format changes
New or changed chunks and files. For each chunk: container (LDB/LMU/LMT/LSD), chunk ID (hex),
field type, meaning, default. New standalone files (e.g. `Save.lgs`), ini keys, folder
conventions. Note liblcf coverage (modeled? preserved-but-opaque? dropped on rewrite?).

## 8. Runtime behavior changes
Limits (picture count, variable count), timing/refresh model, text escape codes, input,
audio, rendering/resolution, save semantics. Anything not expressible as a command/chunk.

## 9. EasyRPG support matrix
Per feature: FULL / PARTIAL / STUB / MISSING / WONTFIX, with PR/issue link, the implementing
`file:line`, and a release-vs-master annotation (e.g. "master-only since #NNNN; not in 0.8.1").

## 10. Test assets
Games / minimal repro projects exercising this fork (cross-ref `docs/games/` and the corpus
at `c:\rg\easyrpg_library`). Note which feature each asset stresses.

## 11. Open questions
RE targets still unresolved; questions to put to upstream / the patch author.

## 12. References
Primary sources first. Archive links for anything that might rot.
