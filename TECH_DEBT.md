# TECH_DEBT.md — ExpPro

The single register of known technical debt in this project. Every entry has a stable ID
(`TD-<CATEGORY>-<n>`) and is mirrored by a `TECH_DEBT(<ID>)` comment at the site in the code.

```
grep -rn "TECH_DEBT" Source/ Config/          # every marker
grep -rn "TD-BUG-1)" Source/                  # one specific item
```

**Scope.** This covers the real game only: `Character/`, `Components/`, `Weapon/`, `AI/`,
`Inventory/`, `Quest/`, `Save/`, `Upgrade/`, `Economy/`, `Extraction/`, `HUD/`, `Gamemode/`,
`PlayerController/`, `Interaction/`, plus build and config. Engine template and starter content
(`Variant_Shooter/`, `Variant_Horror/`, `ExpProDev*` template classes) is not annotated in place —
it appears here only under **Dead code**, where the entry is "delete it".

**Status.** Nothing in this document has been fixed. Adding the markers was a documentation pass —
no code was changed beyond comments and whitespace.

---

## Categories

| Prefix | Meaning |
|---|---|
| `TD-BUG-n` | Defects. Wrong behaviour, crashes, silent data loss. |
| `TD-DEAD-n` | Code that runs nowhere or does nothing. Pure deletion. |
| `TD-ARCH-n` | Design debt. Works today; makes the next feature expensive. |
| `TD-BP-n` | C++ ↔ Blueprint contracts that fail silently at runtime. |
| `TD-STUB-n` | Started and abandoned. Half-wired features. |
| `TD-CFG-n` | Build and configuration. |

Severity: **High** — corrupts data, crashes, or blocks the core loop. **Medium** — wrong or
expensive, with a workaround. **Low** — cleanliness.

---

## Start here

Four items are worth doing before anything else.

| ID | Why first |
|---|---|
| **TD-BUG-1** | Silently destroys player items on pickup. Every hour played loses loot. |
| **TD-BUG-11** | Free duplication of purchased weapons if a second extraction zone is placed. |
| **TD-DEAD-1** | Deleting `WebSockets/` unblocks three other deletions and four module deps. |
| **TD-ARCH-13** | No stable item/quest IDs. Blocks inventory and quest persistence entirely. |

`TD-DEAD-1` and `TD-ARCH-13` are enablers — several other entries are waiting on them.

---

## TD-BUG — Defects

### TD-BUG-1 — `AddItem` silently destroys items (High)
`Inventory/InventoryComponent.cpp` · `UInventoryComponent::AddItem`

When an item stacks onto an existing partial stack, only `min(space, Quantity)` is added. The
remainder is dropped on the floor of the function and `true` is returned anyway. `AItemPickup::Interact`
takes that `true` as "all consumed" and destroys the pickup. Picking up 5 of an item into a stack
with 1 space free loses 4 of them, with nothing logged.

*Fix:* distribute the remainder across further partial stacks, then empty slots. Return false only
if nothing at all could be added, and give partial adds a contract with `AItemPickup` so the pickup
survives holding what didn't fit.

### TD-BUG-2 — `AddItem` only considers the first partial stack (Medium)
Same function. `FindExistingStack` returns the first non-full stack; if it can't take everything,
the code never looks at other partial stacks or at empty slots. Same fix as TD-BUG-1.

### TD-BUG-3 — `RemoveItem` doesn't validate quantity (Low)
`UInventoryComponent::RemoveItem`. A negative `Quantity` inflates the stack; over-removal clears the
slot and still returns `true`. No caller passes bad values today.

### TD-BUG-4 — Hitscan damage re-traces from the wrong origin (Medium)
`Weapon/Weapon.cpp` · `AWeapon::ApplyImpact`

`UCombatComponent::TraceUnderCrosshairs` already resolves the actor under the crosshair, but only
the impact *point* is passed down. `ApplyImpact` then runs a second trace from the weapon actor's
origin to that point. Different origin, different result: shots can damage an actor the player
never aimed at, or be blocked by cover the camera trace saw past. Pass the resolved `FHitResult`
through instead of re-deriving it.

### TD-BUG-5 — Projectiles damage everything they touch (High)
`Weapon/ProjectileBullet.cpp` · `AProjectileBullet::OnHit`

`ApplyDamage` is called on whatever the bullet hit, with no owner, team or class filter. The
shooter, other players and world geometry all receive damage events. Only the *hitmarker sound*
bothers to exclude the owner. Currently masked because the projectile's collision box ignores
`ECC_Pawn`, so the fix is to filter at the damage call, not rely on collision setup.

### TD-BUG-6 — Unguarded `GetPawn()` in the chase timer (Medium)
`AI/EnemyAIController.cpp` · `RefreshChase`. Runs on a repeating timer that isn't cleared when the
pawn is destroyed mid-chase. `TargetActor` is validity-checked on the line above; the pawn is not.

### TD-BUG-7 — Enemies hit through walls, instantly (High)
`AI/EnemyAIController.cpp` · `PerformAttack`

Damage is applied on the same frame the attack montage starts, with no line-of-sight check and no
re-test of range at the moment of impact. The player takes damage before the swing is visible and
while standing behind cover. Drive damage from an anim notify and add an LOS trace.

### TD-BUG-8 — Unguarded `Controller` in movement input (Medium)
`Character/PlayerCharacter.cpp` · `MoveButton`. Input can still be routed for a frame after
unpossession, notably during the elimination → respawn handoff.

### TD-BUG-9 — `GEngine->GameViewport` dereferenced unchecked (Medium)
`HUD/PlayerHUD.cpp` · `DrawHUD`. `GEngine` is null-checked; `GameViewport` isn't. It is null during
level transitions and in headless runs.

### TD-BUG-10 — Hotbar binds its delegate twice (Low)
`HUD/HotbarWidget.cpp` · `InitHotbar`. Binds `OnInventoryChanged` without unbinding first, and is
reached from two paths (`APlayerHUD::AddHotbarWidget` and `InitHotbarForInventory` via `OnPossess`).
The hotbar refreshes twice per inventory change. `UInventoryScreenWidget::InitInventory` already
calls `RemoveAll(this)` first — mirror it.

### TD-BUG-11 — Extraction zones duplicate purchased weapons (High)
`Extraction/ExtractionZone.cpp` · `SpawnPurchasedWeapons`

Runs per-zone at `BeginPlay`, reading the shared purchased-weapon counts without claiming them. Two
extraction zones on one map each spawn the **full** purchased set. The save is only decremented on
pickup, so the duplicates are free. A designer placing a second zone silently creates an exploit.

*Fix:* spawn from a single authority (game mode or subsystem), or have zones claim counts at spawn.

### TD-BUG-12 — Hardcoded player index in the crosshair trace (Low)
`Components/CombatComponent.cpp` · `TraceUnderCrosshairs` uses `GetPlayerController(this, 0)`
instead of the component's owning controller.

### TD-BUG-13 — Manual tick re-registration workaround (Medium)
`Components/CombatComponent.cpp` · `BeginPlay`

Four lines that re-register the component's tick function, commented "Fixed issue where the combat
wasn't ticking." A component created in the owner's constructor with `bCanEverTick` should already
tick. The real cause was never found, so this hides an ordering problem that will resurface.

### TD-BUG-14 — HUD XP push is a timing guess (Medium)
`Character/PlayerCharacter.cpp` · `BeginPlay`. A 0.2s one-shot timer delays the XP push because the
HUD widgets don't exist yet at pawn `BeginPlay`. It is a race, not a fix — a slow frame loses the
update. Same root cause as TD-ARCH-9: the HUD should observe, not be pushed to.

### TD-BUG-15 — Cheat commands ship in every configuration (Medium)
`Character/PlayerCharacter.h`. `WipeSave`, `SetLevel` and `SetDOSCoins` are `UFUNCTION(Exec)` with no
build guard — they are present in Shipping. Wrap in `#if !UE_BUILD_SHIPPING` or move to a
`UCheatManager`.

### TD-BUG-16 — `KillCount` never resets (Low)
`PlayerController/DefaultPlayerController.cpp` · `AddKill`. Never reset on respawn, new run, or save
slot change. It only climbs for the controller's lifetime.

### TD-BUG-17 — Save-slot widget dereferences `BindWidget` pointers unguarded (Medium)
`HUD/SaveSlotSelectWidget.cpp` · `NativeConstruct`. `SlotEntry_0/1/2` are dereferenced with no null
check — a missing or renamed child in the widget BP crashes on construct instead of logging. The
count of three is also hardcoded in two places.

### TD-BUG-18 — Enemies can spawn off the navmesh (High)
`AI/EnemySpawner.cpp` · `TrySpawnOne`

The candidate point is line-traced to geometry but never projected onto the navmesh. Enemies spawned
off-nav can't path, so they stand still permanently — and the spawner still counts them as alive, so
it stops replenishing. The area quietly goes dead. Use `ProjectPointToNavigation`.

### TD-BUG-19 — Enemies destroyed inside their own damage callback (Medium)
`AI/EnemyCharacter.cpp` · `ReceiveDamage`. `Destroy()` is called synchronously from within the
`OnTakeAnyDamage` broadcast. There is no death state, ragdoll, montage or delay — and no window in
which a death animation could ever be added.

### TD-BUG-20 — `DisableInput` on a possibly-null controller (Low)
`Character/PlayerCharacter.cpp` · `MulticastEliminated`. `PlayerController` is only populated by
paths that ran the lazy cast. If none did, input is silently left enabled on a dead pawn.

### TD-BUG-21 — Slot reference aliases the array being mutated (Low)
`Character/PlayerCharacter.cpp` · `DropAllItemsOnDeath`. `Slot` is a reference to the live array
element that `RemoveItem` then clears, and the `SpawnActor` between them can fire overlap callbacks
that also touch the inventory. Safe today only because dead players are filtered out of pickup.
Copy the slot out before spawning.

### TD-BUG-22 — CCD forced on for every weapon, always (Low)
`Weapon/Weapon.cpp` · constructor. Continuous collision detection is enabled permanently to stop
dropped weapons tunnelling through the floor. The original TODO already flagged the cost. Scope it
to the dropped/simulating state, or fix the collision setup so it isn't needed.

---

## TD-DEAD — Dead code

Deletion order matters: **1 → 2/3/4 → 5/6 → 7 → 8 → 9 → 10**, then the `Build.cs` and config
cleanups (TD-CFG-1..4). Build after each group.

All of these are Perforce-tracked; check them out before deleting. Content-side assets must be
deleted in-editor — this list only covers source.

### TD-DEAD-1 — `WebSockets/` prototype (High value, easy)
`WebSocketGameInstance`, `WebSocketClient`, `ARemotePlayer`, `AWSPlayerCharacter`.

Not wired into anything. `UWebSocketGameInstance` is **not** the project's GameInstance — no
`GameInstanceClass` is configured, so the engine uses stock `UGameInstance` and none of this runs.
It is the sole consumer of the `WebSockets`, `Json` and `JsonUtilities` module dependencies, and
`WebSocketGameInstance.cpp` includes `Variant_Shooter/ShooterCharacter.h`, which is what keeps that
dead variant linked. **Delete this first** — it unblocks TD-DEAD-8 and TD-CFG-1.

Also flag `Content/Dev/RoseJ/` (its test maps/assets) for in-editor deletion.

### TD-DEAD-2 — `Private/AI/AICharacter.*`
Empty `ACharacter` subclass from an early AI experiment. Zero references.

### TD-DEAD-3 — `Private/AI/AICharacterController.*`
Included only by `DemoCharacterAI.cpp`, which is itself dead. Delete with TD-DEAD-4.

### TD-DEAD-4 — `AI/DemoCharacterAI.*` + `DemoCharacterAIController.*`
Superseded by `AEnemyCharacter` + `AEnemyAIController`. The only remaining C++ reference is a no-op
cast in `AProjectile::OnHit` (both its branches are empty). **Caution:** `BP_PlayerCharacter` is
reported to reference `DemoCharacterAI` — clear that in-editor first. Its two stale TODOs (hit-react
montage, enemy health bar overlay) both describe work now done properly elsewhere.

### TD-DEAD-5 — `Quest/QuestComponent.*`
Fully superseded by `UQuestManagerComponent`. Implements one hardcoded collect-and-return quest and
is added to nothing in C++. It still binds `OnInventoryChanged` and writes quest HUD text, so if a
Blueprint ever adds it to the pawn it will fight `UQuestManagerComponent` for the same HUD slot.

### TD-DEAD-6 — `Quest/ObjectReturnPoint.*`
`bEnabled` defaults to `false` and turn-in moved to `AQuestGiverNPC`, so every overlap early-outs.
Verify no placed instances in shipped maps, then delete.

### TD-DEAD-7 — `Variant_Horror/`
Unused engine template variant. Not cooked, not referenced by the game. Also flag
`Content/Variant_Horror/` and `Content/Gamemode/Variant_Horror/`.

### TD-DEAD-8 — `Variant_Shooter/`
Unused engine template variant, but still **cooked** via `MapsToCook` (TD-CFG-3). Remove that entry
first, then delete the source. Contains its own parallel weapon/AI/UI stack (`AShooterWeapon`,
`AShooterNPC` on StateTree, `IShooterWeaponHolder`) that is easy to mistake for the real game's.
Its `ShooterCharacter.cpp:135` still traces against `ECC_MAX` — the same bug already fixed in the
real combat code. Also flag `Content/Variant_Shooter/` and `Content/Gamemode/Variant_Shooter/`.

### TD-DEAD-9 — Template base classes
`ExpProDevCharacter`, `ExpProDevGameMode`, `ExpProDevPlayerController`, `ExpProDevCameraManager`.
Only removable once TD-DEAD-7/8 are gone (the variants inherit from `AExpProDevCharacter`).
**Optional** — confirm `BP_FirstPersonCharacter` / `Lvl_FirstPerson` aren't wanted before deleting.
Doing so also clears TD-CFG-4.

### TD-DEAD-10 — `ProcEnvironmentGenerator` + `ProcObjectBase`
Runtime procedural placement prototype in `Private/` + `Public/`. Not part of the island loop, not
referenced by any shipped map. Sole reason for the `ProceduralMeshComponent` dependency, and the
only code still living in the template's `Public/`/`Private/` root layout. Confirm intent first —
this may be someone's active experiment.

### TD-DEAD-11 — `USaveGameSubsystem::SaveToActiveSlot`
No callers, and actively dangerous: it writes straight past `CachedSave`, leaving the in-memory
cache stale so the next flush silently overwrites whatever it just wrote. Remove it, or route it
through the cache like every other writer.

### TD-DEAD-12 — `LogExpProDev` category
Declared as "the main log category used across the project". It isn't — its only three call sites
are in template and variant code. Every real gameplay class logs to `LogTemp`, so the game's output
can't be filtered or given its own verbosity. Either adopt it project-wide or drop it.

### TD-DEAD-13 — `PlayerEliminated` ignores its controller parameters
`Gamemode/DefaultGameMode.cpp`. `VictimController` and `AttackerController` are accepted and never
read. Callers compute and pass them, so the death path looks like it supports kill attribution,
scoring and death messages when it does none of that. Use them or drop them.

### TD-DEAD-14 — Commented-out code and tutorial references
Scattered blocks of dead commented code, plus comments carrying tutorial lecture numbers
("DEATH ANIMATIONS LEC:101", "Dissolve effects LEC:103"). Largest offenders: the abandoned
turn-in-place block in `APlayerCharacter::AimOffset`, camera attachment attempts in the constructor,
and `FORCEINLINE` tutorial notes in `PlayerCharacter.h`. Version control already remembers all of it.

---

## TD-ARCH — Design debt

### TD-ARCH-1 — `APlayerCharacter` is a god object (High)
771 lines. Owns input binding, movement, sprint, health, elimination, XP and levelling, currency,
save/load orchestration, loot selling, interaction dispatch, extraction-zone state, and direct HUD
pushes. Every gameplay feature ends up editing this one file, so every feature branch conflicts here.

*Direction:* extract `UPlayerProgressionComponent` (XP/level/coins) and `UInteractionDriverComponent`
first; move the save orchestration out so the pawn stops including save headers. Keep input,
movement, health and camera on the pawn. Do this **after** the correctness work — they touch the
same lines.

### TD-ARCH-2 — Two parallel interaction systems (High)
Items, quest givers and the extraction zone use `IInteractable` + `PendingInteractable` + the
Interact key. Weapons use an entirely separate path: `AWeapon`'s `AreaSphere` overlap →
`SetOverlappingWeapon` → the replicated `OverlappingWeapon` property → a dedicated Equip key →
`ServerEquipButtonPressed`. Two overlap registrations, two input actions, two prompt mechanisms, for
one player-facing concept. Anything new that can be picked up has to pick a side.

### TD-ARCH-3 — `IInteractable` can't be implemented in Blueprint (Medium)
`Interaction/Interactable.h`. `Interact` is a plain C++ virtual rather than a `BlueprintNativeEvent`,
so no BP-only actor can ever be interactable. There is also no `GetInteractionText()`, which is why
nothing can show a generic "Press E to …" prompt — `AExtractionZone` had to grow its own bespoke
prompt widget instead of reusing a shared one.

### TD-ARCH-4 — Only one quest can be active (High)
`Quest/QuestManagerComponent.h`. A single `FActiveQuestState`. The whole accept/cancel/complete API
assumes it, and `AQuestGiverNPC` has to hard-block every other NPC while a quest is held. Supporting
concurrent quests means reworking the API, the giver logic and the HUD together.

### TD-ARCH-5 — Quest behaviour is a hardcoded enum switch (Medium)
Adding a third quest type (escort, reach-location, timed, protect) requires editing
`UQuestManagerComponent`, `UQuestDefinition` and the HUD. Instanced polymorphic objective objects on
the definition would make new quest types pure data authoring.

### TD-ARCH-6 — Quest text is unlocalisable by construction (Medium)
`SetHUDQuestText` takes `FString`, not `FText`, so callers `.ToString()` the `FText`s that
`UQuestDefinition` stores properly — throwing away the localisation data on the way. `UpdateHUD` also
hardcodes `"Return to Quest Giver"` in English despite `ReturnObjective` existing on the data asset
for exactly that purpose.

### TD-ARCH-7 — Upgrade effects are hardcoded (Medium)
`Upgrade/UpgradeDefinition.h`. A two-value `EUpgradeType` enum. Adding a third effect (move speed,
stamina, loot magnet) needs C++ in three places: the enum, a new bespoke getter on
`UUpgradeManagerComponent`, and a new apply line in `APlayerCharacter::LoadPlayerData`. The two
existing getters are the same loop copy-pasted with a different case.

*Direction:* `TMap<FName /*Attribute*/, float /*ValuePerRank*/>` on the definition, one
`GetAttributeBonus(FName)` on the manager. New upgrades become data-only.

### TD-ARCH-8 — Multiplayer scaffolding in a singleplayer-first game (Medium)
Server/NetMulticast RPCs, `Replicated` properties, `OnRep_` callbacks and `HasAuthority()` branching
run through `APlayerCharacter`, `UCombatComponent`, `AWeapon` and `AProjectile`. Per CLAUDE.md the
project is singleplayer-first, so none of it is exercised or tested — but it still dictates how
every new feature has to be written, and it is the reason the weapon pickup path (TD-ARCH-2) can't
simply be collapsed.

*Decision needed:* keep it as a future-multiplayer investment, or strip it. Leaving it undecided is
the actual cost.

### TD-ARCH-9 — HUD is push-updated through repeated boilerplate (Medium)
Every `SetHUD*` in `ADefaultPlayerController` repeats the same ritual: a lazy
`PlayerHUD == nullptr ? Cast<>` re-resolve, then a hand-rolled chain of null checks reaching three
levels into the widget tree (`PlayerHUD->PlayerOverlay->HealthBar`). The pawn pushes state into the
HUD rather than the HUD observing the pawn, so every new stat means another near-identical function
here plus a matching push call site on the pawn. This is also the root cause of TD-BUG-14.

### TD-ARCH-10 — UI strings hardcoded across six widgets (Low)
`FText::FromString(TEXT("{0} DOS$"))` and friends are inlined in `UShopWidgetBase`,
`USellSummaryWidget`, `UUpgradeShopWidget`, `UWeaponShopWidget` and the controller's
`"Kills: %d"` / `"Level: %d"`. None are localisable and the "DOS$" suffix appears in six places. One
currency-formatting helper plus an `NSLOCTEXT` table.

### TD-ARCH-11 — Preprocessor macros where constants belong (Low)
`#define TRACE_LENGTH 80000.f` in `CombatComponent.h` leaks into every translation unit that
includes it and can't be overridden per weapon. `#define ECC_SkeletalMesh` in `ExpProDev.h` bypasses
`ECollisionChannel`'s type safety and must be kept manually in sync with `DefaultChannelResponses`
in `DefaultEngine.ini` — nothing catches a mismatch.

### TD-ARCH-12 — Declared collision channel never used (Low)
`DefaultEngine.ini` declares a "Projectile" channel (`ECC_GameTraceChannel1`) that no C++
references. `AProjectile` blocks against Visibility, WorldStatic and SkeletalMesh instead of its own
channel, which is why projectile filtering has to be done in code (TD-BUG-5).

### TD-ARCH-13 — Items and quests have no stable identifiers (High)
`UItemDefinition` and `UQuestDefinition` are referenced only by raw pointer. There is no way to
write one to a save file and resolve it back on load. `UUpgradeDefinition` and `UWeaponDefinition`
both already carry an `FName` Id for exactly this reason.

**This is the concrete blocker for TD-ARCH-14.** Do it first.

### TD-ARCH-14 — The save schema is thin (High)
`UExpProSaveGame` persists XP, Level, DOSCoins, purchased upgrades and purchased weapons. Not
persisted: carried inventory, active quest and its progress, completed-quest history.

Consequences: quitting loses everything carried (so extraction is the only thing that makes loot
real — arguably fine as design, but it is accidental, not chosen); and because `CompletedQuests`
lives only on the component, **every completed quest is offered again after a restart and can be
farmed for XP indefinitely.**

*Sequence:* TD-ARCH-13 → add fields → bump `CurrentSaveVersion` → add the migration case in
`MigrateIfNeeded` (the switch is already stubbed and waiting).

### TD-ARCH-15 — Enemy tuning has two sources of truth (Medium)
`AI/EnemyAIController.cpp` re-declares every `AEnemyCharacter` default as a magic-number fallback:
`150.f` range, `10.f` damage, `1.5f` cooldown, `1000.f` roam radius, `3.f` roam wait. They already
disagree with the designer-facing defaults in places, and a designer changing the data asset won't
change the fallback.

### TD-ARCH-16 — Enemy FSM has no attack state (Medium)
Only Roaming and Chasing exist. Attacking is a timer bolted onto the chase refresh, so there is no
wind-up, no strafing, no ranged behaviour, and no way for an archetype to override how it engages.
Target selection is a hardcoded `"Player"` actor-tag string literal that fails silently if the tag
is missing and can't express factions. `SetMaxAge(10.f)` is hardcoded while every neighbouring sight
value is designer-facing. **Blocks enemy variety and any boss encounter.**

### TD-ARCH-17 — No fire rate, ammo or reload model (Medium)
`UCombatComponent::FireButtonPressed`. `bPressed` is stored and replicated but never used to drive
sustained fire. Every weapon is semi-automatic and fires as fast as the player can click — weapons
can't be differentiated by cadence, and there is no ammo economy to hang loot or shop items on.

### TD-ARCH-18 — Ticking that buys nothing (Low)
`APlayerCharacter::Tick` runs every frame solely to call `AimOffset`, which copies one rotation value
the anim instance could read itself (and stores Pitch in a member named `AO_Yaw` — a leftover from
the abandoned turn-in-place work). `AItemPickup::Tick` bobs and spins each pickup individually, so
cost scales with world loot count once enemy drops and death scatter accumulate.

### TD-ARCH-19 — Hardcoded content path in C++ (Medium)
`AI/EnemyCharacter.cpp` constructor: `ConstructorHelpers::FClassFinder` on
`/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed`. Moving or renaming that asset in the editor
fails silently at runtime — every enemy loses its animation, with no cook-time error. Should be an
`EditDefaultsOnly TSubclassOf` set on the enemy BP.

### TD-ARCH-20 — Magic numbers (Low)
Unexplained constants throughout: `±2000uu` ground traces (duplicated between
`AItemPickup::GroundedLocation` and `AEnemySpawner::TrySpawnOne`), the `+100uu` pickup lift, `50uu`
loot spacing along a single world-X line, the `0.2s` HUD timer, camera constants
`160/70/23/130`. The ground-snap logic in particular should exist in exactly one place.

### TD-ARCH-21 — Duplicated eligibility filter in the spawner (Low)
`AEnemySpawner::TrySpawnOne` writes the same eligibility condition twice — once to total the weights,
once to pick — and they must be kept in sync by hand.

### TD-ARCH-22 — No automated tests of any kind (Medium)
There is no test suite; validation is a human running PIE. Every entry in this document was found by
reading, not by a failing test, and TD-BUG-1 is exactly the class of bug one assertion would have
caught. The pure-logic systems (`UInventoryComponent`, `UEconomySubsystem`, `UUpgradeManagerComponent`,
save migration) need no world context and are cheap to cover.

### TD-ARCH-23 — `OnInventoryChanged` is a non-dynamic multicast (Low)
Blueprints can't bind to it, and listeners have no unbind discipline — which is how TD-BUG-10
happens.

---

## TD-BP — Blueprint contract debt

The recurring shape: C++ declares a contract, the Blueprint side has to satisfy it, and nothing
checks. Failures are silent and look like broken gameplay rather than missing setup.

### TD-BP-1 — `BlueprintImplementableEvent`s with no C++ fallback (High)
| Site | If unimplemented |
|---|---|
| `UQuestSelectionWidget::OnQuestsReady` | **This event *is* the quest list.** Talking to a giver opens an empty panel that captures the mouse. |
| `USaveSlotSelectWidget::OnSlotSelected` | Picking a slot marks it active, then nothing. Player stranded on the menu. |
| `APlayerCharacter::OnEnteredExtractionZone` / `OnExitedExtractionZone` | No prompt feedback; no way to tell the overlap fired. |

*Fix pattern:* add a `Native…` C++ default that populates a `BindWidgetOptional` container, so the
flow works before the BP is polished and the BP can still override.

### TD-BP-2 — Required class defaults only fail at runtime (Medium)
`SlotWidgetClass`, `Registry`, `QuestWidgetClass`, `SellSummaryWidgetClass`, the enemy health-bar
widget class and others must be set in BP class defaults. Some now log errors (good); none fail at
cook or validation time. A data-validation pass would catch these before a playtest does.

### TD-BP-3 — No `BLUEPRINT_WIRING.md` (Medium)
The `BindWidget` name contracts — `SlotEntry_0/1/2`, `ListContainer`, `SlotContainer`,
`TotalCoinsText`, `HealthBar`, `XPBar`, `QuestText` and the rest — exist only as UPROPERTY names in
C++ headers. Anyone building or repairing a widget BP has to reverse-engineer them. One checklist
enumerating every required child name, class default and GameMode/HUD/PlayerController assignment
would remove a whole class of "it silently shows nothing" bugs.

### TD-BP-4 — Shop widget BPs still need re-parenting (High — blocks both shops)
`UUpgradeShopWidget` and `UWeaponShopWidget` were reparented onto `UShopWidgetBase` /
`UShopSlotWidget` during the economy refactor. `WBP_UpgradeShop` and `WBP_WeaponShop` still need
re-parenting and re-saving in the editor, and their scroll boxes renaming to `ListContainer`, before
either shop renders anything. Editor work, not code.

---

## TD-STUB — Started and abandoned

### TD-STUB-1 — Crouch (Low)
Input action bound, `Crouch()` called, `bIsCrouched` plumbed through to the anim BP — but no crouch
capsule sizing, speed change or animation set. Pressing crouch does nothing visible. Sites:
`APlayerCharacter::CrouchButtonPressed`, `UPlayerAnimInstance::NativeUpdateAnimation`.

### TD-STUB-2 — Hit reactions (Medium)
Never implemented. Taking damage has no animation or camera feedback anywhere in the game — for the
player, for enemies, or on projectile impact. Four abandoned stubs across
`APlayerCharacter::RecieveDamage`, `OnRep_Health` and `AProjectile::OnHit`. `PlayHitReactMontage()`
doesn't exist. Combat feel is the most visible thing this costs.

### TD-STUB-3 — Death animation and dissolve (Low)
`APlayerCharacter::MulticastEliminated`. The pawn freezes in place for `EliminatedDelay` seconds,
then vanishes. Enemy deaths are worse — see TD-BUG-19, which removes the window a death animation
would need.

### TD-STUB-4 — Inventory screen is display-only (Medium)
`UInventoryScreenWidget` renders the grid but has no slot interactions. `UInventoryComponent::SwapSlots`
exists with **no callers**, and there is no `DropSlot` at all — so items can only leave the inventory
by selling or dying. Opening the inventory is currently a read-only operation.

---

## TD-CFG — Build and configuration

### TD-CFG-1 — Module dependencies with no C++ usage (Medium)
`ExpProDev.Build.cs` declares modules nothing uses, inflating build and link times:

| Module | Status |
|---|---|
| `StateTreeModule`, `GameplayStateTreeModule` | Zero references anywhere in `Source/`. |
| `OnlineSubsystem`, `OnlineSubsystemSteam` | No C++ usage; Steam is plugin/config driven. |
| `Json`, `JsonUtilities` | Used only by the dead `WebSockets/` folder. |
| `WebSockets` | TD-DEAD-1 only. |
| `ProceduralMeshComponent` | TD-DEAD-10 only. |

Remove each as its dependent dead code goes. Keep `NavigationSystem`, `EnhancedInput`, `AIModule`,
`UMG`, `Slate`.

### TD-CFG-2 — `Variant_*` public include paths (Low)
Six entries exist so template code can use flat includes. They also let any new file accidentally
include variant headers without a path — which is how the dead variants keep getting pulled back
into the build. Remove with TD-DEAD-7/8.

### TD-CFG-3 — Template maps in `MapsToCook` (Medium)
`DefaultGame.ini` still cooks `/Game/FirstPerson/Lvl_FirstPerson` and
`/Game/Variant_Shooter/Lvl_Shooter`, pulling their whole content trees into every cook. Cooking
`Lvl_Shooter` is also what keeps `Variant_Shooter/` alive — remove this entry before TD-DEAD-8.

### TD-CFG-4 — `TP_FirstPerson` redirects (Low)
`DefaultEngine.ini` carries `ActiveGameNameRedirects` and four `ActiveClassRedirects` from the
original template module. Only needed while unconverted template assets exist; retire with TD-DEAD-9.

### TD-CFG-5 — No `GameInstanceClass` configured (Medium)
The engine uses stock `UGameInstance`, so the project has nowhere to keep state across a map change —
chosen difficulty, current run context, selected loadout. **Any feature that must survive
menu → island travel needs this first.** (Note: do *not* point it at `UWebSocketGameInstance` — see
TD-DEAD-1.)

---

## Suggested sequence

Ordered by unblocking value, not by size.

**1. Stop the bleeding.** TD-BUG-1, TD-BUG-2 (item loss) · TD-BUG-11 (weapon duplication) ·
TD-BUG-18 (dead spawners) · TD-BUG-7 (hits through walls) · TD-BUG-5 (unfiltered projectile damage).
All are local fixes with no dependencies.

**2. Delete.** TD-DEAD-1 first, then 2/3/4 → 5/6 → 7 → 8 → 9 → 10, then TD-CFG-1..4. Deletion-only,
build after each group. This is the cheapest large reduction in surface area available, and it makes
everything after it easier to reason about.

**3. Unblock persistence.** TD-ARCH-13 (stable IDs) → TD-ARCH-14 (save inventory, quests, completed
quests). Fixes the quest-farming exploit and makes runs feel continuous.

**4. Decide on multiplayer.** TD-ARCH-8 is a fork in the road: strip the replication layer, or commit
to it. TD-ARCH-2 (unify interaction) can't be done cleanly until this is answered.

**5. Make systems extensible.** TD-ARCH-7 (data-driven upgrades) · TD-ARCH-5 / TD-ARCH-4 (quest
types, concurrent quests) · TD-ARCH-16 (enemy FSM). These are what enemy variety, bosses, and a
deeper upgrade tree are all waiting on.

**6. Then refactor.** TD-ARCH-1 (decompose the pawn) and TD-ARCH-9 (delegate-driven HUD) last —
they touch the same code as everything above, so doing them first guarantees rework.

Throughout: TD-ARCH-22 (tests) pairs naturally with step 1 — the inventory and economy fixes are
exactly the code that is cheap to test and expensive to get wrong twice.

---

## Housekeeping

When you fix an item: remove its `TECH_DEBT(...)` comment from the code **and** delete the entry
here in the same change. An entry that outlives its marker (or vice versa) makes this document
untrustworthy, and an untrustworthy debt register gets ignored.

Don't renumber IDs when entries are removed — gaps are fine and keep older commits and code comments
readable.
