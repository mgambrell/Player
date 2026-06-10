# Maniac Patch — build registry & identity convention

Maniac Patch ships as a **whole replacement `RPG_RT.exe`** regenerated for every release (not a
delta patch — this avoids antivirus false positives from in-place byte patching), in
per-language × per-refresh-mode variants. There is no single version number embedded in a
predictable place across the patch's whole history, and at least one shipping build (the one
**Beloved Rapture** uses) was never published. This document defines how we *name* and
*fingerprint* an arbitrary Maniacs runtime so it can be referred to unambiguously across the
specs and the test corpus, and registers every build we have evidence for.

The convention generalizes to any fork that lacks canonical versioning (see
[§4 Applying this to other forks](#4-applying-this-to-other-forks)).

## 1. The key fact: build code = PE compile date

Maniacs build codes are `YYMMDD`. **The code equals the executable's PE/COFF `TimeDateStamp`
date** — verified across every binary we hold:

| Build | VERSIONINFO `FileDescription` | PE `TimeDateStamp` | Matches code? |
|---|---|---|---|
| 220325 (Beloved Rapture) | `RPG Maker 2003 Runtime (Maniacs, v220325, en, im)` | `0x623dd3f7` = 2022-03-25 14:38:47Z | ✅ exact |
| 211010 (en/im) | `RPG Maker 2003 Runtime` (no Maniacs suffix) | `0x61628494` = 2021-10-10 06:13:40Z | ✅ (code from provenance/changelog) |
| 241028 (en, 64-bit) | `RPG Maker 2003 Runtime (Maniacs, v241028, 64bit, en, js)` | `0x671f8b79` = 2024-10-28 13:02:49Z | ✅ exact |

Two consequences:
1. A build that **self-identifies** (VERSIONINFO `Maniacs, vNNNNNN`, a convention introduced
   between 211010 and 220325) is trivially named, and the PE timestamp independently
   corroborates that the reported code is the true compile date — so even an *unpublished* build
   like 220325 is not lying about its identity.
2. A build that **does not self-identify** (every build ≤ 211010 carries only the stock
   `RPG Maker 2003 Runtime` string) can still be dated from the PE `TimeDateStamp` alone.

⚠️ PE timestamps can be zeroed or forged. Treat a timestamp as definitive only when it is
internally consistent (a plausible date that matches the version string and/or provenance). Note
explicitly when a binary's timestamp looks unreliable.

## 2. Naming convention

Every distinct build has (a) an immutable **fingerprint** that is always recorded and (b) a
**canonical build ID** built around the PE timestamp.

### 2.1 Fingerprint (definitive, collision-proof)

Always record, regardless of how the build is named:

- **`sha256`** — the canonical key. Use the 12-hex short form **`sha12`** inline.
- **`pe_timestamp`** — COFF `TimeDateStamp`, hex + UTC datetime.
- **`arch`** — `x86` / `x64`, and file **size**.
- **`versioninfo`** — the `FileDescription` string verbatim (may be plain or empty).

The `sha256` is the ground truth; everything else is a convenience or a provenance aid.

### 2.2 Canonical build ID — `PE`-timestamp backbone + qualifiers

The PE `TimeDateStamp` is present in essentially every build, is objective, and — per §1 —
already *is* a `YYMMDD` build date in the same format Maniacs itself uses. So we make it the
backbone of the ID rather than trusting a possibly-absent or possibly-forged version string:

> **`<fork>-PE<YYMMDD>[-<lang>][-<variant>][-<arch>][-<sha12>]`**

The core token is **`PE<YYMMDD>`** — the `PE` prefix marks the date as *taken from the binary's
PE header* (not from a self-declared string), and `YYMMDD` is Maniacs' own date format, so
`PE220325` reads naturally to anyone familiar with the patch. Then concatenate whatever else
identifies the build, in this fixed order:

| Token | Source | Example | When omit |
|---|---|---|---|
| `<fork>` | which fork | `maniacs` | never |
| `PE<YYMMDD>` | PE `TimeDateStamp` date | `PE220325` | only if timestamp is zeroed/forged (then use `PE000000` and rely on `sha12` + witness) |
| `-<lang>` | VERSIONINFO / provenance | `-en`, `-jp` | if unknown |
| `-<variant>` | refresh mode / feature tag | `-im`, `-pf`, `-js` | if N/A |
| `-<arch>` | PE machine, only when notable | `-64` | for the default `x86` |
| `-<sha12>` | SHA-256 prefix | `-5f3eddd5` | in prose; **always carry it once per document** as the definitive disambiguator |

**Examples**
- Beloved Rapture's build → **`maniacs-PE220325-en-im`** (`5f3eddd5`).
- 211010 English/Immediately → **`maniacs-PE211010-en-im`** (`49c33cb9`) — note this build carries
  *no* Maniacs version string, yet the PE date names it cleanly.
- 241028 English 64-bit → **`maniacs-PE241028-en-64`** (`49ba424f`).

### 2.3 Cross-check the self-declared code; flag disagreement

When a build *does* carry a VERSIONINFO `Maniacs, vNNNNNN` string, it should equal the
`PE<YYMMDD>` date (it does for every build we hold — §1). The PE date stays authoritative; the
self-declared code is corroboration. **If they disagree, that is a red flag** (forged/zeroed
timestamp, repacked or re-stamped binary) — surface both, e.g.
`maniacs-PE181225-en (declares v220325!) (sha …)`, and trust the `sha12` over either.

### 2.4 Witness games & hash-only fallback

Record **all witness games** that ship a build as a secondary human alias (e.g. "the Beloved
Rapture build", "the Yume Nikki 16:9 build") — useful in conversation and indispensable when the
PE timestamp is unusable. If a build has neither a usable timestamp nor a version string, the
`sha12` *is* the name: `maniacs-PE000000-#5f3eddd5`, disambiguated by witness game.

### 2.5 Why the date is not, by itself, a unique key

- Maniacs releases **per-language (`en`/`jp`) × per-refresh-mode (`im`/`pf`) variants of the same
  code**, each a separately-compiled PE with its own hash and timestamp. The three 211010
  variants we hold were compiled within ~12 minutes of each other (06:02–06:14Z). The unique key
  is therefore the **`(PE-date, lang, variant, arch)` tuple — or simply the `sha256`.** This is
  why the canonical ID (§2.2) carries lang/variant and a `sha12` rather than the bare date.
- `im` = 出現条件の変更直後 ("Immediately") page-condition refresh; `pf` = per-frame. Since the
  241028 build these merged into one exe selected by `pf=1` in `RPG_RT.ini`; the 241028 build
  also carries a `js` tag (JavaScript scripting) and is the **32→64-bit transition** (`x64`).
- A re-release on the same day (e.g. the 241029 ZIP wraps the 241028 *engine* with a JP database
  fix) shares the engine timestamp — trust the timestamp/hash, not the ZIP filename.

## 3. Registry

### 3.1 Builds we hold (fully fingerprinted)

All hashes public-checked 2026-06-09: zero scanner listings, benign KADOKAWA RM2k3 runtimes →
treated CLEAN (no upload, nothing executed). Corpus root: `c:\rg\easyrpg_library`.

| Canonical build ID | sha12 | PE TimeDateStamp | arch | size | VERSIONINFO suffix | Witness games / source |
|---|---|---|---|---|---|---|
| `maniacs-PE220325-en-im` | `5f3eddd5` | 2022-03-25 14:38:47Z | x86 | 1,522,176 | `Maniacs, v220325, en, im` | **Beloved Rapture** (`beloved_rapture/`); the priority target |
| `maniacs-PE211010-en-im` | `49c33cb9` | 2021-10-10 06:13:40Z | x86 | 1,485,312 | *(plain)* | Yume Nikki 16:9 (`yumenikki_169/`, byte-identical); official ZIP `maniacs_ref_211010/` |
| `maniacs-PE211010-en-pf` | `4e6124c4` | 2021-10-10 06:06:45Z | x86 | 1,484,288 | *(plain)* | official ZIP `maniacs_ref_211010/` |
| `maniacs-PE211010-jp-im` | `d4378e68` | 2021-10-10 06:02:08Z | x86 | 1,476,608 | *(plain)* | official ZIP `maniacs_ref_211010/` |
| `maniacs-PE211010-jp-pf` | `0890160f` | 2021-10-10 06:04:40Z | x86 | 1,475,584 | *(plain)* | official ZIP `maniacs_ref_211010/` |
| `maniacs-PE241028-en-64` | `49ba424f` | 2024-10-28 13:02:49Z | x64 | 2,515,456 | `Maniacs, v241028, 64bit, en, js` | official ZIP `maniacs_ref_241029/` |
| `maniacs-PE241028-jp-64` | `7065a9dc` | 2024-10-28 12:58:01Z | x64 | 2,505,216 | `Maniacs, v241028, 64bit, jp, js` | official ZIP `maniacs_ref_241029/` |

Machine-readable copy: [`maniac-patch-builds.tsv`](maniac-patch-builds.tsv).

`maniacs-PE211010-en-im (49c33cb9)` is the **closest published predecessor** to BR's
`maniacs-PE220325-en-im (5f3eddd5)` and the reference binary for diffing the "Discord delta"
(what 220325 added on top of 211010). `maniacs-PE241028-en-64 (49ba424f)` is the 64-bit
upper-bound reference.

### 3.2 Published build timeline (from the official changelog)

`maniacs_ref_241029/docs/update.txt` (BingShan's `update.txt`, dated headers `●YYYY/MM/DD`)
records **26 published builds**, then a multi-year gap:

```
2018: 0809 1209
2019: 0119 0120 0121 0122 0217 0220 0526 0625 0630 0904 0920 1007 1020 1021 1103
2020: 0126 0128
2021: 0414 0519 0530 0809 1010
        ── public record ends here ──
        (no 2022 / 2023 entries at all)
2024: 1028   (then 241029 = JP database-localization fix on the 241028 engine)
```

The **2021-10-10 → 2024-10-28 gap** is the key structural fact: any Maniacs build in between was
distributed privately (BingShan's Discord), not on the official site. Build dates earlier than
180809 may exist but are not in this changelog.

### 3.3 Known-but-unheld builds of interest

| Display name | Status | Why it matters |
|---|---|---|
| `maniacs-PE220325-*` (BR) | **Unpublished / dev build** — in the 2021→2024 gap; only the `en-im` variant is held (BR ships it). | The priority target. Its true command surface is established by **binary RE**, not the public docs, precisely because no changelog covers it. See [maniac-patch-commands.md](maniac-patch-commands.md) and the RE notes. |
| `maniacs-PE210414` | Published; **EasyRPG's stated reference target** | EasyRPG verifies "legacy" Maniacs against this build; our 211010 is the next public build after it. (No binary held; PE date will confirm on acquisition.) |
| `maniacs-PE241028`+ newer (2025–2026) | Possible builds after 241028 | Not in the 241029 changelog snapshot; check for newer releases when revisiting. |

## 4. Applying this to other forks

The same fingerprint-first, name-by-best-available-tier approach covers the other un-versioned
forks catalogued in this directory:

- **KotatsuAkira Destiny continuation / IPS-distributed patches** ([destiny.md](destiny.md)) —
  often distributed as IPS deltas against a known base `RPG_RT.exe`; identify by `(base build,
  IPS hash)` and witness game, since the patched exe's PE timestamp tracks the *base*, not the
  patch.
- **Bootlegs & translations** ([bootlegs-translations.md](bootlegs-translations.md)) — EasyRPG
  already fingerprints these by **logo CRC32** and PE section size (`src/exe_reader.cpp`); fold
  that into the fingerprint (`logo_crc`) for those families.
- **Per-game custom runtimes** — name `<fork>-<game-slug>` and record the full fingerprint; the
  game *is* the canonical reference.

General rule: **the `sha256` (plus PE `TimeDateStamp` and, where present, `logo_crc`) is the
durable identity; the human name is a convenience that may be revised as provenance improves.**
When a build has real usage but no canonical name, associate it with its witness game and record
its fingerprint — never invent a version number it does not carry.

## 5. References

- Build changelog: `c:\rg\easyrpg_library\maniacs_ref_241029\docs\update.txt` (BingShan).
- Variant-tag semantics (`im`/`pf`/`en`/`jp`/`js`): [maniac-patch.md](maniac-patch.md) §3, and
  the official site index (https://bingshan1024.github.io/steam2003_maniacs/).
- Corpus fingerprints & malware checks: each `c:\rg\easyrpg_library\<item>\ANALYSIS.md`;
  roll-up `c:\rg\easyrpg_library\CORPUS.md`.
- EasyRPG's existing detection by VERSIONINFO `Maniacs, v` string: `src/exe_reader.cpp`,
  `src/player.cpp` (see [maniac-patch.md](maniac-patch.md) §4).
