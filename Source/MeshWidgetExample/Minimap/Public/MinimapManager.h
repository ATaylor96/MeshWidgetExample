// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeshWidgetExample/Health/HealthComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "MinimapManager.generated.h"

class UMinimapComponent;
class UHealthComponent;


USTRUCT(BlueprintType)
struct FMinimapEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Actor;

	UPROPERTY(BlueprintReadOnly)
	int32 IconStyleIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	FVector WorldLocation = FVector::ZeroVector;

	FMinimapEntry() {}
	FMinimapEntry(AActor* InActor, int32 InIconIdx, const FVector& InLoc)
		: Actor(InActor), IconStyleIndex(InIconIdx), WorldLocation(InLoc) {
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMinimapListChanged);

// New: precise enemy add/remove
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyAdded, UHealthComponent*, HealthComp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyRemoved, UHealthComponent*, HealthComp);

UCLASS(Blueprintable)
class MESHWIDGETEXAMPLE_API AMinimapManager : public AActor
{
	GENERATED_BODY()

public:
	AMinimapManager();

	/** Icons (by index) to choose in the UI. You can fill this array in the editor or via data table. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	TArray<TSoftObjectPtr<UTexture2D>> IconStyles;

	/** Called when list changes (register/unregister/destroy). */
	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FOnMinimapListChanged OnMinimapListChanged;

	// New: enemy events
	UPROPERTY(BlueprintAssignable, Category = "Minimap|Enemies")
	FOnEnemyAdded OnEnemyAdded;

	UPROPERTY(BlueprintAssignable, Category = "Minimap|Enemies")
	FOnEnemyRemoved OnEnemyRemoved;

	/** Returns current entries (valid actors only). */
	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void GetEntries(TArray<FMinimapEntry>& OutEntries) const;

	/** Register/unregister from component */
	void RegisterMinimap(UMinimapComponent* Comp);
	void UnregisterMinimap(UMinimapComponent* Comp);

	// Get current health components of registered enemies
	UFUNCTION(BlueprintCallable, Category = "Minimap|Enemies")
	void GetEnemyHealthComponents(TArray<UHealthComponent*>& Out) const;

protected:
	UPROPERTY()
	TSet<TWeakObjectPtr<UMinimapComponent>> Registered;

	UPROPERTY()
	TSet<TWeakObjectPtr<UHealthComponent>> RegisteredEnemies;

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
public:
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

private:
	void TryAddEnemyForComponent(UMinimapComponent* Comp);
	void TryRemoveEnemyForComponent(UMinimapComponent* Comp);
	void CullInvalid();
};