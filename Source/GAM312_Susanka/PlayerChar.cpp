// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChar.h"

#include "Resource_M.h"
#include "BuildingPart.h"
#include "PlayerWidget.h"
#include "Engine/OverlapResult.h"
#include "ObjectiveWidget.h"

// Sets default values
APlayerChar::APlayerChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Camera setup
	PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Cam"));

	//Attach camera to head
	PlayerCamComp->SetupAttachment(GetMesh(), "head");
	
	//Set camera rotation to match character
	PlayerCamComp->bUsePawnControlRotation = true;

	//Initialize Arrays
	ResourceInfo.Empty();
}

// Called when the game starts or when spawned
void APlayerChar::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerHandle StatsTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(StatsTimerHandle, this, &APlayerChar::DecreaseStats, 2.0f, true);

	if (objWidget)
	{
		objWidget->UpdatebuildObj(0.0f);
		objWidget->UpdatematOBJ(0.0f);
	}
}

// Called every frame
void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (playerUI)
	{
		playerUI->UpdateBars(Health, Hunger, Stamina);
	}
	
	if (isBuilding)
	{
		if (GhostPart)
		{
			UpdateGhostTransform();
		}
		else
		{
			isBuilding = false; // safety recovery
		}
	}
}

// Called to bind functionality to input
void APlayerChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Axis Inputs
	PlayerInputComponent->BindAxis("PlayerForward", this, &APlayerChar::MovePlayerForward);
	PlayerInputComponent->BindAxis("MovePlayerRight", this, &APlayerChar::MovePlayerRight);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerChar::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerChar::AddControllerYawInput);

	//Action Inputs
	PlayerInputComponent->BindAction("JumpEvent", IE_Pressed, this, &APlayerChar::StartJump);
	PlayerInputComponent->BindAction("JumpEvent", IE_Released, this, &APlayerChar::StopJump);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar::HandlePrimaryAction);
	PlayerInputComponent->BindAction("PlayerHurt", IE_Pressed, this, &APlayerChar::PlayerHurt);
	PlayerInputComponent->BindAction("Cancel", IE_Pressed, this, &APlayerChar::CancelBuilding);
	PlayerInputComponent->BindAction("ZoomPartIn", IE_Pressed, this, &APlayerChar::ZoomPartIn);
	PlayerInputComponent->BindAction("ZoomPartOut", IE_Pressed, this, &APlayerChar::ZoomPartOut);
	PlayerInputComponent->BindAction("GridToggle", 	IE_Pressed, this, &APlayerChar::GridTogglePressed);
	PlayerInputComponent->BindAction("GridToggle", IE_Released, this, &APlayerChar::GridToggleReleased);
	PlayerInputComponent->BindAxis("Rotate", this, &APlayerChar::RotatePart);
}

void APlayerChar::MovePlayerForward(float axisValue)
{
	FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0); //Get player rotation without pitch
	FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); //Use our defined rotation to input our movement
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::MovePlayerRight(float axisValue)
{
	FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0); //Get player rotation without pitch
	FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); //Use our defined rotation to input our movement
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::StartJump()
{
	bPressedJump = true;
}

void APlayerChar::StopJump()
{
	bPressedJump = false;
}

void APlayerChar::HandlePrimaryAction()
{
	if (isBuilding && GhostPart)
	{
		ConfirmPlacement();
	}
	else
	{
		FindObject();
	}
}

void APlayerChar::FindObject()
{
	//Init Line Trace
	FHitResult HitResult;
	FVector StartLocation = PlayerCamComp->GetComponentLocation(); //Line trace start location set to player cam origin 
	FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f; //Establish a point 800 units in front of player cam
	FVector EndLocation = StartLocation + Direction;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); //ignore player char 
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnFaceIndex = true;

	if (!isBuilding) //Perform Line Trace. Pass in necessary/desired variables. 
	{

		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
		{
			AResource_M* HitResource = Cast<AResource_M>(HitResult.GetActor()); //Cast to the hit actor, if resource, set it to variable. 

			//If the hit was a resource, do this. 
			//if (!HitResource || Stamina < 5.0f)
			//	return;

			if (HitResource && Stamina >= 5.0f) //Validate whether it is a resource, if yes, check to make sure there is resource left 
			{
				for (FHarvestResource& Resource : HitResource->ResourceInfo)
				{
					if (Resource.CurrentReserve <= 0)
						continue;

					int AmountToGive = FMath::Min(Resource.CurrentReserve, Resource.HarvestAmount);

					if (AmountToGive <= 0)
						continue;
					bool bFound = false; //if an inventoried item, add it in 

					for (FInventoryResource& InvItem : ResourceInfo)
					{
						if (InvItem.ResourceID == Resource.ResourceID)
						{
							InvItem.Amount += AmountToGive;

							if (Resource.Category == EResourceCategory::Food)
							{
								InvItem.FoodEffects = Resource.FoodEffects;
							}

							else if (Resource.Category == EResourceCategory::Material)
							{
								InvItem.MaterialEffects = Resource.MaterialEffects;
							}

							bFound = true;
							break;
						}
					}

					//if not currently in inventory, create a new inventory holder 
					if (!bFound)
					{
						FInventoryResource NewItem;
						NewItem.ResourceID = Resource.ResourceID;
						NewItem.Amount = AmountToGive;
						NewItem.Category = Resource.Category;

						if (Resource.Category == EResourceCategory::Food)
						{
							NewItem.FoodEffects = Resource.FoodEffects;
						}

						else if (Resource.Category == EResourceCategory::Material)
						{
							NewItem.MaterialEffects = Resource.MaterialEffects;
						}

						ResourceInfo.Add(NewItem);
					}

					//deplete the resource node 
					Resource.CurrentReserve -= AmountToGive;
					matsCollected += AmountToGive;
					objWidget->UpdatematOBJ(matsCollected);
				}

				//Destroy empty nodes 
				bool bEmpty = true;

				for (const FHarvestResource& Resource : HitResource->ResourceInfo)
				{
					if (Resource.CurrentReserve > 0)
					{
						bEmpty = false;
						break;
					}
				}

				if (bEmpty)
				{
					HitResource->Destroy();
				}

				UGameplayStatics::SpawnDecalAtLocation
				(GetWorld(),
					hitDecal,
					FVector(10.0f, 10.0f, 10.0f),
					HitResult.Location,
					FRotator(-90, 0, 0), 2.0f);

				SetStamina(-5.0f); //Drain player stamina 
			}
		}
	}
}

void APlayerChar::PlayerHurt()
{
	SetHunger(-25.0f);
}

void APlayerChar::SetHealth(float amount)
{
	if (Health + amount < 100.0f)
	{
		Health = Health + amount;
	}
	else
	{
		Health = 100.0f;
	}
}

void APlayerChar::SetHunger(float amount)
{
	if (Hunger + amount < 100.0f)
	{
		Hunger = Hunger + amount;
	}
	else
	{
		Hunger = 100.0f;
	}
}

void APlayerChar::SetStamina(float amount)
{
	if (Stamina + amount < 100.0f)
	{
		Stamina = Stamina + amount;
	}
	else
	{
		Stamina = 100.0f;
	}
}

void APlayerChar::DecreaseStats()
{
	SetStamina(10.0f);

	if (Hunger > 0)
	{
		SetHunger(-1.0f);
	}

	if (Hunger <= 0)
	{
		SetHealth(-25.0f);
	}
}

const FBuildingRecipe* APlayerChar::GetRecipe(FName BuildingID) const
{
	for (const FBuildingRecipe& R : AvailableRecipes)
	{
		if (R.BuildingID == BuildingID)
		{
			return &R;
		}
	}

	return nullptr;
}

bool APlayerChar::CraftBuilding(FName BuildingID)
{
	const FBuildingRecipe* Recipe = GetRecipe(BuildingID);
	if (!Recipe)
		return false;

	// Validate all costs first
	for (const FBuildCost& Cost : Recipe->Costs)
	{
		FInventoryResource* InvItem = nullptr;

		for (FInventoryResource& Inv : ResourceInfo)
		{
			if (Inv.ResourceID == Cost.ResourceID)
			{
				InvItem = &Inv;
				break;
			}
		}

		if (!InvItem || InvItem->Amount < Cost.Amount)
			return false;
	}
	
	// Deduct after validation
	for (const FBuildCost& Cost : Recipe->Costs)
	{
		for (FInventoryResource& Inv : ResourceInfo)
		{
			if (Inv.ResourceID == Cost.ResourceID)
			{
				Inv.Amount -= Cost.Amount;
				break;
			}
		}
	}

	// Add to building inventory
	FBuildingInventory* Existing = nullptr;

	for (FBuildingInventory& Item : BuildingInventory)
	{
		if (Item.BuildingID == BuildingID)
		{
			Existing = &Item;
			break;
		}
	}

	if (Existing)
	{
		Existing->Amount++;
	}
	else
	{
		FBuildingInventory NewItem;
		NewItem.BuildingID = BuildingID;
		NewItem.Amount = 1;
		NewItem.BuildingClass = Recipe->BuildingClass;
		BuildingInventory.Add(NewItem);
	}

	return true;
}

void APlayerChar::SpawnBuilding(FName BuildingID, bool& isSuccess)
{
	SetActiveBuilding(BuildingID);
	isSuccess = (GhostPart != nullptr);
}

void APlayerChar::SetActiveBuilding(FName BuildingID)
{
	PendingBuildingID = BuildingID;

	const FBuildingRecipe* Recipe = GetRecipe(BuildingID);
	if (!Recipe)
		return;

	bool bHasBuilding = false;

	for (const FBuildingInventory& Item : BuildingInventory)
	{
		if (Item.BuildingID == BuildingID &&
			Item.Amount > 0)
		{
			bHasBuilding = true;
			break;
		}
	}

	if (!bHasBuilding)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("No %s available in building inventory"),
			*BuildingID.ToString()
		);

		return;
	}

	/*for (const FBuildCost& Cost : Recipe->Costs)
	{
		bool bHasItem = false;

		for (const FInventoryResource& Inv : ResourceInfo)
		{
			if (Inv.ResourceID == Cost.ResourceID &&
				Inv.Amount >= Cost.Amount)
			{
				bHasItem = true;
				break;
			}
		}

		if (!bHasItem)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("Cannot build %s: missing resources"),
				*BuildingID.ToString());

			return;
		}
	}*/

	if (GhostPart)
	{
		GhostPart->Destroy();
		GhostPart = nullptr;
	}

	FActorSpawnParameters SpawnParams;

	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	FVector EndLocation = StartLocation + (PlayerCamComp->GetForwardVector() * BuildDistance);
	FRotator Rotation(0.0f, BuildRotationYaw, 0.0f);

	GhostPart = GetWorld()->SpawnActor<ABuildingPart>(
		Recipe->BuildingClass,
		EndLocation,
		Rotation,
		SpawnParams
	);

	if (!GhostPart)
		return;

	GhostPart->SetActorEnableCollision(false);
	GhostPart->SetGhostMode(true);

	isBuilding = true;
}

/*void APlayerChar::SetActiveBuilding(FName BuildingID)
{
	PendingBuildingID = BuildingID;

	const FBuildingRecipe* Recipe = GetRecipe(BuildingID);
	if (!Recipe)
		return;

	// If we already have a ghost, just update it
	if (GhostPart)
	{
		// Swap mesh / class representation instead of respawning
		GhostPart->Destroy();
		GhostPart = nullptr;
	}

	FActorSpawnParameters SpawnParams;

	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	FVector EndLocation = StartLocation + (PlayerCamComp->GetForwardVector() * BuildDistance);
	FRotator Rotation(0.0f, BuildRotationYaw, 0.0f);

	GhostPart = GetWorld()->SpawnActor<ABuildingPart>(
		Recipe->BuildingClass,
		EndLocation,
		Rotation,
		SpawnParams
	);

	if (!GhostPart)
		return;

	GhostPart->SetActorEnableCollision(false);
	GhostPart->SetGhostMode(true); // if you implemented earlier

	isBuilding = true;
}*/

void APlayerChar::ConfirmPlacement()
{
	if (!GhostPart->bPlacementValid)
	{
		return;
	}

	if (!isBuilding || !GhostPart || PendingBuildingID == NAME_None)
		return;

	FBuildingInventory* Item = nullptr;

	for (FBuildingInventory& I : BuildingInventory)
	{
		if (I.BuildingID == PendingBuildingID)
		{
			Item = &I;
			break;
		}
	}

	if (!Item || Item->Amount <= 0)
		return;

	Item->Amount--;

	//GhostPart->SetActorLocation(
	//	PlayerCamComp->GetComponentLocation() +
	//	PlayerCamComp->GetForwardVector() * BuildDistance
	//);

	//GhostPart->SetActorRotation(FRotator(0.0f, BuildRotationYaw, 0.0f));

	GhostPart->SetGhostMode(false);
	GhostPart->SetActorEnableCollision(true);

	GhostPart = nullptr;
	isBuilding = false;
	PendingBuildingID = NAME_None;

	objectsBuilt++;
	objWidget->UpdatebuildObj(objectsBuilt);
}

void APlayerChar::UpdateGhostTransform()
{
	if (!GhostPart)
		return;

	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	FVector EndLocation = StartLocation + (PlayerCamComp->GetForwardVector() * BuildDistance);

	GhostPart->SetActorLocation(EndLocation);

	FRotator Rotation(0.0f, BuildRotationYaw, 0.0f);
	GhostPart->SetActorRotation(Rotation);

	TrySnapToBuilding();

	GhostPart->SetPlacementValid(
		ValidatePlacement()
	);
}

void APlayerChar::CancelBuilding()
{
	if (GhostPart)
	{
		GhostPart->Destroy();
		GhostPart = nullptr;
	}

	isBuilding = false;
	PendingBuildingID = NAME_None;
}

void APlayerChar::ZoomPartIn()
{
	float Step = bGridModifierHeld
		? BuildDistanceLargeStep
		: BuildDistanceStep;

	BuildDistance = FMath::Clamp(
		BuildDistance - Step,
		MinBuildDistance,
		MaxBuildDistance
	);

	if (GhostPart)
	{
		UpdateGhostTransform();
	}
}

void APlayerChar::ZoomPartOut()
{
	float Step = bGridModifierHeld
		? BuildDistanceLargeStep
		: BuildDistanceStep;

	BuildDistance = FMath::Clamp(
		BuildDistance + Step,
		MinBuildDistance,
		MaxBuildDistance
	);

	if (GhostPart)
	{
		UpdateGhostTransform();
	}
}

void APlayerChar::GridTogglePressed()
{
	bGridModifierHeld = true;
}

void APlayerChar::GridToggleReleased()
{
	bGridModifierHeld = false;
}

void APlayerChar::RotatePart(float AxisValue)
{
	if (!isBuilding)
		return;

	if (FMath::IsNearlyZero(AxisValue))
		return;

	float RotationStep =
		bGridModifierHeld
		? 90.0f
		: 22.5f;

	BuildRotationYaw += FMath::Sign(AxisValue) * RotationStep;

	BuildRotationYaw = FMath::Fmod(BuildRotationYaw, 360.0f);

	if (BuildRotationYaw < 0)
	{
		BuildRotationYaw += 360.0f;
	}

	if (GhostPart)
	{
		UpdateGhostTransform();
	}
}

bool APlayerChar::ValidatePlacement()
{
	if (!GhostPart)
		return false;

	FCollisionShape Shape =
		GhostPart->Mesh->GetCollisionShape();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GhostPart);

	TArray<FOverlapResult> Results;

	GetWorld()->OverlapMultiByObjectType(
		Results,
		GhostPart->GetActorLocation(),
		GhostPart->GetActorQuat(),
		FCollisionObjectQueryParams::AllObjects,
		Shape,
		Params
	);

	for (const FOverlapResult& Result : Results)
	{
		AActor* Actor = Result.GetActor();
		if (!Actor)
			continue;

		// BLOCK ONLY THESE
		if (Actor->IsA(AResource_M::StaticClass()))
			return false;

		if (Actor->IsA(ACharacter::StaticClass()))
			return false;

		//if (Actor->IsA(ABuildingPart::StaticClass()))
			//return false;

		// OPTIONAL: if you ever add enemies
		// if (Actor->IsA(AEnemy::StaticClass()))
		//     return false;

		// EVERYTHING ELSE (Landscape, Water, etc.) is allowed
	}

	for (const FOverlapResult& Result : Results)
	{
		AActor* HitActor = Result.GetActor();

		if (HitActor)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Placement overlap: %s"),
				*HitActor->GetName()
			);
		}
	}

	return true;
}

bool APlayerChar::TrySnapToBuilding()
{
	if (!GhostPart)
		return false;

	const float SnapRadius = 150.0f;

	FCollisionShape Shape =
		FCollisionShape::MakeSphere(SnapRadius);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GhostPart);

	TArray<FOverlapResult> Results;

	GetWorld()->OverlapMultiByChannel(
		Results,
		GhostPart->GetActorLocation(),
		FQuat::Identity,
		ECC_GameTraceChannel1,
		Shape,
		Params
	);

	for (const FOverlapResult& Result : Results)
	{
		UPrimitiveComponent* SnapComp =
			Result.GetComponent();

		if (!SnapComp)
			continue;

		TArray<USceneComponent*> GhostSnapPoints =
			GhostPart->GetSnapPoints();

		if (GhostSnapPoints.Num() == 0)
		{
			return false;
		}

		float BestDistance = TNumericLimits<float>::Max();
		USceneComponent* BestGhostSnap = nullptr;

		for (USceneComponent* GhostSnap : GhostSnapPoints)
		{
			if (!GhostSnap)
				continue;

			float Dist = FVector::DistSquared(
				GhostSnap->GetComponentLocation(),
				SnapComp->GetComponentLocation()
			);

			if (Dist < BestDistance)
			{
				BestDistance = Dist;
				BestGhostSnap = GhostSnap;
			}
		}

		if (!BestGhostSnap)
		{
			continue;
		}

		FVector Offset =
			BestGhostSnap->GetComponentLocation()
			- GhostPart->GetActorLocation();

		GhostPart->SetActorLocation(
			SnapComp->GetComponentLocation()
			- Offset
		);

		return true;
	}

	return false;
}