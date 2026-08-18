# ROADMAP.md — ExpPro gameplay features

Planned gameplay content. These are **new features**, not debt — for known defects and design debt
see [TECH_DEBT.md](TECH_DEBT.md).

Rescued from the old `TASKS_BACKLOG.md` (phase E) when that file was retired. Everything else it
contained was either already implemented or was technical debt, and now lives in `TECH_DEBT.md`.

## Context for any of these

- UE 5.6, single module `ExpProDev`. Read `CLAUDE.md` first.
- The real game is `ADefaultGameMode` + `APlayerCharacter` + `ADefaultPlayerController` +
  `APlayerHUD` + `UCombatComponent` + `AWeapon` + `AEnemyCharacter`/`AEnemyAIController`/`AEnemySpawner`,
  plus the `Inventory/`, `Quest/`, `Save/`, `Upgrade/`, `Economy/`, `Extraction/` and `HUD/` folders.
- Never build on `Variant_Shooter/`, `Variant_Horror/`, `WebSockets/`, `Private/AI/` or
  `DemoCharacterAI` — all dead code awaiting deletion (`TECH_DEBT.md` → TD-DEAD).
- Singleplayer first: no Server RPCs, NetMulticast functions or Replicated properties in new code.
- Perforce: tracked files are read-only until checked out. On an EPERM write failure, stop and ask.
- No test suite. A clean build is the minimum bar; list the in-PIE checks to run afterwards.

---

## E1 — Extraction stakes: lose unsold loot on death

**Depends on:** TD-ARCH-14 (inventory persistence).

Death currently costs nothing — respawn keeps everything, so extraction carries no tension.

- On death, all `EItemType::Loot` slots drop at the death location as `AItemPickup` actors — a
  recoverable corpse run, not deletion. Non-loot items (weapons, consumables) are kept.
  `APlayerCharacter::DropAllItemsOnDeath` already does the fan-out spawn; it needs to become
  type-selective via a new `UInventoryComponent::ExtractSlotsOfType(EItemType)`.
- Insurance: a one-run consumable bought pre-run through `UEconomySubsystem::TryPurchaseInsurance`.
  While active, loot is kept on death; the flag clears on trigger or on a successful extraction,
  whichever comes first. Persist it in `UExpProSaveGame` (version bump).
- HUD: an insurance indicator while active, following existing overlay patterns.
- Save on death as a checkpoint, so quitting after dying can't restore lost loot.

**Acceptance:** die carrying loot → pickups at the death spot, weapons retained, recoverable by
walking back. Die insured → loot kept, insurance consumed. Quit after death → loot still gone.

---

## E2 — Island difficulty tiers

**Depends on:** TD-CFG-5 (a `GameInstanceClass` must exist to hold the choice across map travel).

- `UDifficultyDefinition` (`UDataAsset`) in `Source/ExpProDev/Difficulty/`: DisplayName, Description,
  EnemyCountMultiplier, EnemyHealthMultiplier, EnemyDamageMultiplier, LootValueMultiplier,
  XPMultiplier, RequiredPlayerLevel, UI/tint fields.
- Chosen on the main menu before the run via a `UDifficultySelectWidget` (follow existing `HUD/`
  patterns, with C++ fallback rows). Stored on the GameInstance so it survives travel.
- Application: `AEnemySpawner` multiplies spawn counts and replenish caps at BeginPlay;
  `AEnemyCharacter` scales MaxHealth and attack damage at spawn; XP-on-kill and loot `BaseValue` at
  sell time take their multipliers. Apply the loot multiplier in the sell path so item data assets
  stay untouched.
- Tiers above the save's Level show as locked.
- No new maps — difficulty modifies the existing island.

**Acceptance:** a hard tier spawns more and tougher enemies and pays more per loot and kill; a locked
tier can't be selected below its level; the choice survives menu → island travel.

---

## E3 — Loot rarity

Mostly data plumbing over existing structs.

- `EItemRarity` (Common → Legendary) + a `Rarity` field on `UItemDefinition` (default Common), and
  one project-wide rarity → `FLinearColor` mapping as the single source of truth.
- `FLootEntry` already carries weights. Add a helper that scales weights per-rarity when a multiplier
  is supplied, so E2's difficulty hook can drive it.
- Visuals: `AItemPickup` tints its mesh via a dynamic material instance parameter (document the
  parameter name the user must add in-editor). Inventory and hotbar slots show the rarity colour via
  an optional `UImage* RarityBorder`.
- Sell summary rows show rarity colour. `BaseValue` stays authored per-item — no auto-scaling.
- Enemy drop tables are already per-enemy configurable; verify and report rather than changing.

**Acceptance:** a Rare item shows a tinted pickup and tinted slot; the sell summary reflects colours;
untagged items default to Common and behave exactly as before.

---

## E4 — Enemy variety and the first boss

**Depends on:** TD-ARCH-16 (the FSM needs a real Attacking state first) and TD-ARCH-15.

Combat variety is the largest content gap. Build only on the real AI stack — do not touch
`Variant_Shooter`'s StateTree AI.

- Audit `AEnemyCharacter` and expose what is currently hardcoded as `EditDefaultsOnly` so
  `BP_BasicEnemy` / `FastSmall` / `Tank` can configure it. Add one new behaviour flag: `bRanged` —
  ranged enemies hold at `AttackRange` and fire an `AProjectile` (using the damage-at-spawn API)
  instead of meleeing.
- `AEnemyAIController`: extend the FSM with an Attacking state honouring range and cooldown for both
  melee and ranged. Move the `"Player"` tag to a UPROPERTY while you're in there.
- `ABossEnemy : AEnemyCharacter` — large health pool; two phases (at 50% health: faster, plus a
  telegraphed radial AoE burst); its own high-value `FLootTable` and large XP; a kill notification the
  quest system's EliminateEnemies objective can target (class matching via `IsChildOf` should already
  work — verify).
- Boss health bar: top-of-screen or overhead, following existing `OverheadWidget` / `PlayerOverlay`
  patterns.
- `ABossSpawnPoint`, or a one-time-spawn flag on `AEnemySpawner` (bosses must not replenish).
- Report the BP/asset work the user must do: create the BP child, assign meshes and anims, place the
  spawner.

**Acceptance:** a ranged enemy keeps its distance and shoots; the boss phases at 50%, telegraphs its
AoE, drops boss loot, awards XP, and completes an EliminateEnemies quest targeting its class.
