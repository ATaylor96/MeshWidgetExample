// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MinimapComponent.generated.h"

class AMinimapManager;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MESHWIDGETEXAMPLE_API UMinimapComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UMinimapComponent();

	/** Picked by designers; used to choose icon texture in UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	int32 IconStyleIndex = 0;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	int32 GetIconStyleIndex() const { return IconStyleIndex; }

protected:
	UPROPERTY()
	TWeakObjectPtr<AMinimapManager> Manager;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	AMinimapManager* FindManager() const;
};