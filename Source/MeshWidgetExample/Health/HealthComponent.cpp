// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "Math/UnrealMathUtility.h"
#include "MeshWidgetExample/Minimap/Public/MinimapManager.h"
#include "MeshWidgetExample/Minimap/Public/MinimapComponent.h"
#include "Kismet/GameplayStatics.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	// Randomize per-NPC health
	Health = FMath::RandRange(MaxHealth * 0.2f, MaxHealth);
	OnHealthChanged.Broadcast(Health);
	TrySelfRegisterWithMinimap();
}

void UHealthComponent::TrySelfRegisterWithMinimap()
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(World, AMinimapManager::StaticClass(), Found);
	if (Found.Num() == 0) return;

	// If this actor doesn’t have a minimap comp, we can’t be tracked by the minimap manager’s enemy set.
	// You can decide to REQUIRE a UMinimapComponent, or relax it:
	if (AActor* Owner = GetOwner())
	{
		if (!Owner->FindComponentByClass<UMinimapComponent>())
		{
			// Optional: auto-add one at runtime, or just bail and let your design enforce it.
			return;
		}
	}

	// Nothing else needed; once the minimap comp registers, the manager will discover our HealthComponent and fire OnEnemyAdded.
}