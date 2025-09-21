// Fill out your copyright notice in the Description page of Project Settings.


#include "MinimapComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MinimapManager.h"

UMinimapComponent::UMinimapComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMinimapComponent::BeginPlay()
{
	Super::BeginPlay();
	if (AMinimapManager* Mgr = FindManager())
	{
		Manager = Mgr;
		Mgr->RegisterMinimap(this);
	}
}

void UMinimapComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AMinimapManager* Mgr = Manager.Get())
	{
		Mgr->UnregisterMinimap(this);
	}
	Super::EndPlay(EndPlayReason);
}

AMinimapManager* UMinimapComponent::FindManager() const
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMinimapManager::StaticClass(), Found);
	return Found.Num() > 0 ? Cast<AMinimapManager>(Found[0]) : nullptr;
}