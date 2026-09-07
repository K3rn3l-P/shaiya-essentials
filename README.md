# Shaiya Essentials (archived fork)

Fork of [Spelunkern/shaiya-essentials](https://github.com/Spelunkern/shaiya-essentials) (itself
archived), a client-side DLL for 6.4 PT client 182 modifications. Dormant since August 2025, no
active development.

## What this adds

Reconstructed several internal client engine classes (`CWorldMgr`, `CMonster`, `CNetwork`,
`CTexture`, `CWindow`, `CQuickSlot`, `CStaticText`, `CDataFile`) from the compiled binary — typed
wrappers around hardcoded function addresses recovered via disassembly. Reorganized the
reverse-engineered headers under a single `include/shaiya/include/` tree, consistent with the
other Shaiya forks on this account.

## Environment

Windows 10, Visual Studio 2022, C++23, DirectX SDK (June 2010).

## Attribution

The base client library and build prerequisites are
[Spelunkern/shaiya-essentials](https://github.com/Spelunkern/shaiya-essentials), shared as-is by
the author, no license attached.

## State

Archived, no further changes planned.
