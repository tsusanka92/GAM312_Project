// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Materials/MaterialInterface.h"
#include "BuildingPart.generated.h"

UCLASS()
class GAM312_SUSANKA_API ABuildingPart : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuildingPart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere)
		UArrowComponent* PivotArrow;

	UPROPERTY(EditAnywhere)
		FName BuildingID;

	UPROPERTY()
		bool bIsGhost = false;

	UPROPERTY(BlueprintReadOnly)
		bool bPlacementValid = true;

	UFUNCTION()
		void SetPlacementValid(bool bValid);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
		UMaterialInterface* MainMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
		UMaterialInterface* ValidMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
		UMaterialInterface* InvalidMaterial = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snap")
		USceneComponent* SnapRoot;

	UFUNCTION(BlueprintCallable)
		TArray<USceneComponent*> GetSnapPoints() const;

	UFUNCTION()
		void SetGhostMode(bool bGhost);
};
