# Shader bundle (`.snb`) format

Shader bundle version 2 is a little-endian binary format. Runtime `.snb` files
contain shader code and the reflection data required for named resource
bindings. JSON is an optional metadata-only debug representation and is not
loaded at runtime.

## Binary layout

All integer fields are unsigned 32-bit little-endian values. Offsets are
absolute byte offsets from the start of the file.

### Header (32 bytes)

| Offset | Field |
| ---: | --- |
| 0 | Magic: `SNSB` |
| 4 | Version: `2` |
| 8 | File size |
| 12 | Entry count |
| 16 | Entry table offset |
| 20 | String table offset |
| 24 | String table size |
| 28 | Reserved |

### Entry (56 bytes)

| Offset | Field |
| ---: | --- |
| 0 | Shader stage |
| 4 | Shader format |
| 8 | Code offset |
| 12 | Code size |
| 16 | Resource table offset |
| 20 | Resource count |
| 24 | Sampler count |
| 28 | Storage buffer count |
| 32 | Storage texture count |
| 36 | Uniform buffer count |
| 40 | FNV-1a 64-bit code hash, low word |
| 44 | FNV-1a 64-bit code hash, high word |
| 48 | Reserved |
| 52 | Reserved |

### Resource (16 bytes)

| Offset | Field |
| ---: | --- |
| 0 | Kind (`0`: uniform buffer, `1`: texture) |
| 4 | Binding slot |
| 8 | Name offset relative to the string table |
| 12 | Name byte length |

The resource records for each entry are contiguous. Resource names are UTF-8
bytes and are not null-terminated. Shader code begins on a 16-byte boundary.

## Debug JSON

`ShaderBundle.dumpJson(entries)` returns a readable representation containing
the version, stages, formats, code sizes and hashes, resource counts, and named
bindings. It deliberately omits shader code and base64 data. The shader compiler
example can optionally write this text beside the bundle as `<name>.snb.json`.

Version 1 binary bundles and version 1 JSON/base64 bundles remain readable for
migration, but all newly packed bundles use version 2 binary output.
