// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChar.h"

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
	BuildingArray.SetNum(3); //Initialize building array
	ResourcesArray.SetNum(3); //Initialize resource array
	ResourcesNameArray.Add(TEXT("Wood"));
	ResourcesNameArray.Add(TEXT("Stone"));
	ResourcesNameArray.Add(TEXT("Berry"));
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

	playerUI->UpdateBars(Health, Hunger, Stamina);

	//constant running debug to show the players current stats and inventory
	if (GEngine != nullptr)
	{
		//FString StatsText = FString::Printf(TEXT("Health: %.0f | Stamina: %.0f | Hunger: %.0f\nWood: %d | Stone: %d | Berry: %d"),
		//	Health, Stamina, Hunger,
		//	ResourcesArray[0], ResourcesArray[1], ResourcesArray[2]); //Set a text popup that reads out the players current stats and inventory.

		//GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Green, StatsText); //Adds a readout on the top left every frame
		
		if (isBuilding)
		{
			if (spawnedPart)
			{
				FVector StartLocation = PlayerCamComp->GetComponentLocation();
				FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;
				FVector EndLocation = StartLocation + Direction;
				spawnedPart->SetActorLocation(EndLocation);
			}
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
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar::FindObject);
	PlayerInputComponent->BindAction("PlayerHurt", IE_Pressed, this, &APlayerChar::PlayerHurt);
	PlayerInputComponent->BindAction("RotPart", IE_Pressed, this, &APlayerChar::RotateBuilding);
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

void APlayerChar::FindObject()
{
	//Init Linetrace
	FHitResult HitResult;
	FVector StartLocation = PlayerCamComp->GetComponentLocation(); //Line trace start location set to player cam origin
	FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f; //Establish a point 800 units in front of player cam
	FVector EndLocation = StartLocation + Direction; //Trace between start and direction

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
			if (HitResource && HitResource->totalResource > 0 && Stamina >= 5.0f) //Validate whether it is a resource, if yes, check to make sure there is resource left
			{
				FString hitName = HitResource->resourceName; //set variable = to resource name
				int resourceValue = HitResource->resourceAmount; //set variable = to resource amount (per hit, not total)
				int amountToGive = FMath::Min(HitResource->totalResource, resourceValue); //set a variable equal to the lesser between the remaining resources or the resource drain
				GiveResources(amountToGive, hitName); //update players resources with available amount
				HitResource->totalResource -= amountToGive; //Set hit resource to whatever amount has been reduced

				matsCollected = matsCollected + amountToGive;
				objWidget->UpdatematOBJ(matsCollected);

				//FString resourceLeftText = FString::Printf(TEXT("%s collected: %d | Remaining: %d"), 
				//	*HitResource->resourceName, amountToGive, HitResource->totalResource); //debug text showing type of resource, amount collected, left, and current player inventory

				//check(GEngine != nullptr);
				//GEngine->AddOnScreenDebugMessage(100, 5.0f, FColor::Yellow, resourceLeftText); //debug of the resource consumed

				//Check to see if node is depleted, if yes, delete
				if (HitResource->totalResource <= 0)
				{
					HitResource->Destroy();
					//check(GEngine != nullptr);
					//GEngine->AddOnScreenDebugMessage(101, 5.0f, FColor::Red, TEXT("Resource Depleted")); //debug removal of the node
				}

				UGameplayStatics::SpawnDecalAtLocation(GetWorld(), hitDecal, FVector(10.0f, 10.0f, 10.0f), HitResult.Location, FRotator(-90, 0, 0), 2.0f);
				SetStamina(-5.0f); //Drain player stamina

				//Save for Debug purposes
				/*if (hitName == "Berry")
				{
					SetStamina(10.0f);
					SetHunger(1.0f);
					GEngine->AddOnScreenDebugMessage(102, 5.0f, FColor::Blue, TEXT("You ate a berry")); //debug for player eating a berry
				}*/
			}
		}		
	}

	else
	{
		isBuilding = false;
		objectsBuilt = objectsBuilt + 1.0f;

		objWidget->UpdatebuildObj(objectsBuilt);
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

void APlayerChar::GiveResources(float amount, FString resourceType)
{
	if (resourceType == "Wood")
	{
		ResourcesArray[0] = ResourcesArray[0] + amount;
	}

	if (resourceType == "Stone")
	{
		ResourcesArray[1] = ResourcesArray[1] + amount;
	}

	if (resourceType == "Berry")
	{
		ResourcesArray[2] = ResourcesArray[2] + amount;
	}
}

void APlayerChar::UpdateResources(float woodAmount, float stoneAmount, FString buildingObject)
{
	if (woodAmount <= ResourcesArray[0])
	{
		if (stoneAmount <= ResourcesArray[1])
		{
			ResourcesArray[0] = ResourcesArray[0] - woodAmount;
			ResourcesArray[1] = ResourcesArray[1] - stoneAmount;

			if (buildingObject == "Wall")
			{
				BuildingArray[0] = BuildingArray[0] + 1;
			}
			if (buildingObject == "Floor")
			{
				BuildingArray[1] = BuildingArray[1] + 1;
			}
			if (buildingObject == "Ceiling")
			{
				BuildingArray[2] = BuildingArray[2] + 1;
			}
		}
	}
}

void APlayerChar::SpawnBuilding(int buildingID, bool& isSuccess)
{
	if (!isBuilding)
	{
		if (BuildingArray[buildingID] >= 1)
		{
			isBuilding = true;
			FActorSpawnParameters SpawnParams;
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;
			FVector EndLocation = StartLocation + Direction;
			FRotator myRot(0, 0, 0);

			BuildingArray[buildingID] = BuildingArray[buildingID] - 1;

			spawnedPart = GetWorld()->SpawnActor<ABuildingPart>(BuildPartClass, EndLocation, myRot, SpawnParams);

			isSuccess = true;
		}

		isSuccess = false;
	}
}

void APlayerChar::RotateBuilding()
{
	if (isBuilding)
	{
		spawnedPart->AddActorWorldRotation(FRotator(0, 90, 0));
	}
}
