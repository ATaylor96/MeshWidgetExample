// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Components/Widget.h"

#include "MinimapIconsWidget.generated.h"

class USlateVectorArtData;
class AMinimapManager;

UCLASS()
class MESHWIDGETEXAMPLE_API UMinimapIconsWidget : public UWidget
{
	GENERATED_BODY()

public:
	// A simple quad or vector art for the icon; we’ll instance it.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	USlateVectorArtData* IconMeshAsset = nullptr;

	// SceneCapture2D orthographic width in UU (what your capture uses).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Capture")
	float OrthoWidthUU = 10000.f;

	// If your minimap material zooms in/out, put that factor here (e.g. 2.0 for “2x zoom in”).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Capture")
	float MaterialZoom = 1.0f;

	// When true, clamp icons to the visible circle (half of effective world width).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Clamp")
	bool bAutoClampToVisible = true;

	// Optional manual clamp (UU). Used only when bAutoClampToVisible == false.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Clamp", meta = (EditCondition = "!bAutoClampToVisible"))
	float ClampWorldRadiusOverride = 2500.f;

	// Optional: let icons rotate with actor yaw (relative to player).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	bool bIconsUseActorYaw = true;

	// Optional: add a world-space Z cutoff (ignore huge altitude deltas).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float ZTolerance = 100000.f;

	// Optional axis flips for how world offsets map into screen space.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Axes")
	bool bFlipX = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Axes")
	bool bFlipY = false;

public:
	UMinimapIconsWidget();

	//~ Begin UWidget Interface
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UWidget Interface

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	TSharedPtr<class SMinimapMeshWidget> MyMesh;
	int32 IconMeshId = -1;

	friend class SMinimapMeshWidget;
};