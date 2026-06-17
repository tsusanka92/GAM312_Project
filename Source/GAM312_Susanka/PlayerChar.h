// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ResourceTypes.h"
#include "PlayerChar.generated.h"

class AResource_M;
class ABuildingPart;
class UPlayerWidget;
class UObjectiveWidget;

UCLASS()
class GAM312_SUSANKA_API APlayerChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerChar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
		void MovePlayerForward(float axisValue);

	UFUNCTION()
		void MovePlayerRight(float axisValue);

	UFUNCTION()
		void StartJump();

	UFUNCTION()
		void StopJump();

	UFUNCTION()
		void HandlePrimaryAction();

	UFUNCTION()
		void FindObject();
		
	UFUNCTION()
		void PlayerHurt();

	UPROPERTY(VisibleAnywhere)
		UCameraComponent* PlayerCamComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
		float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
		float Hunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
		float Stamina = 100.0f;

	UPROPERTY()
		FName PendingBuildingID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
		TArray<FInventoryResource> ResourceInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
		TArray<FBuildingRecipe> AvailableRecipes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
		TArray<FBuildingInventory> BuildingInventory;

	UPROPERTY(EditAnywhere, Category = "Hit Marker")
		UMaterialInterface* hitDecal;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
		float BuildDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
		float BuildDistanceStep = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
		float BuildDistanceLargeStep = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
		float MinBuildDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
		float MaxBuildDistance = 2000.0f;

	UPROPERTY()
		ABuildingPart* GhostPart = nullptr;

	UPROPERTY()
		float BuildRotationYaw = 0.0f;

	UPROPERTY()
		bool bGridModifierHeld = false;

	UPROPERTY()
		bool isBuilding = false;

	UPROPERTY()
		ABuildingPart* spawnedPart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UPlayerWidget* playerUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UObjectiveWidget* objWidget;

	UPROPERTY()
		float objectsBuilt;

	UPROPERTY()
		float matsCollected;

	UFUNCTION(BlueprintCallable)
		void SetHealth(float amount);

	UFUNCTION(BlueprintCallable)
		void SetHunger(float amount);

	UFUNCTION(BlueprintCallable)
		void SetStamina(float amount);

	UFUNCTION()
		void DecreaseStats();

	UFUNCTION(BlueprintCallable)
		bool CraftBuilding(FName BuildingID);

	UFUNCTION(BlueprintCallable)
		void SpawnBuilding(FName buildingID, bool& isSuccess);

	UFUNCTION(BlueprintCallable)
		void SetActiveBuilding(FName BuildingID);

	UFUNCTION()
		void ZoomPartIn();

	UFUNCTION()
		void ZoomPartOut();

	UFUNCTION()
		void GridTogglePressed();

	UFUNCTION()
		void GridToggleReleased();

	UFUNCTION()
		void CancelBuilding();

	UFUNCTION(BlueprintCallable)
		void ConfirmPlacement();

	UFUNCTION()
		void UpdateGhostTransform();

	UFUNCTION()
		void RotatePart(float AxisValue);

	UFUNCTION()
		bool ValidatePlacement();

	UFUNCTION()
		bool TrySnapToBuilding();

	const FBuildingRecipe* GetRecipe(FName BuildingID) const;
};
