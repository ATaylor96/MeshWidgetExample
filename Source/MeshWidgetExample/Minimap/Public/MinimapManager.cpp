// Fill out your copyright notice in the Description page of Project Settings.


#include "MinimapManager.h"
#include "MinimapComponent.h"
#include "MeshWidgetExample/Health/HealthComponent.h"
#include "Engine/World.h"

AMinimapManager::AMinimapManager()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AMinimapManager::BeginPlay()
{
	Super::BeginPlay();
}

void AMinimapManager::RegisterMinimap(UMinimapComponent* Comp)
{
	if (!Comp) return;
	Registered.Add(Comp);
	OnMinimapListChanged.Broadcast();

	TryAddEnemyForComponent(Comp);
}

void AMinimapManager::UnregisterMinimap(UMinimapComponent* Comp)
{
	if (!Comp) return;
	TryRemoveEnemyForComponent(Comp);
	Registered.Remove(Comp);
	OnMinimapListChanged.Broadcast();
}

void AMinimapManager::TryAddEnemyForComponent(UMinimapComponent* Comp)
{
	if (!Comp) return;
	AActor* OwnerActor = Comp->GetOwner();
	if (!IsValid(OwnerActor)) return;

	if (UHealthComponent* HC = OwnerActor->FindComponentByClass<UHealthComponent>())
	{
		if (!RegisteredEnemies.Contains(HC))
		{
			RegisteredEnemies.Add(HC);
			OnEnemyAdded.Broadcast(HC);
		}
	}
}

void AMinimapManager::TryRemoveEnemyForComponent(UMinimapComponent* Comp)
{
	if (!Comp) return;
	AActor* OwnerActor = Comp->GetOwner();
	if (!IsValid(OwnerActor)) return;

	if (UHealthComponent* HC = OwnerActor->FindComponentByClass<UHealthComponent>())
	{
		if (RegisteredEnemies.Remove(HC) > 0)
		{
			OnEnemyRemoved.Broadcast(HC);
		}
	}
}

void AMinimapManager::GetEntries(TArray<FMinimapEntry>& OutEntries) const
{
	OutEntries.Reset();
	for (auto It = Registered.CreateConstIterator(); It; ++It)
	{
		if (const UMinimapComponent* Comp = It->Get())
		{
			if (AActor* CompOwner = Comp->GetOwner())
			{
				if (!IsValid(CompOwner)) continue;
				OutEntries.Emplace(CompOwner, Comp->GetIconStyleIndex(), CompOwner->GetActorLocation());
			}
		}
	}
}

void AMinimapManager::GetEnemyHealthComponents(TArray<UHealthComponent*>& Out) const
{
	Out.Reset();
	for (const TWeakObjectPtr<UHealthComponent>& WeakHC : RegisteredEnemies)
	{
		if (UHealthComponent* HC = WeakHC.Get())
		{
			Out.Add(HC);
		}
	}
}

void AMinimapManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CullInvalid();
}

void AMinimapManager::CullInvalid()
{
	// Clean Registered (minimap comps)
	for (auto It = Registered.CreateIterator(); It; ++It)
	{
		const bool bDead = !It->IsValid() || !IsValid((*It).Get()->GetOwner());
		if (bDead)
		{
			// Mirror remove from enemies too
			if (UMinimapComponent* Comp = It->Get())
			{
				TryRemoveEnemyForComponent(Comp);
			}
			It.RemoveCurrent();
			OnMinimapListChanged.Broadcast();
		}
	}

	// Clean RegisteredEnemies (health comps)
	for (auto It = RegisteredEnemies.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
			// We can’t broadcast here because we don’t know which Comp caused it,
			// but listeners should handle a missing HC gracefully.
		}
	}
}