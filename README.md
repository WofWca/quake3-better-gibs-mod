# quake3-better-gibs-mod

Quake III Arena mod that makes gibs' physics more realistic and satisfying.

https://github.com/user-attachments/assets/a5a8d0a2-a835-4b70-b4c9-d1e0161a348b

https://github.com/user-attachments/assets/23454b69-2ec3-4722-af26-2030e9c865b4

Namely, with this mod gibs' speed and direction
depends on the direction and the amount of damage you inflicted
on the poor opponent.

This mod also fixes a few gibs-related bugs,
e.g. the fact that in the original game the shotgun gibs very rarely
(namely it only gibs when you aim at the enemy's feet).
See the individual `.patch` files.

And yes, if you simply want **_MOAR GIBS_**,
we got you covered: `\set cg_gibs 10; set cg_gibsExtraRandomVelocity 1000` (or however much you need instead of `10`):

![Screenshot of 100 pieces of gibs flying off](https://github.com/user-attachments/assets/78f975ca-a204-4e87-867f-1f9118869893)

## Usage

The installation steps for this mod are the same as for almost every other mod.

1. Download the mod file
   - For vanilla (original) Quake III Arena:

     [`pak999-vanillaq3-baseq3-better-gibs-mod.pk3`](https://github.com/WofWca/quake3-better-gibs-mod/releases/latest/download/pak999-vanillaq3-baseq3-better-gibs-mod.pk3)

     This file is also compatible with ioquake3 and engines based on it,
     so download this one if in doubt.

   - For [ioquake3](https://github.com/ioquake/ioq3)
     and other ioquake3-based games:

     [`pak999-ioquake3-baseq3-better-gibs-mod.pk3`](https://github.com/WofWca/quake3-better-gibs-mod/releases/latest/download/pak999-ioquake3-baseq3-better-gibs-mod.pk3)

   - For vanilla (original) Quake III: Team Arena:

     [`pak999-vanillaq3-missionpack-better-gibs-mod.pk3`](https://github.com/WofWca/quake3-better-gibs-mod/releases/latest/download/pak999-vanillaq3-missionpack-better-gibs-mod.pk3)

   - For ioquake3 Quake III: Team Arena:

     [`pak999-ioquake3-missionpack-better-gibs-mod.pk3`](https://github.com/WofWca/quake3-better-gibs-mod/releases/latest/download/pak999-ioquake3-missionpack-better-gibs-mod.pk3)

   - There is also a version of the
     ["baseq3a" mod](https://github.com/ec-/baseq3a)
     with the "better gibs" mod on top:

     [`pak8a-baseq3a-mod-with-better-gibs.pk3`](https://github.com/WofWca/quake3-better-gibs-mod/releases/latest/download/pak8a-baseq3a-mod-with-better-gibs.pk3)

2. Put the file alongside the `pak0.pk3` file of your game, usually  
   `C:\Programs (x86)\ioquake3\baseq3` or  
   `C:\Programs (x86)\Steam\steamapps\common\Quake 3 Arena\baseq3`.
   For Team Arena the final folder is `missionpack` instead of `baseq3`.

   The directory structure will look like this:

   ```tree
   Quake 3 Arena
   ├── baseq3
   │   ├── pak0.pk3
   │   ├── pak1.pk3
   │   ├── pak2.pk3
   │   ├── pak3.pk3
   │   ├── pak4.pk3
   │   ├── pak5.pk3
   │   ├── pak6.pk3
   │   ├── pak7.pk3
   │   ├── pak8.pk3
   │   └── pak999-baseq3-better-gibs-mod.pk3
   ├── missionpack
   │   ├── pak0.pk3
   │   └── pak999-missionpack-better-gibs-mod.pk3
   └── quake3.exe
   ```

   Again, if you don't have `missionpack`, you don't need to create it.

3. Start the game!

### Usage details

This mod is not compatible with mods
that modify the client-side logic of the game, i.e. `vm/cgame.qvm`.

Some (but not all) features of this mod
don't require it to be installed on the server,
so you can enjoy it on basically any server
(except the ones with `sv_pure 1`).

### New CVARs

- `cg_oldGibs` and `g_oldGibs`: set to 0 to disable most (but not all!)
  of this mod's features
- `cg_gibs` is now non-binary,
  (acting the same as `cg_gibfactor` in the GibHappy mod!):
  set to 1.3 to launch 3 more pieces of gibs,
  or to 0.5 to half the amount of gibs
- `cg_gibsInheritPlayerVelocity`
- `cg_gibsPiecesFromKnockback`
- `cg_gibsRandomVelocityFromKnockback`
- `cg_gibsVerticalVelocityFromKnockback`
- `cg_gibsExtraRandomVelocity`
- `cg_gibsExtraVerticalVelocity`
- `cg_gibsBounceFactor`
- `cg_gibsBounceFactorRandomness`
- `cg_gibsRotationFactor`
- `cg_gibsBetterCameraOnGib`
- `cg_bounceMarksMinImpactSpeed`
- `cg_bounceSoundMinImpactSpeed`
- `g_gibsMissileDirectionKnockbackWeight`
  As you can tell by the `g_` prefix, this variable techically changes gameplay.
  However, this only affects the direction in which the player's camera will fly
  when they get gibbed.
- `g_gibsNewEvGibPlayerParmProtocol`: this mod technically modifies
  the network protocol, namely the `EV_GIB_PLAYER` event's argument.
  Now we pass the knockback speed as the argument,
  instead of the killer's entity number.
  But in vanilla Quake 3 the argument is anyway unused so it is fine.
  However, if some other mod does rely on this argument,
  you can set this CVAR to 0 to revert this change.
- `g_gibsOnCollisionMinSpeed`
- `g_gibsOnCollisionBaseDamage`
- `g_gibsOnCollisionAffectLivePlayers`
  For fooling around (off by default).
- `g_gibsOnCollisionInheritPlayerVelocity`
- (maybe I've missed some: search the code for `+vmCvar_t`)

Another way to (almost) restore the old behavior:

```cfg
// cg_oldGibs 0
// g_oldGibs 0
set cg_gibs 1.0
set cg_gibsInheritPlayerVelocity 0
set cg_gibsExtraRandomVelocity 250
set cg_gibsRandomVelocityFromKnockback 0
set cg_gibsExtraVerticalVelocity 250
set cg_gibsBounceFactor 0.6
set cg_gibsRotationFactor 0
set cg_gibsBetterCameraOnGib 0
set g_gibsMissileDirectionKnockbackWeight 1
set g_gibsNewEvGibPlayerParmProtocol 0
// Can't "restore" these
// cg_bounceMarksMinImpactSpeed
// cg_bounceSoundMinImpactSpeed
```

### More fun!

Add the following to the `baseq3/autoexec.cfg` file,
then enter a game and press `F8`.

```cfg
exec better-gibs-cfgs/autoexec.cfg
bind F8 "vstr gibsFunCmd"
```

## How it works

Nothing too special.
The most important feature that mod introduces
is that gibs inherit player velocity (`cg_gibsInheritPlayerVelocity`).
And player velocity data is already available client-side.
Players velocity gets changed as the player receives damage.
More damage = more velocity.
That's it.

## Integrating into other mods

As was said, this mod modifies `vm/cgame.qvm` and `vm/qagame.qvm`,
and thus it cannot run alongside other mods that modify those files.  
Thus, if you want to run this mod together with another mod
that modifies those files,
you must merge the source code of that mod
with the source code of this mod,
and compile the `.qvm` files.

To do that:

```bash
git clone https://github.com/WofWca/quake3-better-gibs-mod.git
cd my-mod-dir
git am ../quake3-better-gibs-mod/patches/*.patch
```

This should apply the patches to your mod's code, by creating git commits.
There might be conflicts, which should not be too hard to resolve.
Then apply the copyright notice, `Copyright (C) 2025, 2026  WofWca`,
wherever you have them.

However, it might be simpler to resolve the conflicts
at once and not for each commit separately.
To achieve that, you can apply the patches to one of the earlier commits
of your mod, and then merge the resulting branch
into the main branch of your mod.

Also see the `ioq3` branch in this repo,
which already applied these patches on top of ioquake3.
In fact, the `.patch` files have been generated from that branch,
with `git format-patch --output-directory=./patches --no-numbered ioquake/main..ioq3`.
To build a vanillaq3-compatible version in CI,
replace `COMMAND ${Q3ASM} -o ${QVM_FILE} ${ASM_FILES}`
with `COMMAND ${Q3ASM} -vq3 -o ${QVM_FILE} ${ASM_FILES}`.
See [instructions](https://github.com/ioquake/ioq3/blob/3ef30e759eac79f7e3f98ee495accc76c9807f79/README.md?plain=1#L493-L499).

Some of the patches of this mod have already been integrated
into the ["baseq3a" mod](https://github.com/ec-/baseq3a).
The `pak8a-baseq3a-mod-with-better-gibs.pk3` file was built
from the `baseq3a` branch of this repo.

## Similar projects

- [GibHappy](https://web.archive.org/web/20051226113935/http://www.planetquake.com/gibfactory/),

  Also downloadable [on lvlworld.com](https://www.lvlworld.com/mod/id:45).

  GibHappy doesn't have directional gibs, but has a Gib Gun!!

- [Quake 3 Movie Maker's Edition (q3mme)](https://github.com/entdark/q3mme)

  You might have seen it in action while watching some Quake movies.

  (As I discovered after already having made my mod),
  q3mme implements directional gibs, flexibly scripted
  with a config file.  
  The direction is configurable to be either the direction
  from the killer to the gibbed player,
  or the gibbed player's velocity (as in our mod).  
  See
  - <https://github.com/entdark/q3mme/blob/09b42e6b69367538961b3e33ed6c5acb9263d7bd/trunk/code/cgame/cg_event.c#L1073-L1103>
  - <https://github.com/entdark/q3mme/blob/09b42e6b69367538961b3e33ed6c5acb9263d7bd/trunk/code/cgame/cg_effects.c#L378-L392>
  - <https://github.com/entdark/q3mme/blob/09b42e6b69367538961b3e33ed6c5acb9263d7bd/trunk/files/scripts/base_player.fx#L93-L144>

- [wolfcamql: quakelive/quake3 demo player](https://github.com/brugal/wolfcamql)

  Implements Quake Live -looking "gibs" (sparks) that have directionality
  (but also still supports Quake 3 gibs),
  a bunch of `cg_gib` CVARs
  (most of which seem to only apply to Quake Live sparks?),
  and it also supports q3mme scripts.  
  (As you can tell by the name of the project,
  it supports Quake Live replays, where `eventParm` of `EV_GIB_PLAYER`
  is direction.)
  - <https://github.com/brugal/wolfcamql/blob/f33dd8819efc3f837b84ade5aff849acd7503ba0/code/cgame/cg_event.c#L3723-L3786>
  - <https://github.com/brugal/wolfcamql/blob/cdd9d29cd7c20d7d98f17a58e2bee4186ed4bcd3/code/cgame/cg_effects.c#L1091-L1185>
  - <https://github.com/brugal/wolfcamql/blob/cdd9d29cd7c20d7d98f17a58e2bee4186ed4bcd3/code/cgame/cg_effects.c#L968-L1049>
  - <https://github.com/brugal/wolfcamql/blob/f33dd8819efc3f837b84ade5aff849acd7503ba0/code/cgame/cg_main.c#L1387-L1407>
  - <https://github.com/brugal/wolfcamql/blob/77c7c5752cecaeb3f1d7467e205bd2076f39ea40/package-files/wolfcam-ql/scripts/q3mme.fx#L214-L370>

- [Wolfenstein: Enemy Territory](https://github.com/id-Software/Enemy-Territory),
  or [ET: Legacy](https://github.com/etlegacy/etlegacy)

  Implements proper initial position of individual gibs
  matching the player model's parts (as we try to do in this mod),
  and directionality.
  - <https://github.com/etlegacy/etlegacy/blob/61dd79d9781d081e679a46c7d03aabbba0cb51e0/src/cgame/cg_event.c#L2604-L2613>
  - <https://github.com/etlegacy/etlegacy/blob/61dd79d9781d081e679a46c7d03aabbba0cb51e0/src/cgame/cg_effects.c#L573-L746>

- Maybe you could find other
  [id Tech 3 -based games](https://en.wikipedia.org/wiki/Id_Tech_3)
  or mods for those games that modify `CG_GibPlayer`?  
  id Tech forks are truly innumerable...  
  One can start with
  <https://github.com/search?q=%22CG_GibPlayer%22+path%3Acg_effects.c&type=code>,
  and hide identical files.

## License

These patches are dual-licensed under either
the ["QIIIA Game Source License"](./QIIIA%20Game%20Source%20License.txt),
or the ["GPL-2.0"](./COPYING.txt),
at your option.
That is, you may apply these pathes to any other project
based on Quake III Arena source code, be it the GPL or pre-GPL release.

Some more about Quake III licensing here:

- <https://github.com/ioquake/ioq3?tab=readme-ov-file#standalone-game-licensing>
- <https://github.com/ec-/baseq3a/pull/59>
