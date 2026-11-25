# Weapons Rework — One Weapon Per Hero (Spec)

**Status:** DRAFT — awaiting Jake's approval. No code is written against this spec yet.
**Date:** 2026-11-25
**Builds on:** `doc/HeroSystem.md` (phases 0–3), `doc/hero/HERO_BOMBSHELL_ABILITIES.md`.

---

## 1. Goal

Every hero spawns holding their own signature weapon. Weapons are no longer found, bought or
picked up. The weapon is part of the hero, like their abilities.

| Hero | Weapon | Fire | Ammo model |
|---|---|---|---|
| **Bombshell** | Dual Pistols | Semi-auto hitscan, alternating left/right | Magazine, reload |
| **Molotov Guy** | Molotov | Thrown arcing projectile, fire pool on impact | 1 bottle, "light new bottle" reload |
| **Musket Man** | Three Muskets | Single heavy hitscan shot per musket | 3 rounds, each musket reloads on its own |
| **Hero #4** | *TBD* | — | — |

Weapon identities come from the designers' drafts in `doc/HeroSystem.md` §11.

## 2. What already exists (no work needed)

- `UHeroDefinition::StartingWeapon` (`TSubclassOf<AWeapon>`).
- `AHeroCharacter::EquipStartingWeapon()` spawns it and equips it in `PossessedBy`. A respawn makes
  a new pawn, so the hero is re-armed automatically.
- `AWeapon` (hitscan) and `AWeaponProjectile` + `AProjectile` (projectile) already cover both fire
  types. Damage already scales with the hero's `DamageMultiplier` through `GetOutgoingDamage()`.
- Spin Shoot calls `AWeapon::Fire()` directly, so it works with any hero weapon.

So the hero → weapon link is done. This rework is: **remove the old acquisition paths, add a fire
cadence + ammo model, and build the three weapons.**

## 3. Phase W1 — Scrap weapon acquisition

### W1a. Weapon shop and extraction-zone spawns

Weapons are tied to heroes, so buying them no longer makes sense. Weapon upgrades belong in each
hero's `UpgradeTree` (phase 5 of the hero system).

**Delete:**
- `Weapon/WeaponDefinition.h`, `Weapon/WeaponRegistry.h/.cpp`
- `HUD/WeaponShopWidget.h/.cpp`, `HUD/WeaponSlotWidget.h/.cpp`
- `UEconomySubsystem::TryPurchaseWeapon` and the owned-weapon-count getter
- `AExtractionZone::SpawnPurchasedWeapons`, `WeaponRegistry`, and any trace field used only by it
- `AWeapon::ConsumeWeaponId` and the consume block in `UCombatComponent::EquipWeapon`
- `UExpProSaveGame::PurchasedWeapons`

**Save compatibility:** save games use tagged property serialization, so an old save that still
holds `PurchasedWeapons` loads and silently drops the field. To verify in PIE with an existing slot.

**Retires:** `TD-BUG-11` (extraction zones duplicate purchased weapons).

### W1b. Pickup and equip-from-world

**Delete:**
- `AWeapon`: `AreaSphere`, `PickupWidget`, `OnSphereOverlap`, `OnSphereEndOverlap`,
  `ShowPickupWidget`, `Dropped()`, the `EWS_Dropped` state and its physics setup.
- `APlayerCharacter`: `OverlappingWeapon`, `OnRep_OverlappingWeapon`, `SetOverlappingWeapon`,
  `EquipAction`, `EquipButton`, `ServerEquipButtonPressed`.
- `TD-BUG-22`: weapons never drop, so forced CCD on the weapon mesh goes too.

**Death:** `APlayerCharacter::Eliminated` currently calls `EquippedWeapon->Dropped()`, which leaves
a weapon in the world that anyone could pick up. Change it to destroy the weapon. The respawned
pawn gets a fresh one from `EquipStartingWeapon`.

**Retires:** `TD-ARCH-2` (two parallel interaction systems) — only `IInteractable` remains. Also
removes the `OverlappingWeapon` part of `TD-ARCH-8`. Update the comment in `Interaction/Interactable.h`.

### W1c. Inventory weapon plumbing (dead code)

`FInventorySlot::WeaponInstance`, the `WeaponInstance` parameter of `UInventoryComponent::AddItem`,
and `UItemDefinition::WeaponClass` are never set by any caller. Delete them.

`EItemType::Weapon` stays unless no `DA_Item_*` asset uses it. Removing an enum value that an asset
references corrupts that asset's value.

### W1d. Editor work

- Delete level-placed weapon actors (`BP_AssaultRifle` etc.) from every map.
- Delete `WBP_WeaponShop`, `WBP_WeaponSlot`, `DA_WeaponRegistry`, `DA_Weapon_*`.
- Remove the weapon shop from the main-menu flow.
- Remove the `Equip` action from the input mapping context.

## 4. Phase W2 — Fire cadence and ammo

All three weapons need a fire-rate cap and ammo. None of them is fully automatic, so **automatic
fire is not built** (add it when a hero needs it).

### Model

New fields on `AWeapon`, category `"Weapon"`, `EditAnywhere`:

| Property | Meaning |
|---|---|
| `float FireInterval` | Minimum seconds between shots. Stops click-spamming. |
| `int32 MagazineSize` | Rounds when full. `0` = infinite ammo. |
| `EWeaponReloadMode ReloadMode` | `Magazine` or `PerRound`. |
| `float ReloadTime` | `Magazine`: time to refill the whole magazine. `PerRound`: time to restore one round. |

Runtime state (`BlueprintReadOnly`): `int32 CurrentAmmo`, `bool bIsReloading`.

**Magazine mode** (Pistols, Molotov): firing the last round starts the reload automatically. The
reload refills to `MagazineSize` after `ReloadTime`. Manual reload key: see decision 2.

**PerRound mode** (Muskets): whenever `CurrentAmmo < MagazineSize`, a timer restores one round
every `ReloadTime`. Each fired musket reloads on its own, and you can fire a loaded one while
another reloads.

### Firing path

- New `bool AWeapon::TryFire(const FVector& HitTarget)`. It checks `FireInterval`, ammo and reload
  state, spends one round, then calls `Fire()`. It returns `false` if the shot was refused.
- `UCombatComponent` calls `TryFire` instead of `Fire`.
- **`Fire()` stays the raw "one shot" call and spends no ammo.** Abilities keep calling it directly,
  so Spin Shoot does not drain the pistols. Triple Musket (Musket Man's ultimate) can call it three
  times and then refill.

### Presentation hooks

- `FOnWeaponAmmoChanged(int32 Current, int32 Max)`, `BlueprintAssignable`. The HUD binds to it.
- `BlueprintImplementableEvent`s: `OnFired`, `OnReloadStarted`, `OnReloadFinished`. BP drives
  sounds, muzzle flash, and the musket-on-back visuals.

### HUD

An ammo readout on the player overlay, bound to `FOnWeaponAmmoChanged`. Hidden when
`MagazineSize == 0`. Plain text (`7 / 12`) for now; Musket Man can later show three icons in BP.

**Partly retires** `TD-ARCH-17` (no fire rate, ammo or reload). The automatic-fire part stays open.

## 5. Phase W3 — The three weapons

All numbers are **proposed defaults**. They are `UPROPERTY` values on the weapon BP, so tuning
needs no code.

### 5.1 Bombshell — Dual Pistols

**Class:** `AWeaponDualPistols : AWeapon` (new, small).

**Behaviour:**
- Semi-auto: one shot per click, capped by `FireInterval`.
- Shots alternate between the right and left pistol. The trace starts at the active pistol's
  `Muzzle` socket. Sound and muzzle flash play at that pistol.
- One shared magazine.

**How it works:**
- Adds `OffhandMesh` (`USkeletalMeshComponent`). On equip it is attached to the character mesh's
  `LeftHandSocket`. The main mesh still goes to `RightHandSocket` as today.
- `AWeapon` gains a virtual `GetMuzzleTransform()`. The base returns the main mesh's `Muzzle`
  socket. `AWeaponDualPistols` returns the active pistol's socket and flips after each shot.
- `ApplyImpact` traces from `GetMuzzleTransform()` instead of `GetActorLocation()`. This moves the
  trace origin from the actor's pivot to the muzzle for every weapon.

**Asset needs:** a `LeftHandSocket` on the hero skeleton. Pistol mesh exists at
`Content/Weapons/Pistol/`.

| Property | Default |
|---|---|
| `BaseDamage` | 15 |
| `FireInterval` | 0.15 s |
| `MagazineSize` | 16 (8 per pistol) |
| `ReloadMode` / `ReloadTime` | Magazine / 2 s |

The 2 s reload is from the draft ("Shooting Speed — 2s reload").

### 5.2 Molotov Guy — Molotov

**Classes:**
- Weapon: a BP of the existing `AWeaponProjectile`. No new weapon class.
- `AProjectileMolotov : AProjectile` (new). Gravity on, no bounce. On impact it spawns a fire area
  and destroys itself.
- `AFireArea : AActor` (new). A sphere that lives `Duration` seconds. Every `TickInterval` it
  applies damage through `ApplyDamage` to each overlapping actor except the instigator. It uses a
  timer, not actor tick.

**Throw arc:** `AWeaponProjectile::Fire` aims straight at the crosshair point. With gravity the
bottle would land short. Add `float LaunchPitchOffset` (degrees, default 0) to `AWeaponProjectile`
so the Molotov BP can throw upward. Other projectile weapons are unaffected.

**Visuals:** the bottle mesh is hidden while reloading ("lighting a new bottle"), in BP via
`OnReloadStarted` / `OnReloadFinished`.

| Property | Default |
|---|---|
| Impact damage (`BaseDamage`) | 20 |
| Fire damage per tick | 8 every 0.5 s |
| Fire radius / duration | 250 cm / 4 s |
| `LaunchPitchOffset` | 10° |
| `MagazineSize` | 1 |
| `ReloadMode` / `ReloadTime` | Magazine / 5 s |

The 5 s reload is from the draft ("5s to light new bottle").

**Fire damage and `DamageMultiplier`:** the projectile passes its scaled damage to the fire area at
spawn, so upgrades scale the fire too.

### 5.3 Musket Man — Three Muskets

**Class:** a BP of base `AWeapon`. No new class — the `PerRound` ammo mode is the mechanic.

**Behaviour:**
- One heavy hitscan shot per click.
- 3 rounds = 3 muskets. Each fired musket reloads separately over 10 s.
- The three muskets on his back are BP cosmetics. `OnFired` / `FOnWeaponAmmoChanged` show and hide
  them. C++ treats them as one weapon with 3 rounds.

| Property | Default |
|---|---|
| `BaseDamage` | 90 |
| `FireInterval` | 0.6 s (time to swap muskets) |
| `MagazineSize` | 3 |
| `ReloadMode` / `ReloadTime` | PerRound / 10 s |

The 10 s per musket is from the draft ("10s reload per musket").

### 5.4 Hero #4

No design exists yet. Hero_04 keeps the current placeholder weapon until it does. The W2 model
should cover a new weapon as pure data unless it needs a new mechanic.

## 6. Out of scope

- Weapon upgrades (they go in each hero's `UpgradeTree`, hero-system phase 5).
- Automatic fire, recoil, spread, aim-down-sights changes.
- Hero abilities that use these weapons (Triple Musket, Broken Bottle Jab, etc.). W2 makes them
  possible but they are separate specs.
- Replication. Singleplayer first. New fields are not `Replicated`.
- `TD-BUG-4` beyond the muzzle-origin change in §5.1 (passing the full `FHitResult` through).

## 7. Decisions needing approval

1. **Scrap the weapon shop entirely** (W1a)? *Recommended: yes.* The alternative is keeping it for
   some other purchasable, but nothing needs it today.
2. **Manual reload key (R)?** *Recommended: yes* — one `ReloadAction` on `APlayerCharacter`, used
   by Magazine mode only. Without it, players can only reload by emptying the magazine.
3. **Ammo carried across death?** *Recommended: no.* A respawn gets a full new weapon.
4. **Hero #4's weapon** — leave it TBD, or give a direction now?
5. **Default values** in §5. Change any of them.
6. **Molotov friendly fire on self** — the spec makes the fire ignore its thrower. Confirm.

## 8. Order of work

Each phase is compiled and PIE-checked before the next starts.

1. **W1** scrap (shop, pickup, dead inventory plumbing). Game still plays with the current weapon.
2. **W2** fire cadence + ammo + HUD readout. Test on the current weapon first.
3. **W3** Musket Man (pure data, proves W2) → Dual Pistols → Molotov.

## 9. Files touched

| File | Change |
|---|---|
| `Weapon/Weapon.h/.cpp` | Remove pickup/drop. Add cadence, ammo, reload, `TryFire`, `GetMuzzleTransform`, events. |
| `Weapon/WeaponProjectile.h/.cpp` | `LaunchPitchOffset`. |
| `Weapon/WeaponDualPistols.h/.cpp` | **New.** |
| `Weapon/ProjectileMolotov.h/.cpp` | **New.** |
| `Weapon/FireArea.h/.cpp` | **New.** |
| `Weapon/WeaponDefinition.h`, `Weapon/WeaponRegistry.h/.cpp` | **Deleted.** |
| `HUD/WeaponShopWidget.*`, `HUD/WeaponSlotWidget.*` | **Deleted.** |
| `Components/CombatComponent.h/.cpp` | Call `TryFire`. Remove consume block. Attach offhand. |
| `Character/PlayerCharacter.h/.cpp` | Remove pickup/equip path. Destroy weapon on death. Reload input (if approved). |
| `Economy/EconomySubsystem.h/.cpp` | Remove weapon purchase. |
| `Extraction/ExtractionZone.h/.cpp` | Remove purchased-weapon spawning. |
| `Save/ExpProSaveGame.h` | Remove `PurchasedWeapons`. |
| `Inventory/InventoryComponent.h/.cpp`, `Inventory/ItemDefinition.h` | Remove weapon plumbing. |
| `Interaction/Interactable.h` | Update comment. |
| HUD overlay (`HUD/PlayerOverlay`) | Ammo text. |
| `doc/TECH_DEBT.md` + code markers | Remove TD-BUG-11, TD-ARCH-2, TD-BUG-22. Narrow TD-ARCH-17 and TD-ARCH-8. |

No `Build.cs` change.

## 10. PIE test checklist

1. Each hero spawns holding their own weapon. No weapons lie around the map or the extraction zone.
2. The Equip key does nothing. The main menu has no weapon shop. An old save slot still loads.
3. Bombshell: clicks alternate left/right pistol, the counter drops 16→0, auto-reload takes 2 s.
   Spin Shoot does not change the counter.
4. Molotov Guy: the bottle arcs, a fire pool damages enemies for 4 s, and the next bottle is ready
   after 5 s.
5. Musket Man: 3 shots then empty. Muskets come back one at a time, 10 s apart.
6. Die and respawn. A fresh, full weapon is equipped and nothing is left on the ground.
7. Damage upgrades raise the damage of all three weapons, including Molotov fire.
