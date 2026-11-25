# Hero — Bombshell Abilities (Spec)

**Status:** APPROVED 2026-11-25 (Main option (a), proposed defaults, full damage). C++ written,
not yet compiled or PIE-tested.
**Date:** 2026-11-25
**Builds on:** `doc/HeroSystem.md` (phases 0–3). This spec changes the phase 3 ability framework
and adds Bombshell's first real abilities.

---

## 1. Ability slot definitions

These replace the earlier Signature / Addon / Ultimate model.

| Slot | Key | Meaning |
|---|---|---|
| **Main** | Mouse 1 | Primary attacking method. |
| **Power** | E | Primary unique ability for that hero. |
| **Addon** | F | Secondary ability that supports the hero. |
| **Ultimate** | Q | Power-up ability. Deals the most impact. |

**Main stays on the existing weapon path.** Mouse 1 keeps firing through `UCombatComponent`.
A hero may optionally define a Main ability, which would override weapon fire. No current hero
needs one, so see §6 for how much of Main is built now.

## 2. Bombshell kit

| Slot | Ability | Notes |
|---|---|---|
| Main (M1) | Dual pistols | Existing weapon fire. No Main ability. |
| Power (E) | **Spin Shoot** | §4 |
| Addon (F) | *none for now* | Slot left empty. |
| Ultimate (Q) | **Bomb Kick** | §5 |

Upgrades and ranks are **out of scope** for all abilities. They will be designed later. All
tuning values are plain `UPROPERTY` floats for now.

## 3. Framework changes (Phase A)

### A1. Rename Signature to Power

- `EHeroAbilitySlot::Signature` becomes `Power`.
- The native tags `Ability.Slot.Signature` and `Cooldown.Signature` become `Ability.Slot.Power`
  and `Cooldown.Power`.
- `UHeroDefinition::SignatureAbility` becomes `PowerAbility`.
- `AHeroCharacter::SignatureAbilityAction` becomes `PowerAbilityAction`.
- `UAbilityBarWidget::SignatureSlot` (a `BindWidget`) becomes `PowerSlot`.
- The demo `UHeroAbility_Dash` keeps working in the Power slot.

**Asset impact (agreed: re-assign by hand, no CoreRedirect):**
- Each `DA_Hero_*` asset loses its `SignatureAbility` value. Re-pick it as `PowerAbility`.
- The hero pawn Blueprint loses `SignatureAbilityAction`. Re-pick it as `PowerAbilityAction`.
- `WBP_AbilityBar`, if authored, must rename its child widget `SignatureSlot` to `PowerSlot`.
  It fails to compile until that is done.

### A2. Slot comes from the grant, not the ability

**Problem:** each ability sets its own `Slot` in its constructor. The cooldown tag comes from that
value. If a definition puts an ability in a different slot, the HUD watches the wrong cooldown
tag. Two abilities that share a `Slot` value would also share one cooldown.

**Change:**
- Remove the `Slot` property from `UHeroAbility`.
- `AHeroCharacter::GrantAbilities` tells the ability's instance which slot it was granted into,
  straight after `GiveAbility`.
- `GetCooldownTags` and `ApplyCooldown` use that granted slot.

**Why this works (verified in UE 5.6 source):**
- `UAbilitySystemComponent::OnGiveAbility` creates the instance at grant time for
  `InstancedPerActor` + `ReplicateNo` abilities. Ours are both.
- `InternalTryActivateAbility` calls `CanActivateAbility` on that primary instance, not the CDO.
  So the granted slot is visible to the cooldown check.

### A3. Non-instant abilities

**Problem:** `UHeroAbility::ActivateAbility` runs commit, execute and `EndAbility` in one frame.
Spin Shoot needs to stay active for several seconds.

**Change:**
- Add `bool bEndsImmediately = true` to `UHeroAbility`. Existing abilities are unaffected.
- When it is `false`, the ability stays active after `ExecuteHeroAbility`. The subclass ends it
  itself through a new `FinishAbility(bool bWasCancelled)` helper.
- For the end-of-ability BP hook, use the engine's own **On End Ability** event
  (`K2_OnEndAbility`). It fires on normal end and on cancel, so a separate `OnAbilityEnded` was
  not added. BP uses it to stop looping VFX or sound.
- Cancellation (for example death) goes through `EndAbility`, so a subclass's cleanup must live
  in its `EndAbility` override, not only in its own finish path.

### A4. Damage helper

**Fact:** enemies have no ability system component. Only `APlayerCharacter` has one. A
GameplayEffect cannot be applied to an enemy directly.

**Change:**
- Add `UHeroAbility::GetScaledDamage(float BaseDamage)`. It multiplies by the caster's
  `DamageMultiplier` attribute.
- Ability damage is delivered with `UGameplayStatics::ApplyDamage` / `ApplyRadialDamage`.
- Any target that *is* a player already turns that into `GE_Damage` through
  `APlayerCharacter::RecieveDamage`. So health changes still only happen through GAS.
- The demo `UHeroAbility_ChargeBurst` switches to the helper.

Giving enemies their own ASC and attribute set is **out of scope**.

### A5. Equip the starting weapon

**Problem:** `UHeroDefinition::StartingWeapon` exists, but nothing reads it. Heroes spawn
unarmed. Spin Shoot needs a weapon.

**Change:** in `AHeroCharacter::PossessedBy`, spawn `StartingWeapon` and equip it through
`UCombatComponent::EquipWeapon`, if the hero has nothing equipped.

**Supporting accessors:**
- `UCombatComponent::GetEquippedWeapon()`, public and const.
- `APlayerCharacter::GetCombat()`, public and const. `Combat` is currently private.

## 4. Spin Shoot (Power, E)

**Class:** `UHeroAbility_SpinShoot : UHeroAbility`, with `bEndsImmediately = false`.

**Behaviour:**
1. The player presses E. The ability activates only if a weapon is equipped. Without one it does
   not activate and does not start the cooldown.
2. The body spins in a circle at a constant speed, `SpinCount` full turns.
3. The **camera stays fixed** on where the player was looking.
4. The player **can keep walking** during the spin.
5. The equipped weapon fires `ShotsPerSecond` times per second, each shot in the direction the
   body faces at that moment. Shots form a flat ring.
6. After `SpinCount` turns the ability ends. The body snaps back to the camera's facing.
7. The 15 s cooldown starts on activation.

**Tuning (`UPROPERTY`, `EditDefaultsOnly`, category `Hero Ability`):**

| Property | Meaning | Default |
|---|---|---|
| `CooldownSeconds` | Inherited. Delay before reuse. | 15 |
| `SpinCount` (X) | Full 360° turns. | 2 |
| `SpinSpeed` (Y) | Degrees per second, constant. | 720 *(proposed)* |
| `ShotsPerSecond` (Z) | Fire rate during the spin. | 10 *(proposed)* |
| `ShotRange` | Distance to each shot's target point. | 3000 *(proposed)* |

At the proposed defaults the spin lasts 1 s and fires 10 shots.

**How it works:**
- **Only the mesh rotates** (its relative yaw). The actor and camera are untouched. *(Changed
  after PIE: rotating the actor made the camera shake, because the spring arm is attached to the
  mesh and the mesh swings around the capsule centre.)*
- The spring arm is a child of the mesh. For the spin's duration it is re-parented to the
  capsule (keeping its world position), so the spinning mesh cannot move the camera. It goes
  back to its original parent, socket and relative transform when the ability ends.
- **Camera zoom:** calls the shared `AHeroCharacter::ZoomCamera` (§5b) with `ZoomMultiplier`
  (default 2×), the spin's duration, and `ZoomBlendTime` (default 0.25 s). A cancelled spin calls
  `StopCameraZoom`.
- A per-frame timer (`SetTimerForNextTick`, re-armed each step) drives the spin. Each step
  measures the real elapsed time, advances the mesh's relative yaw by `SpinSpeed × dt`, and fires
  every `1 / ShotsPerSecond` seconds.
- Each shot calls `AWeapon::Fire(HitTarget)`, where `HitTarget` = weapon location + the direction
  of (actor yaw + spin so far) × `ShotRange`. That reuses the weapon's sound, trace, `ApplyDamage` and `DamageMultiplier`
  scaling. A projectile weapon also works with no change.
- `EndAbility` clears the timer and restores the mesh's saved relative rotation. Death or
  cancellation runs the same cleanup.

**BP hooks:** `OnAbilityActivated` (spin VFX/sound start), **On End Ability** (stop them).

**Dual pistols:** the ability fires whatever weapon is equipped. A real dual-pistol weapon (two
meshes, alternating muzzles) is separate weapon work. It needs no change to this ability.

## 5. Bomb Kick (Ultimate, Q)

**Class:** `UHeroAbility_BombKick : UHeroAbility`, with `bChargeGated = true`. It is instant.

**Behaviour:**
1. The player presses Q with a full ultimate meter. The whole meter is spent.
2. A bomb appears at the hero's feet and explodes straight away.
3. Every enemy in a large radius takes damage. The hero takes none.
4. The hero is launched straight upwards. Horizontal momentum is kept.
5. There is no fall damage in the project, so the landing is harmless.

**Tuning (`UPROPERTY`, `EditDefaultsOnly`, category `Hero Ability`):**

| Property | Meaning | Default |
|---|---|---|
| `BlastDamage` | Base damage before `DamageMultiplier`. | 250 *(proposed)* |
| `BlastRadius` | Explosion radius, cm. | 800 *(proposed)* |
| `LaunchStrength` | Upward launch velocity. | 1400 *(proposed)* |

**How it works:**
- `UGameplayStatics::ApplyRadialDamage` at the hero's location, using `GetScaledDamage(BlastDamage)`,
  ignoring the hero, full damage across the radius.
- `LaunchCharacter(FVector(0, 0, LaunchStrength), false, true)`. That keeps horizontal velocity
  and overrides vertical.
- The bomb is **visual only**. `OnAbilityActivated` fires in BP, which spawns the bomb mesh,
  explosion VFX and sound at the feet. There is no C++ bomb actor, because it explodes in the same
  frame it appears.

**Decided:** full damage across the whole radius, no falloff.

**Camera zoom, height-driven (approved 2026-11-25; replaces a fixed 5× / 2 s zoom):**
- Bomb Kick is now long-running (`bEndsImmediately = false`). It stays active from the blast
  until the hero lands.
- Each frame it traces down from the feet (`ECC_Visibility`). The target zoom is
  `Lerp(1, MaxZoomMultiplier, Clamp(Height / MaxZoomHeight))`, smoothed with
  `FInterpTo(ZoomInterpSpeed)`, and applied through `AHeroCharacter::SetCameraZoom`.
- The ability ends once `IsMovingOnGround()` is true after a 0.2 s lift-off grace, or after
  `MaxAirTime`. On ending, `StopCameraZoom(0.3)` eases the camera back. Cancel or death does the
  same.
- Defaults: `MaxZoomMultiplier` 5, `MaxZoomHeight` 1000 cm (≈ the peak of a 1400 launch at default
  gravity), `ZoomInterpSpeed` 8, `MaxAirTime` 5 s.

**Explosion VFX (decided 2026-11-25):** Blueprint only, in `GA_Bombshell_BombKick`. Start from
**Event On Ability Activated**, *not* Event ActivateAbility. `UHeroAbility::ActivateAbility`
overrides the native function without calling Super, so the engine's BP event never fires.
Chain: Get Avatar Actor from Actor Info, then Cast To HeroCharacter, then Spawn System at
Location at the actor location minus the capsule half-height.

## 5b. Shared camera zoom (approved 2026-11-25)

On `AHeroCharacter`, `BlueprintCallable`, so C++ and BP abilities can both use it:

- `ZoomCamera(float Multiplier, float Duration, float BlendTime = 0.25)`. The spring arm eases out
  to `Multiplier ×` its normal length over `BlendTime`, holds, then eases back in over the last
  `BlendTime` of `Duration`. `BlendTime` is clamped to half of `Duration`. The ease is
  `FMath::InterpEaseInOut` with exponent 2.
- Calling it again mid-zoom restarts from the current length, so there is no jump.
- The normal length is captured once, on the first zoom, so repeated zooms cannot drift.
- `StopCameraZoom(float BlendTime = 0.25)` eases straight back from wherever the zoom is.
- `SetCameraZoom(float Multiplier)` sets the length immediately and cancels any timed zoom. It is
  for zooms that follow a live value every frame, like Bomb Kick's height. Finish with
  `StopCameraZoom`.
- A per-frame timer (`SetTimerForNextTick`, re-armed each step) drives it.
- The spring arm is found with `FindComponentByClass<USpringArmComponent>()`, so
  `APlayerCharacter` is unchanged.

## 6. Decisions (resolved 2026-11-25)

Resolved: Main = option (a), proposed defaults accepted, full damage.

1. **Main slot scope.** Options:
   - (a) Build nothing for Main now. Add it when the first hero needs a Main ability, for example
     Molotov Guy. *(Recommended: no current hero uses it.)*
   - (b) Add the `Main` enum value, `UHeroDefinition::MainAbility` and a grant handle now, with no
     input binding.
2. **Proposed default values** in §4 and §5. Change any of them.
3. **Bomb Kick damage falloff.** See §5.

## 7. Files touched

| File | Change |
|---|---|
| `Core/ExpProGameplayTags.h/.cpp` | Signature tags renamed to Power. |
| `Abilities/HeroAbility.h/.cpp` | Enum rename, `Slot` removed, granted slot, `bEndsImmediately`, `FinishAbility`, `GetScaledDamage`. |
| `Abilities/HeroDemoAbilities.h/.cpp` | Remove `Slot =` lines. ChargeBurst uses `GetScaledDamage`. |
| `Abilities/BombshellAbilities.h/.cpp` | **New.** `UHeroAbility_SpinShoot`, `UHeroAbility_BombKick`. |
| `Hero/HeroDefinition.h` | `SignatureAbility` renamed to `PowerAbility`. |
| `Hero/HeroCharacter.h/.cpp` | Input rename, granted-slot hand-off, starting weapon equip. |
| `HUD/AbilityBarWidget.h/.cpp` | `SignatureSlot` renamed to `PowerSlot`. |
| `HUD/AbilitySlotWidget.h` | Default `AbilitySlot` changed to `Power`. |
| `Components/CombatComponent.h` | `GetEquippedWeapon()` accessor. |
| `Character/PlayerCharacter.h` | `GetCombat()` accessor. |

No `Build.cs` change. `GameplayAbilities`, `GameplayTags` and `GameplayTasks` are already linked.

## 8. Editor work after the build

1. Re-assign `PowerAbility` on each `DA_Hero_*` asset.
2. Re-assign `PowerAbilityAction` on the hero pawn Blueprint. Bind it to **E** in the input
   mapping context.
3. If `WBP_AbilityBar` exists, rename its `SignatureSlot` child to `PowerSlot`, and set that
   child's **Ability Slot** property to `Power`. The old enum value no longer exists.
4. Make a Bombshell hero definition:
   - `PowerAbility` = `UHeroAbility_SpinShoot`
   - `UltimateAbility` = `UHeroAbility_BombKick`
   - `AddonAbility` empty
   - `StartingWeapon` = a pistol weapon BP
5. Optional: BP subclasses of the two abilities, for `OnAbilityActivated` / `OnAbilityEnded`
   presentation.

## 9. PIE test checklist

1. Spawn as Bombshell. A weapon is equipped without picking one up.
2. Press E. The body spins 2 turns, the camera stays still, shots fire in a ring, and you can walk.
3. Press E again straight away. Nothing happens. The Power HUD slot shows the 15 s cooldown.
4. Spin into enemies. They take damage, and kills feed the ultimate meter.
5. Fill the meter, press Q. Enemies in the radius take damage, you launch upwards, the meter
   empties, and you take no damage.
6. Press Q with the meter not full. Nothing happens.
7. Die mid-spin. On respawn, the body faces the camera and rotation behaves normally.
