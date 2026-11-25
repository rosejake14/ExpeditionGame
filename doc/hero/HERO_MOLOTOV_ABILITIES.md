# Hero — Molotov Guy Abilities (Spec)

**Status:** Power (§2) and Ultimate (§3) both APPROVED 2026-11-25. C++ written in
`Abilities/MolotovAbilities.h/.cpp`, not yet compiled or PIE-tested.
**Date:** 2026-11-25
**Builds on:** `doc/hero/HERO_BOMBSHELL_ABILITIES.md` (framework Phase A: Power/Addon/Ultimate
slots, slot from grant, `bEndsImmediately`, `GetScaledDamage`, shared camera zoom).

---

## 1. Kit

| Slot | Ability | Notes |
|---|---|---|
| Main (M1) | *decide later* | Whatever `StartingWeapon` is set to. No Main slot work. |
| Power (E) | **Throw Molotov** | §2 |
| Addon (F) | *none for now* | Slot left empty. |
| Ultimate (Q) | *TBD* | Jake will describe it next. |

Upgrades and ranks remain out of scope. Every tuning value is a plain `UPROPERTY`, editable on the
GA Blueprint (`GA_Molotov_Throw`).

## 2. Throw Molotov (Power, E)

### Behaviour (all answered by Jake)

1. The player presses E. A molotov (a sphere for now) spawns just in front of the middle of the
   mesh.
2. It flies toward where the **crosshair** points, in a **gravity arc**.
3. It breaks on **anything**: floor, wall or enemy.
4. **Impact damage, direct hit only:** if it hits an enemy, that enemy takes `ImpactDamage`.
   Nothing else takes impact damage.
5. **Fire always spawns on the ground directly below the impact point**, including when it hits a
   wall or an enemy.
6. The fire burns for `FireDuration`. Every `FireTickInterval`, each enemy inside `FireRadius`
   takes `FireTickDamage`.
7. **No afterburn:** an enemy stops taking damage the moment it leaves the fire.
8. **No self-damage:** Molotov Guy is immune to his own fire.
9. **Stacks:** each fire ticks independently. An enemy in two overlapping fires takes both ticks.
10. **No cap** on fires. The cooldown limits them: at 5 s cooldown and 6 s burn, at most 2 overlap.
11. Cooldown starts on throw.

### Tuning (`UPROPERTY`, `EditDefaultsOnly`, category `Hero Ability`)

| Property | Meaning | Default | Source |
|---|---|---|---|
| `CooldownSeconds` | Inherited. | 5 s | Jake |
| `ThrowSpeed` | Launch speed, cm/s. Sets the arc's range. | 1500 | Jake |
| `ImpactDamage` | Direct-hit damage, before `DamageMultiplier`. | 50 | Jake |
| `FireRadius` | Radius of the fire patch, cm. | 500 | Jake |
| `FireDuration` | Burn time, s. | 6 | Jake |
| `FireTickDamage` | Damage per tick, before `DamageMultiplier`. | 5 | Jake |
| `FireTickInterval` | Seconds between ticks. | 0.5 (= 10 DPS) | Jake |
| `SpawnForwardOffset` | How far in front of the mesh centre it spawns, cm. | 60 | Jake |
| `FireHeight` | How far above the fire's ground point an enemy can be and still burn, cm. | 200 | Jake |
| `MolotovClass` | Projectile class to spawn. | `AMolotovProjectile` | *proposed* |
| `FireClass` | Fire patch class to spawn. | `AMolotovFire` | *proposed* |

`MolotovClass` and `FireClass` let you point the GA at Blueprint subclasses, which is where the
Niagara visuals go.

### How it works

**Aim:**
- Spawn point = the mesh's bounds centre + the actor's forward vector × `SpawnForwardOffset`.
- Aim point = a line trace (`ECC_Visibility`, ignoring the hero) from the camera along the control
  rotation, out to a long distance. If nothing is hit, use the trace end.
- Throw direction = (aim point − spawn point), normalised. Because of gravity the molotov drops
  below the crosshair over distance. That matches "flies in the direction of the crosshair".

**Spawning:**
- `SpawnActorDeferred<AMolotovProjectile>`.
- The ability sets `InitialSpeed = ThrowSpeed`, the impact damage, and the fire settings, all
  scaled through `GetScaledDamage` at throw time. Then `FinishSpawning`.
- Owner and instigator = the hero, so kills credit the player and ultimate charge still builds.

**`AMolotovProjectile` (new actor):**
- Root: `USphereComponent`, radius 10 cm. Blocks `WorldStatic`, `WorldDynamic` and `Pawn`, and
  ignores the instigator, so it can't hit the thrower.
- Visual: a `UStaticMeshComponent` using the engine's `/Engine/BasicShapes/Sphere`, scaled to
  match, with no collision.
- `UProjectileMovementComponent` with gravity scale 1 and no bounce.
- **`OnHit`:**
  1. If the hit actor is a pawn other than the instigator, apply `ImpactDamage` with
     `UGameplayStatics::ApplyDamage`.
  2. Trace straight down from the impact point to find the ground, using `ECC_Visibility` and
     ignoring pawns.
  3. Spawn `FireClass` at the ground point.
  4. Destroy itself.
- Safety: `InitialLifeSpan` 10 s, so a molotov thrown off the map cleans itself up.

**`AMolotovFire` (new actor):**
- No collision component. A looping timer every `FireTickInterval` runs an
  `OverlapMultiByObjectType` sphere query (`ECC_Pawn`, radius `FireRadius`) at its location.
- For each unique pawn found:
  - skip it if it is the instigator
  - skip it if it is more than `FireHeight` above the fire's ground point
  - otherwise apply `FireTickDamage` with `ApplyDamage`
- No afterburn: damage only happens on ticks while the enemy is inside.
- Stacking: each fire is its own actor with its own timer.
- Ends via `SetLifeSpan(FireDuration)`.
- **Debug circle:** on each tick, `DrawDebugCircle` (orange, flat on the ground, radius
  `FireRadius`) lasting one tick interval, so it shows continuously while burning. Debug drawing is
  compiled out of Shipping builds automatically.
- **BP hook:** `OnFireStarted(float Radius, float Duration)`, a `BlueprintImplementableEvent`
  fired in `BeginPlay`. Spawn the Niagara fire there in a BP subclass. Use BP **Event End Play** to
  stop it, or just give the Niagara system a matching lifetime.

**The ability `UHeroAbility_ThrowMolotov`:**
- Instant: `bEndsImmediately = true`.
- Refuses to activate if there's no world or hero. It needs no weapon.

### Logging

`LogExpProDev`:
- `ThrowMolotov: thrown`, with speed and direction
- `Molotov: hit <actor>`
- `MolotovFire: started at <loc>`
- `MolotovFire: tick hit N pawns`, at Verbose
- `MolotovFire: burned out`

## 3. Gunpowder Burp (Ultimate, Q)

**Status:** APPROVED 2026-11-25.

### Behaviour (all answered by Jake)

1. The player presses Q with a full ultimate meter. The whole meter is spent.
2. Molotov Guy breathes a **sustained stream** of fire forward for `BurpDuration`.
3. The stream **follows the camera**, so turning or looking around sweeps it.
4. Every `BurpTickInterval`, each enemy inside the cone takes `BurpTickDamage`.
5. The fire **passes through walls**: anything inside the cone is hit, cover or not.
6. Molotov Guy is **slowed** to `MoveSpeedMultiplier` of his speed while burping.
7. It **cannot be ended early**. It always runs the full duration, unless he dies.
8. It leaves **no ground fire**, and the **camera is unchanged**.

### Tuning (`UPROPERTY`, `EditDefaultsOnly`, category `Hero Ability`)

| Property | Meaning | Default | Source |
|---|---|---|---|
| `BurpDuration` | Seconds the stream lasts. | 5 | Jake |
| `BurpTickDamage` | Damage per tick, before `DamageMultiplier`. | 20 | Jake |
| `BurpTickInterval` | Seconds between ticks. | 0.2 (= 100 DPS) | Jake |
| `ConeLength` | Reach, cm. | 1500 | Jake |
| `ConeAngle` | Full cone angle, degrees. | 60 | Jake |
| `MoveSpeedMultiplier` | Walk speed while burping. | 0.5 | Jake |

### How it works

**The ability `UHeroAbility_GunpowderBurp`:**
- Long-running: `bChargeGated = true`, `bEndsImmediately = false`.
- On activate, it starts two timers:
  - a looping damage tick every `BurpTickInterval`, with the first tick straight away
  - an end timer at `BurpDuration`, which calls `FinishAbility`

**Each tick:**
- **Origin** = actor location + `(0, 0, BaseEyeHeight)`, roughly the mouth.
- **Direction** = the control rotation's forward vector, which is the camera, including
  looking up and down.
- Find pawns with `OverlapMultiByObjectType` (`ECC_Pawn`, a sphere of radius `ConeLength`
  at the origin).
- Keep each unique pawn, except the hero, that is:
  - within `ConeLength` of the origin, and
  - within `ConeAngle / 2` of the direction, measured to the pawn's actor location
- Damage each one with `ApplyDamage(GetScaledDamage(BurpTickDamage))`. There is no line-of-sight
  check, because the fire passes through walls.
- **Debug cone:** `DrawDebugCone` (orange) for one tick interval, so it shows continuously while
  burping.

**Slow:**
- On activate, save the movement component's `MaxWalkSpeed`.
- Every tick, set `MaxWalkSpeed` = saved × `MoveSpeedMultiplier`. Re-applying it every tick stops
  a sprint press mid-burp from overriding the slow for more than one tick.
- On end, restore the saved value.
- **Known limitation:** if sprint is pressed or released during the burp, the speed restored at
  the end is whatever it was when the burp *started*. It corrects itself on the next sprint
  press or release.

**`EndAbility` override** (runs on normal end and on death/cancel):
- clears both timers
- restores `MaxWalkSpeed`

**BP hooks:** `OnAbilityActivated` (start the Niagara fire stream; attach it to the head or mesh so
it follows) and **On End Ability** (stop it).

### Logging

`LogExpProDev`:
- `GunpowderBurp: started`
- `GunpowderBurp: tick hit N pawns`, at Verbose
- `GunpowderBurp: ended`

## 4. Files

| File | Change |
|---|---|
| `Abilities/MolotovAbilities.h/.cpp` | **New.** `UHeroAbility_ThrowMolotov`, `AMolotovProjectile`, `AMolotovFire`, `UHeroAbility_GunpowderBurp`. One file pair, matching `BombshellAbilities`. |

No `Build.cs` change. `ProjectileMovementComponent`, `SphereComponent`, `DrawDebugHelpers` and
`UGameplayStatics` are all in the `Engine` module.

**Why not reuse `AProjectile`:**
- Its collision box ignores the `Pawn` channel, so it can't hit enemy capsules directly.
- Its `OnHit` carries tech debt: TD-DEAD-4 and TD-STUB-2.
- It's tied to the weapon damage path.

A standalone actor is smaller and avoids building on known defects.

## 5. Editor work after the build

1. Create `GA_Molotov_Throw` as a BP child of `HeroAbility_ThrowMolotov`, and tune it in Class
   Defaults.
2. Optional: make `BP_MolotovFire`, a child of `MolotovFire`. Add Niagara in **Event On Fire
   Started**, then set `FireClass` on the GA to it.
3. Create the Molotov Guy `UHeroDefinition`:
   - `PowerAbility` = `GA_Molotov_Throw`
   - `AddonAbility` empty
   - `UltimateAbility` = `GA_Molotov_Burp`, a BP child of `HeroAbility_GunpowderBurp`
   - `StartingWeapon` = your choice
4. Add it to the hero registry.

## 6. PIE test checklist

1. Press E. A small sphere arcs toward the crosshair.
2. Throw at the floor. An orange debug circle (5 m radius) appears and lasts 6 s.
3. Stand an enemy in the fire. It takes 5 damage every 0.5 s, and stops the moment it walks out.
4. Hit an enemy directly. It takes 50 impact damage, and the fire spawns on the ground beneath it.
5. Throw at a wall. The fire appears on the floor below the impact point.
6. Stand in your own fire. You take no damage.
7. Overlap two fires on one enemy. It takes double ticks.
8. Press E again within 5 s. Nothing happens.
9. Fill the meter and press Q. An orange debug cone (15 m, 60°) follows the camera for 5 s, and
   the meter empties.
10. Enemies in the cone take 20 damage every 0.2 s, including behind walls.
11. Movement is at half speed during the burp, and back to normal after it.
12. Die mid-burp. On respawn, speed is normal and there is no lingering cone.
