# Hero System — build log & design record

Living document for the four-hero feature. It records **why** each decision was made, what the
current state is, and what the next step is. Update the *Progress log* at the bottom as each
phase lands — future-you and the rest of Team Expedition read this before touching
`AHeroCharacter`.

Companion docs: [`../CLAUDE.md`](../CLAUDE.md) (conventions) ·
[`../TECH_DEBT.md`](../TECH_DEBT.md) (defects) · [`../ROADMAP.md`](../ROADMAP.md) (other features)

---

## 1. Goal

Four playable heroes, chosen on a dedicated select screen before a run (Overwatch-style), each
with their own abilities, their own level/XP, and their own upgrade tree.

**Heroes are deliberately generic for now.** Designers have not crafted roles yet, so Phase 2
authors four identically-statted placeholders (`Hero_01`–`Hero_04`) that differ only in name,
colour and mesh. The whole architecture exists so that turning one of them into a real character
is *data authoring*, not a code change.

Three draft heroes (Bombshell, Molotov Guy, Musket Man) have been sketched as **examples of the
shape**, not as content to build. They are recorded in [§11 Appendix](#11-appendix--worked-example)
and every schema decision below is validated against them — if the data model can express all
three without new C++, it is right.

---

## 2. Locked decisions

| Decision | Choice | Why |
|---|---|---|
| Hero architecture | **Hybrid** — one `UHeroDefinition` data asset per hero driving a shared `AHeroCharacter`; a thin C++ subclass only where a hero genuinely can't be data-driven | `APlayerCharacter` is already a god object (TD-ARCH-1). Four full subclasses would mean writing every future feature four times. |
| Ability tech | **GAS** (`GameplayAbilities` plugin) | Chosen deliberately over a lightweight custom component. Industry-standard, and its GameplayEffect model also cleans up the upgrade system. Cost: significant setup, see §5. |
| Currency | **Shared** DOS$ across all heroes | No four-times grind for the same wallet. |
| Account progression | **Shared** account Level + XP | Gates hero and content unlocks. |
| Hero progression | **Per-hero** Level + XP, **per-hero** upgrade trees | Gives each hero build identity and a reason to replay. |
| Hero unlocking | **Both** — an account-Level gate *and* a DOS$ purchase | Level proves you've played; DOS$ makes it a real economy sink. |
| Selection flow | Main Menu → Save Slot → Level Picker *(exists)* → **Hero Select** *(new)* → Start | Hero swappable every run, so per-hero builds stay meaningful. |
| Hero content | Placeholder `Hero_01`–`Hero_04`, stock mannequins | No art dependency; designers replace the data later. |

---

## 3. Hard blocker — RESOLVED in phase 0

The project ran on stock `UGameInstance`, so there was nowhere to store the selected hero across
the menu → island map travel, and hero selection could not work at all. That was tracked as
`TECH_DEBT.md` → TD-CFG-5.

`UExpProGameInstance` (`Source/ExpProDev/Core/`) now fills that role and is registered as
`GameInstanceClass` in `Config/DefaultEngine.ini`. **TD-CFG-5 has been deleted from the register.**
`ROADMAP.md` E2 (difficulty tiers) was blocked on the same thing and is now unblocked — it should
store the chosen difficulty alongside `SelectedHeroId` and `SelectedLevel`.

---

## 4. Architecture

```
APlayerCharacter                    (existing god object — left in place)
└── AHeroCharacter                  + UHeroDefinition*   (all hero data)
                                    + UAbilitySystemComponent
                                    + UHeroAttributeSet
     ├── AHeroCharacter_<Name>      escape hatch — only if a hero needs bespoke C++
     └── ...                        (unused for now: all four heroes are pure data)
```

Everything a hero *is* lives in its `UHeroDefinition`. A C++ subclass is the exception, not the
rule — if you are about to add one, first check whether an ability or an attribute can express
it instead.

### `UHeroDefinition : UDataAsset`

| Field | Purpose |
|---|---|
| `FName HeroId` | Save key. **Never rename after a save exists.** |
| `FText DisplayName`, `Description` | Hero select screen |
| `FText RoleLabel` | "Tank" / "Damage" — display only, no enum needed |
| `UTexture2D* Icon`, `Portrait` | Hero select screen |
| `TSubclassOf<AHeroCharacter> HeroClass` | Defaults to `AHeroCharacter` |
| `USkeletalMesh* Mesh`, `TSubclassOf<UAnimInstance> AnimClass` | Visuals |
| `UMaterialInterface* OverrideMaterial` | How the four placeholders are told apart |
| `float MeshScale`, `CapsuleHalfHeight`, `CapsuleRadius` | Build and height — "short and heavy" vs "tall and heavy" |
| `TSubclassOf<UGameplayEffect> DefaultAttributes` | Base Health / Stamina / MoveSpeed / AttackPower / MaxUltimateCharge |
| `TSubclassOf<UHeroAbility> SignatureAbility` | Slot 1 — the hero's weapon action |
| `TSubclassOf<UHeroAbility> AddonAbility` | Slot 2 — secondary attack |
| `TSubclassOf<UHeroAbility> UltimateAbility` | Slot 3 — charge-gated |
| `TMap<UInputAction*, FGameplayTag> AbilityInputs` | Data-driven input → ability binding |
| `UUpgradeRegistry* UpgradeTree` | This hero's own upgrade shop contents |
| `TSubclassOf<AWeapon> StartingWeapon` | Loadout |
| `int32 RequiredAccountLevel` | Unlock gate — hidden/locked below this |
| `int32 UnlockCost` | DOS$ price once the level gate is met |

`UHeroRegistry : UDataAsset` holds `TArray<UHeroDefinition*>` and a `FindById(FName)` — mirroring
`UUpgradeRegistry` and `UWeaponRegistry` exactly.

### The four placeholders

Stock mannequin content already in `Content/Characters/Mannequins/` gives four distinguishable
heroes with no new art:

| Hero | Mesh | Material | Level gate | DOS$ |
|---|---|---|---|---|
| `Hero_01` | `SKM_Manny_Simple` | `MI_Manny_01_New` | 1 | 0 (starter) |
| `Hero_02` | `SKM_Manny_Simple` | `MI_Manny_02_New` | 3 | 500 |
| `Hero_03` | `SKM_Quinn_Simple` | `MI_Quinn_01` | 6 | 1500 |
| `Hero_04` | `SKM_Quinn_Simple` | `MI_Quinn_02` | 10 | 3000 |

Gates and prices are **placeholders for designers to tune** — they live in the data asset, so
changing them needs no code. All four share one identical `GE_HeroDefaultAttributes` until real
stat blocks exist.

---

## 5. GAS integration

- **Plugin:** enable `GameplayAbilities` in `ExpProDev.uproject`.
- **Build.cs:** add `GameplayAbilities`, `GameplayTags`, `GameplayTasks`.
- **`InitGlobalData()`:** `UAbilitySystemGlobals::Get().InitGlobalData()` must be called once at
  startup, guarded by `IsAbilitySystemGlobalsInitialized()`. Call it from
  `UExpProGameInstance::Init()`. **Skipping this does not fail at startup** — it crashes later,
  only once an ability uses target data. Easy to lose days to.
- **ASC ownership:** on the **pawn** (`AHeroCharacter`), implementing `IAbilitySystemInterface`.
  Correct for singleplayer-first. *If multiplayer ever returns, the ASC must move to
  `ADefaultPlayerState` so it survives respawn* — record that here when it happens.
- **Tags:** declare natively in `ExpProGameplayTags.h/.cpp`. Never string literals.

### `UHeroAttributeSet`

`Health`, `MaxHealth`, `Stamina`, `MaxStamina`, `MoveSpeed`, `AttackPower`,
`UltimateCharge`, `MaxUltimateCharge`.
Use the engine's `GAMEPLAYATTRIBUTE_*` accessor macros from `AttributeSet.h`.

### The Health migration — the single riskiest change

`APlayerCharacter` currently owns `float Health`, `float MaxHealth` and `float DamageMultiplier`
as plain floats, with `RecieveDamage` bound to `OnTakeAnyDamage` and `UpdateHUDHealth()` pushing
to the overlay. GAS wants those values in the attribute set.

**Do not keep both.** Delete the floats outright and re-point `GetHealth()`, `GetMaxHealth()` and
`GetDamageMultiplier()` at the attribute set. Keeping the old floats "just for the HUD" creates
two sources of truth and is the classic half-GAS trap.

Because the getters keep their signatures, `UPlayerOverlay`, `UEnemyHealthBarWidget` and
`UCombatComponent` compile unchanged.

**Damage bridge:** enemies keep calling `UGameplayStatics::ApplyDamage`. `RecieveDamage` applies a
`GE_Damage` with a SetByCaller magnitude instead of decrementing a float — so no enemy code
changes in this phase.

### The three-slot ability template

Every drafted hero decomposes into the same three slots. This is the single most important thing
the examples revealed — it means the ability framework is fixed, and heroes are data.

| Slot | Gated by | Upgradeable | Example |
|---|---|---|---|
| **Signature** | Cooldown = reload time | Base upgrade, 3 ranks | Dual Pistols · Molotov · Three Muskets |
| **Addon** | Cooldown, or unlimited | Damage upgrade, 3 ranks | Spin Shoot · Broken Bottle Jab · Small Dagger |
| **Ultimate** | **Charge, built from kills** | Not upgradeable | Bomb Kick · Gunpowder Burp · Triple Musket |

Two consequences for the framework:

- **Ultimates are not cooldown abilities.** They are gated on `UltimateCharge` reaching
  `MaxUltimateCharge`, and charge is granted on enemy kills. `UHeroAbility` therefore needs a
  `bChargeGated` flavour whose cost is the full charge bar, plus a `GE_GrantUltimateCharge`
  applied from the existing kill/XP path. The HUD shows a fill meter, not a countdown.
- **"3 upgrades" means ability rank, not a new ability.** Use GAS's native ability levels
  (`FGameplayAbilitySpec::Level`) and read damage/cooldown from a curve keyed on level, rather
  than authoring three separate abilities.

Until designers finalise kits, Phase 3 fills these slots with generic demonstrators (a dash, a
self-heal burst, a charge-gated damage burst) shared by all four placeholders. They exist to
prove the pipeline end-to-end — granting, input binding, cooldown, charge, HUD.

---

## 6. Progression & save schema

Shared: `DOSCoins`, account `Level` / `XP`, `PurchasedWeapons`.
Per-hero: unlocked flag, hero level, hero XP, purchased upgrades, unlocked abilities.

```cpp
USTRUCT()
struct FHeroSaveData
{
    GENERATED_BODY()
    UPROPERTY() bool  bUnlocked  = false;
    UPROPERTY() int32 HeroLevel  = 1;
    UPROPERTY() float HeroXP     = 0.f;
    UPROPERTY() TMap<FName, int32> PurchasedUpgrades;
    UPROPERTY() TArray<FName>      UnlockedAbilities;
};
```

Added to `UExpProSaveGame`: `TMap<FName, FHeroSaveData> Heroes;` and `FName LastSelectedHeroId;`.

> **Keep the existing `Level` and `XP` field names** as the account-level fields. They are already
> account-wide, and renaming a `UPROPERTY` silently drops the saved value for every existing save
> file. Bump `CurrentSaveVersion` 1 → 2 and add a migration case in
> `USaveGameSubsystem::MigrateIfNeeded` that moves the old flat `PurchasedUpgrades` onto
> `Hero_01` and marks it unlocked.

### Hero unlocking

Two conditions, both required: account `Level >= RequiredAccountLevel`, then a DOS$ purchase of
`UnlockCost`. `Hero_01` is free at level 1 so a new save always has someone to play.

The purchase goes through the existing economy owner as
`UEconomySubsystem::TryPurchaseHero(FName HeroId, const UHeroRegistry* Registry)`, mirroring
`TryPurchaseUpgrade` and `TryPurchaseWeapon` exactly. It already broadcasts `OnBalanceChanged`
and `OnPurchaseCompleted(FName Id)`, which the hero select screen binds to — no polling.

### Upgrades become GameplayEffects (retires TD-ARCH-7)

`UUpgradeDefinition` drops the hardcoded `EUpgradeType` enum for two data-driven flavours:

1. **Attribute upgrade** — `FGameplayAttribute ModifiedAttribute` + `EGameplayModOp Operation` +
   `ValuePerLevel`. `UUpgradeManagerComponent` builds one dynamic GE per purchase on load.
   Covers Health Boost, Damage Boost, move speed, and anything else added later.
2. **Ability-rank upgrade** — `FGameplayTag AbilityTag` + rank count. Raises that granted
   ability's `FGameplayAbilitySpec::Level`, and the ability reads its damage/cooldown from a
   curve keyed on level. This is what "Shooting Speed, 3 upgrades" and "Broken Bottle Jab,
   3 damage upgrades" actually are.

This kills the "three C++ edits per new upgrade" problem TD-ARCH-7 describes, makes new upgrades
pure data authoring, and works per-hero for free. **Delete the TD-ARCH-7 marker and its
`TECH_DEBT.md` entry when this lands.**

Each hero's `UpgradeTree` is then just two upgrade lines of three ranks each — the shape every
drafted hero already follows.

---

## 7. Phases

Each phase compiles and is PIE-testable on its own. Do not start the next until the current one is
verified in-editor.

### Phase 0 — Foundations ✅ *written, not yet compiled*

Delivered:

| File | Change |
|---|---|
| `ExpProDev.uproject` | `GameplayAbilities` plugin enabled |
| `Source/ExpProDev/ExpProDev.Build.cs` | `GameplayAbilities`, `GameplayTags`, `GameplayTasks` added |
| `Source/ExpProDev/Core/ExpProGameInstance.h/.cpp` | **New.** Holds `SelectedHeroId` / `SelectedLevel`; calls `InitGlobalData()` in `Init()`; static `Get()` resolver |
| `Source/ExpProDev/Core/ExpProGameplayTags.h/.cpp` | **New.** Native tags for the three ability slots, SetByCaller magnitudes, and `State.Dead` |
| `Config/DefaultEngine.ini` | `GameInstanceClass=/Script/ExpProDev.ExpProGameInstance`; TD-CFG-5 marker removed |
| `TECH_DEBT.md` | TD-CFG-5 entry deleted |
| `ROADMAP.md` | E2's dependency on TD-CFG-5 updated — now satisfied |

**Closes TD-CFG-5. Unblocks ROADMAP E2.** No gameplay behaviour changes.

*Before compiling:* regenerate Visual Studio project files, because `.uproject` and `Build.cs`
both changed.

*PIE check:* game launches and plays exactly as before; `Saved/Logs/ExpProDev.log` contains
`ExpProGameInstance: init complete (...)`.

> The init log is deliberately **outside** the `IsAbilitySystemGlobalsInitialized()` guard. The
> editor frequently builds the ability system globals during its own startup, so a log placed
> inside the branch stays silent even when everything is wired correctly — making a missing line
> impossible to distinguish from a `GameInstanceClass` that never resolved. The line must print on
> every PIE start regardless of which side of the branch ran.

### Phase 1 — GAS under the existing character ✅ *written, not yet compiled*

| File | Change |
|---|---|
| `Abilities/HeroAttributeSet.h/.cpp` | **New.** Health, MaxHealth, Stamina, MaxStamina, DamageMultiplier, MoveSpeed, UltimateCharge, MaxUltimateCharge, plus the `Damage` meta attribute. Clamping in `PreAttributeChange` and `PostGameplayEffectExecute` |
| `Abilities/DamageEffect.h/.cpp` | **New.** Instant GE with one SetByCaller modifier on `Damage`, defined in C++ rather than as an asset |
| `Character/PlayerCharacter.h/.cpp` | ASC + attribute set subobjects; implements `IAbilitySystemInterface`; `InitAbilityActorInfo` in `PossessedBy`; health floats deleted; damage bridged through the GE |

Design notes worth keeping:

- **`Damage` is a meta attribute.** Incoming damage lands on it and `PostGameplayEffectExecute`
  drains it into Health. Armour, shields and damage types can later be added in that one place.
- **The attribute is named `DamageMultiplier`, not `AttackPower`** as §5 originally said. It is a
  multiplier (1.0 = unmodified) and `AWeapon::GetOutgoingDamage` uses it as one — renaming the
  concept would have silently changed behaviour.
- **Getters keep their old signatures**, so `ADefaultPlayerController` and `AWeapon` compile
  untouched. They are no longer `FORCEINLINE` since they now dereference the attribute set.
- **`MaxHealth` as a designer field became `BaseMaxHealth`.** If `BP_PlayerCharacter` overrode the
  old `MaxHealth`, that value must be re-entered on `BaseMaxHealth`.
- **Init order is load-bearing:** `ADefaultPlayerController::OnPossess` calls `Super::OnPossess`
  (→ `PossessedBy` → `InitialiseAttributes`, base stats) *before* `LoadPlayerData` (upgrades on
  top). Reversing those two would silently discard every purchased upgrade.
- The `Replicated` / `OnRep_Health` scaffolding is gone; GAS owns attribute replication.

**Test checklist — run before starting phase 2**

| # | Do this | Expect |
|---|---|---|
| 1 | Build, then PIE | Compiles; no Blueprint errors left unresolved |
| 2 | Look at the health bar on spawn | Full, showing the same number as before (100 unless `BaseMaxHealth` says otherwise) |
| 3 | Let an enemy hit you | Bar drops by the usual amount — **not** frozen, not instant death |
| 4 | Let an enemy kill you | Death, loot scatters, respawn works as before |
| 5 | Shoot an enemy | Dies in the same number of shots as before |
| 6 | Buy a health upgrade, then re-enter the level | Max health is higher than step 2 |

Step 3 is the one that matters: a frozen bar means the SetByCaller magnitude isn't resolving.

### Phase 2 — Hero data layer + four placeholders ✅ *C++ written, assets still to author*

| File | Change |
|---|---|
| `Hero/HeroDefinition.h` | **New.** Identity, appearance, stats, ability slots, loadout, unlock gates |
| `Hero/HeroRegistry.h/.cpp` | **New.** `FindById` plus `GetDefaultHero` for the no-selection fallback |
| `Hero/HeroCharacter.h/.cpp` | **New.** Applies a definition — appearance on spawn, stats via an `InitialiseAttributes` override |
| `Character/PlayerCharacter.h` | `InitialiseAttributes` made `virtual`; protected `GetAttributes()` added |
| `Gamemode/DefaultGameMode.h/.cpp` | `HeroRegistry` property, `ResolveSelectedHero`, and overrides of both pawn-spawn hooks |

Design notes worth keeping:

- **Everything degrades safely.** No registry set → falls back to `DefaultPawnClass`. No hero
  selected (the normal case when PIE starts on an island rather than the menu) → falls back to the
  registry's first entry. A definition with a null mesh/material or zero scale leaves the pawn's
  own. A half-configured hero therefore plays like the old character rather than breaking.
- **Appearance fields use "unset means keep your own"**, so placeholders can't accidentally
  change the capsule and leave the hero floating or sunk into the floor.
- **Stats are always applied**, with defaults matching the old `APlayerCharacter` values — a
  definition left at defaults plays exactly like the pre-hero player.
- **Spawning is deferred** (`SpawnActorDeferred` → `SetHeroDefinition` → `FinishSpawningActor`) so
  the definition is applied before `BeginPlay` and possession. `AlwaysSpawn` matches the existing
  respawn behaviour.
- Both spawn hooks are `BlueprintNativeEvent`s, so the overrides are on
  `GetDefaultPawnClassForController_Implementation` and `SpawnDefaultPawnAtTransform_Implementation`.
- Ability slots are typed `TSubclassOf<UGameplayAbility>` because `UHeroAbility` does not exist
  until phase 3; it will derive from it, so assets assigned now stay valid.
- `SetSkeletalMeshAsset`, not `SetSkeletalMesh` — the latter is deprecated on the
  `USkinnedMeshComponent` base as of 5.1.

**Assets to author in the editor** (nothing works until these exist):

1. Four `UHeroDefinition` assets, `DA_Hero_01`–`DA_Hero_04`, per the table in §4.
2. One `UHeroRegistry` asset, `DA_HeroRegistry`, listing all four in order.
3. Set `HeroRegistry` on `BP_DefaultGameMode` to `DA_HeroRegistry`.
4. Set each definition's `HeroClass`. If `BP_PlayerCharacter` should stay the base, reparent it to
   `AHeroCharacter` and use it; otherwise point `HeroClass` at a Blueprint deriving from
   `AHeroCharacter`.

**Test checklist — run before starting phase 3**

| # | Do this | Expect |
|---|---|---|
| 1 | Build and PIE *before* authoring any assets | Game plays exactly as before — the null-registry fallback holds |
| 2 | Author the assets, set `HeroRegistry`, PIE | Spawns as `Hero_01`; log shows `HeroCharacter: spawned as 'Hero_01'` |
| 3 | Play normally for a minute | Movement, shooting, damage, inventory all behave as before |
| 4 | Reorder the registry so `Hero_03` is first, PIE | Spawns as Quinn with `MI_Quinn_01` |
| 5 | Die and respawn | Respawns as the same hero, not the default pawn |
| 6 | Check purchased upgrades still apply | Max health still reflects health upgrades |

Step 4 is the real proof the data path works. Step 5 catches the respawn route bypassing the
hero spawn.

### Phase 3 — Abilities

`UHeroAbility : UGameplayAbility` base with BP presentation hooks and a `bChargeGated` flavour.
Input bound generically via `AbilityInputs` (tag-driven, not the legacy InputID enum). The three
slots from §5 — Signature / Addon / Ultimate — filled by generic demonstrators. Ultimate charge
granted from the existing kill path via `GE_GrantUltimateCharge`. `UAbilityBarWidget` +
`UAbilitySlotWidget` follow the `UHotbarWidget` pattern; the ultimate slot draws a fill meter
rather than a cooldown sweep.
**Status: ✅ C++ written, assets still to author.**

| File | Change |
|---|---|
| `Abilities/HeroAbility.h/.cpp` | **New.** Base class: data-driven cooldown, charge gating, `OnAbilityActivated` BP hook, `EHeroAbilitySlot` |
| `Abilities/HeroCooldownEffect.h/.cpp` | **New.** One shared cooldown effect; duration is SetByCaller |
| `Abilities/HeroSupportEffects.h/.cpp` | **New.** `UHealEffect`, `UUltimateChargeEffect`, `UUltimateSpendEffect` |
| `Abilities/HeroDemoAbilities.h/.cpp` | **New.** Placeholder Dash / Heal Burst / Charge Burst |
| `Hero/HeroCharacter.h/.cpp` | Grants the definition's three abilities; binds three input actions; `GetAbilityForSlot` |
| `Character/PlayerCharacter.h/.cpp` | `GrantUltimateCharge` |
| `AI/EnemyCharacter.cpp` | Kills now feed the ultimate meter |
| `HUD/AbilityBarWidget`, `HUD/AbilitySlotWidget` | **New.** Three-slot bar following the `UHotbarWidget` pattern |
| `HUD/PlayerHUD.h/.cpp`, `PlayerController/DefaultPlayerController.cpp` | Bar creation and hand-off on possession |

Design notes worth keeping:

- **One cooldown effect for the whole game.** `UHeroCooldownEffect` takes its duration from
  SetByCaller and its identity from the spec's `DynamicGrantedTags`, so no ability needs its own
  cooldown asset. `CooldownSeconds = 0` means no cooldown at all — that is how "unlimited use"
  abilities like Broken Bottle Jab get authored.
- **Ultimates never touch the cooldown path.** `GetCooldownTags` returns null for them, and
  `CanActivateAbility` gates on a full meter instead; activation spends the whole bar.
- **Input activates by spec handle, not by tag.** Simpler than asset tags, and it sidesteps the
  5.5 deprecation of `UGameplayAbility::AbilityTags` (`GetAssetTags`/`SetAssetTags` if ever needed).
- **The three input actions live on `AHeroCharacter`, not the definition** — every hero uses the
  same three keys, so repeating them in four assets would be pure duplication. This replaces the
  `TMap<UInputAction*, FGameplayTag> AbilityInputs` originally sketched in §4.
- **Only kills charge ultimates.** `GrantUltimateCharge` is called from `AEnemyCharacter`, not
  from `AddXP`, so quest XP deliberately does not fill the meter.
- **The HUD asks the ability system for cooldown time** via `GetActiveEffectsTimeRemaining`
  rather than running its own timer, so the bar cannot drift out of step with the real gate.
- `InstancedPerActor` instancing — `NonInstanced` is deprecated as of 5.5.

**Assets to author in the editor**

1. Three Input Actions (`IA_AbilitySignature`, `IA_AbilityAddon`, `IA_AbilityUltimate`) plus
   mappings in the existing Input Mapping Context — suggested keys Q, E, R.
2. Set those three on the hero pawn Blueprint (`SignatureAbilityAction` etc.).
3. On each `DA_Hero_*`, set the three ability classes to `HeroAbility_Dash`,
   `HeroAbility_HealBurst`, `HeroAbility_ChargeBurst`.
4. `WBP_AbilitySlot` parented to `UAbilitySlotWidget`; bind visuals to `FillFraction`,
   `bIsReady`, `CooldownRemaining`, `AbilityIcon`, and hide when `bHasAbility` is false.
5. `WBP_AbilityBar` parented to `UAbilityBarWidget`, containing three slot widgets named exactly
   `SignatureSlot` / `AddonSlot` / `UltimateSlot`, each with its **`Ability Slot`** property set
   to match. (The property is `AbilitySlot`, not `Slot` — `UWidget` already owns a member called
   `Slot` and UHT rejects shadowing it.)
6. Set `AbilityBarWidgetClass` on `BP_PlayerHUD` to `WBP_AbilityBar`.

**Test checklist — run before starting phase 4**

| # | Do this | Expect |
|---|---|---|
| 1 | Build and PIE before authoring assets | Plays as before; no ability bar, no errors |
| 2 | Author the assets, PIE, press the signature key | Hero dashes forward |
| 3 | Press it again immediately | Nothing happens; that slot shows ~5s of cooldown refilling |
| 4 | Take damage, then press the addon key | Health goes up; that slot starts a 15s cooldown |
| 5 | Press the ultimate key at zero charge | Nothing happens |
| 6 | Kill enemies until the ultimate slot is full, then press it | Radial burst damages nearby enemies; meter empties |

Step 6 proves the whole charge loop. Step 3 proves the shared cooldown effect is tagged per-slot
rather than blocking every ability at once.

### Phase 4 — Hero select screen

`UHeroSelectWidget` + `UHeroSelectEntryWidget` — copy the `USaveSlotSelectWidget` /
`USaveSlotEntryWidget` pattern, it is the exact shape needed. New `WBP_HeroSelect` alongside the
existing `WBP_LevelPicker` in `Content/Maps/MainMenu/MainMenuUI/`.

> **Blueprint change required.** `WBP_LevelPicker` is a pure Blueprint widget with no C++ class,
> so it currently opens the map itself. It must instead store the chosen level on the
> GameInstance and push `WBP_HeroSelect`, which then does the `OpenLevel`.

*PIE check:* pick Smugglers Bay + `Hero_03` → correct map, Quinn mesh with `MI_Quinn_01`.

### Phase 5 — Per-hero progression

Save v2 + migration, hero XP/level split in `AddXP`, per-hero upgrade trees, and the
upgrades → GameplayEffect rework from §6.
*PIE check:* level `Hero_01`, switch to `Hero_02` → it is level 1, DOS$ shared, `Hero_01`'s
upgrades absent; a pre-existing save loads with nothing lost.

### Phase 6 — Hero unlocking

`TryPurchaseHero` on `UEconomySubsystem`, the level gate, and locked/affordable/owned states on
the select screen.
*PIE check:* a hero below its level gate shows locked; at level but unaffordable shows the price
greyed; buying deducts DOS$, persists, and makes it selectable next run.

---

## 8. Risks

| Risk | Mitigation |
|---|---|
| Two sources of truth for Health | Delete the floats in Phase 1; never shadow them |
| `InitGlobalData()` omitted | Crashes late, not at startup — do it in Phase 0 and log it |
| GAS learning curve | Phase 1 is deliberately small and isolated |
| `WBP_LevelPicker` is pure BP and opens the map itself | Phase 4 rewires it to defer to hero select — a Blueprint edit, not a code one |
| Blueprint reparenting breaks asset refs | `BP_PlayerCharacter` → `AHeroCharacter`; audit DefaultPawnClass and HUD casts |
| Heroes inherit the god object | Consider extracting `UPlayerProgressionComponent` during Phase 5, while already touching that code |
| Placeholder heroes ship by accident | They are visibly `Hero_01`–`Hero_04` with stock mannequins — obvious enough that it can't slip through |

---

## 9. Answered questions

| Question | Answer |
|---|---|
| What are the four heroes? | Undecided by design. Generic `Hero_01`–`Hero_04` with identical data, amended once designers craft roles. |
| Hero meshes? | Stock UE mannequins — `SKM_Manny_Simple` / `SKM_Quinn_Simple` × two material instances each. |
| Level select? | Already exists (`WBP_LevelPicker`). Only the hero select screen is new. |
| Unlock model? | Both — an account-Level gate *and* a DOS$ purchase. |

---

## 10. Progress log

| Date | Phase | Status | Notes |
|---|---|---|---|
| 2026-09-09 | — | Planned | Design record written. Decisions locked (§2). Nothing implemented yet. |
| 2026-09-09 | — | Planned | Open questions answered (§9): placeholder heroes, stock mannequins, existing level picker, dual unlock gate. Phase list re-cut to 0–6; art dependency removed. |
| 2026-09-09 | — | Planned | Three draft heroes received (§11). Schema validated against them: added the three-slot ability template, ultimate charge attributes, ability-rank upgrades, and build/height fields. |
| 2026-09-09 | 0 | **Written — awaiting compile** | Foundations done: GAS plugin, module deps, `UExpProGameInstance`, native tag set, `GameInstanceClass` config. TD-CFG-5 retired from the register. Needs project-file regeneration, then a build and a PIE launch to confirm the init log line. |
| 2026-09-09 | 0 | ✅ **Verified in PIE** | `ExpProGameInstance: init complete (ability system globals were already built).` The editor had built GAS globals itself; the guard means our call still covers packaged builds. Note: the init log had to be moved outside the init guard first — a probe inside it cannot distinguish "not running" from "already initialised". |
| 2026-09-09 | 1 | **Written — awaiting compile** | Attribute set, C++ damage effect, ASC on `APlayerCharacter`, health floats deleted. Needs a build; expect Blueprint compile errors anywhere BP referenced the old `Health` / `MaxHealth` / `DamageMultiplier` variables. |
| 2026-09-09 | 1 | ✅ **Verified in PIE** | All six checks pass. Health is fully attribute-backed; damage, death, weapon scaling and health upgrades all behave as before. |
| 2026-09-09 | 2 | **C++ written — assets to author** | `UHeroDefinition`, `UHeroRegistry`, `AHeroCharacter`, game mode spawn overrides. Falls back to the old pawn until `DA_HeroRegistry` exists, so it is safe to build and play before authoring anything. |
| 2026-09-09 | 2 | ✅ **Verified in PIE** | All six checks pass. Hero data path works end to end — reordering the registry swaps mesh and material, and respawn keeps the hero. |
| 2026-09-09 | 3 | **C++ written — assets to author** | Ability base class, shared cooldown effect, three placeholder abilities, kill-fed ultimate charge, three-slot HUD bar. Inert until the input actions and widget Blueprints exist. |
| 2026-09-09 | 3 | **Fix — health HUD was push-only** | Healing changed the attribute but never refreshed the bar, because `UpdateHUDHealth` was only called from `RecieveDamage`. `APlayerCharacter::PossessedBy` now binds `GetGameplayAttributeValueChangeDelegate` for Health and MaxHealth, so *any* source refreshes the HUD; the manual call in `RecieveDamage` was removed as redundant. **Re-test damage as well as healing.** |

---

## 11. Appendix — worked example

**Reference only. Do not build these.** Three heroes drafted to show what a finished character
looks like. Their value here is as a test of the schema: every field below is expressible in
`UHeroDefinition` + `UHeroAttributeSet` + the upgrade tree, with **no new C++ per hero**.

| | Bombshell | Molotov Guy | Musket Man |
|---|---|---|---|
| Role | Tank | Damage | Tank |
| Health | High | Medium | High |
| Move speed | Slow | Average | Average |
| Build | Short, heavy | Tall/medium, medium | Tall, heavy |
| Weapon | Dual Pistols | Molotov | Three Muskets (worn on back) |
| **Signature** | Shooting Speed — 2s reload | Fire Spread Range — 5s to light new bottle | Shot Quantity — 10s reload per musket |
| **Addon** | Spin Shoot — 15s cooldown | Broken Bottle Jab — unlimited | Small Dagger — unlimited |
| **Ultimate** | Bomb Kick | Gunpowder Burp | Triple Musket (all three, reloads on use) |
| Ultimate charge | Enemy kill XP | Enemy kill XP | Enemy kill XP |
| Upgrade lines | Signature ×3, Addon damage ×3 | Signature ×3, Addon damage ×3 | Signature ×3, Addon damage ×3 |

### How each maps to the schema

| Design concept | Where it lives |
|---|---|
| Role label, health tier, move speed | `RoleLabel` + `DefaultAttributes` GE (`MaxHealth`, `MoveSpeed`) |
| Height and build | `MeshScale`, `CapsuleHalfHeight`, `CapsuleRadius` |
| Weapon | `StartingWeapon` (existing `AWeapon` system) |
| Signature / Addon / Ultimate | The three ability slots |
| Reload and cooldown times | Cooldown GE duration on each `UHeroAbility`, curve-scaled by rank |
| "Unlimited use" | Ability with no cooldown GE |
| "3 upgrades" on either line | Ability-rank upgrades in that hero's `UpgradeTree` |
| Ultimate charged by kill XP | `UltimateCharge` / `MaxUltimateCharge` + `GE_GrantUltimateCharge` on kill |
| All the "Price: TBD" values | `BaseCost` / `CostScalePerLevel` on each `UUpgradeDefinition` |

**Nothing here needs a C++ subclass.** The one thing that would is a genuinely novel *mechanic* —
Bombshell's bomb physics or the burp cone — and those are `UHeroAbility` subclasses, which are
shared assets, not per-hero character classes.
