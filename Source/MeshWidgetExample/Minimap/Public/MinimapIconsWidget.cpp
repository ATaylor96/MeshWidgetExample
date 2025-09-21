// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.


#include "MinimapIconsWidget.h"
#include "MeshWidgetExample/MeshWidgetExample.h"
#include "Slate/SMeshWidget.h"
#include "Slate/SlateVectorArtInstanceData.h"
#include "MinimapManager.h"
#include "Kismet/GameplayStatics.h"


// ------- Slate widget that mirrors SParticleMeshWidget pattern -------
class SMinimapMeshWidget : public SMeshWidget
{
public:
	SLATE_BEGIN_ARGS(SMinimapMeshWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& Args, UMinimapIconsWidget& InOwner)
	{
		Owner = &InOwner;
	}

	virtual int32 OnPaint(const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyClippingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override
	{
		// Early out: no mesh
		if (!Owner || Owner->IconMeshId == -1)
		{
			return SMeshWidget::OnPaint(Args, AllottedGeometry, MyClippingRect,
				OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
		}

		const UWorld* World = Owner->GetWorld();
		if (!World)
		{
			return SMeshWidget::OnPaint(Args, AllottedGeometry, MyClippingRect,
				OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
		}

		// Widget geometry (DPI-aware absolute pixels)
		const FVector2D SizeLocal = AllottedGeometry.GetLocalSize();
		const FVector2D AbsTL = AllottedGeometry.LocalToAbsolute(FVector2D(0, 0));
		const FVector2D AbsBR = AllottedGeometry.LocalToAbsolute(FVector2D(SizeLocal.X, SizeLocal.Y));
		const FVector2D SizeAbs = AbsBR - AbsTL;                  // absolute (desktop) pixels
		const FVector2D CenterAbs = AbsTL + SizeAbs * 0.5f;       // center in absolute pixels

		// Dynamic pixels-per-world calculation:
		// Effective world width shown on the minimap = OrthoWidth / MaterialZoom
		const float EffectiveWorldWidth = Owner->OrthoWidthUU / FMath::Max(Owner->MaterialZoom, 0.001f);
		const float AbsWidthPx = FMath::Max(SizeAbs.X, 1.f);
		const float UnitsPerPixel = EffectiveWorldWidth / AbsWidthPx; // UU per screen pixel

		// Player transform
		const APlayerController* PC = World->GetFirstPlayerController();
		const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		const FVector PlayerLoc = Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector;
		const float PlayerYawDeg = Pawn ? Pawn->GetActorRotation().Yaw + 90.f : 0.f;
		const float PlayerYawRad = FMath::DegreesToRadians(PlayerYawDeg);

		// Manager lookup (first found instance)
		AMinimapManager* Manager = nullptr;
		{
			TArray<AActor*> Found;
			UGameplayStatics::GetAllActorsOfClass(const_cast<UWorld*>(World), AMinimapManager::StaticClass(), Found);
			if (Found.Num() > 0)
			{
				Manager = Cast<AMinimapManager>(Found[0]);
			}
		}

		FSlateInstanceBufferData PerInstanceUpdate;

		if (Manager)
		{
			TArray<FMinimapEntry> Entries;
			Manager->GetEntries(Entries);

			// Auto clamp = visible circle radius in UU
			const float ClampWorldRadius =
				Owner->bAutoClampToVisible ? (EffectiveWorldWidth * 0.5f) : FMath::Max(Owner->ClampWorldRadiusOverride, 0.f);
			const float ClampRadiusSqr = (ClampWorldRadius > 0.f) ? ClampWorldRadius * ClampWorldRadius : FLT_MAX;

			// Precompute rotation (rotate map under the player)
			const float c = FMath::Cos(-PlayerYawRad);
			const float s = FMath::Sin(-PlayerYawRad);

			for (const FMinimapEntry& E : Entries)
			{
				if (!E.Actor.IsValid())
				{
					continue;
				}

				// Optional altitude filter
				if (Owner->ZTolerance > 0.f && FMath::Abs(E.WorldLocation.Z - PlayerLoc.Z) > Owner->ZTolerance)
				{
					continue;
				}

				// World ? player-relative
				const FVector Rel = (E.WorldLocation - PlayerLoc);

				// 2D clamp
				const float Dist2DSqr = Rel.X * Rel.X + Rel.Y * Rel.Y;
				if (Dist2DSqr > ClampRadiusSqr)
				{
					continue;
				}

				// Rotate offsets by -PlayerYaw to keep player "up"
				const float RX = c * Rel.X - s * Rel.Y;
				const float RY = s * Rel.X + c * Rel.Y;

				// Map to screen pixel space.
				// Start with Y-up (screen is Y-down) ? apply minus on RY first, then allow flips.
				float SX = RX;
				float SY = -RY;

				if (Owner->bFlipX) SX = -SX;
				if (Owner->bFlipY) SY = -SY;

				const float PX = SX / UnitsPerPixel;
				const float PY = SY / UnitsPerPixel;

				// Absolute pixel placement at widget center. DO NOT multiply by geometry scale here—
				// LocalToAbsolute already gave us DPI-scaled absolute space. (Fixes size wobble).
				const FVector2D IconAbs = CenterAbs + FVector2D(PX, PY);

				// Icon rotation (invert so it “matches” minimap rotation visually).
				// Old: (Actor - Player). New (inverted): (Player - Actor).
				float IconYawRad = 0.f;
				if (Owner->bIconsUseActorYaw)
				{
					const float ActorYaw = E.Actor->GetActorRotation().Yaw;
					IconYawRad = FMath::DegreesToRadians(PlayerYawDeg - ActorYaw);
				}

				// Pack per-instance (X, Y, RotRad, AtlasIndex)
				const FVector4 Packed(IconAbs.X, IconAbs.Y, IconYawRad, static_cast<float>(E.IconStyleIndex));
				PerInstanceUpdate.Add(TArray<UE::Math::TVector4<float>>::ElementType(Packed));
			}
		}

		const_cast<SMinimapMeshWidget*>(this)->UpdatePerInstanceBuffer(Owner->IconMeshId, PerInstanceUpdate);

		return SMeshWidget::OnPaint(Args, AllottedGeometry, MyClippingRect,
			OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}

private:
	UMinimapIconsWidget* Owner = nullptr;
};

// ---------------- UMinimapIconsWidget ----------------

UMinimapIconsWidget::UMinimapIconsWidget()
{
}

void UMinimapIconsWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (IconMeshAsset)
	{
		constexpr int32 MaxIcons = 2048; // tune for your needs
		IconMeshId = MyMesh->AddMesh(*IconMeshAsset);
		MyMesh->EnableInstancing(IconMeshId, MaxIcons);
	}
}

void UMinimapIconsWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyMesh.Reset();
	IconMeshId = -1;
}

TSharedRef<SWidget> UMinimapIconsWidget::RebuildWidget()
{
	MyMesh = SNew(SMinimapMeshWidget, *this);
	return MyMesh.ToSharedRef();
}