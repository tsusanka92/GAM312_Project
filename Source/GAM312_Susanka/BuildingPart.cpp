// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingPart.h"

#include "Engine/EngineTypes.h"
#include "Engine/CollisionProfile.h"

// Sets default values
ABuildingPart::ABuildingPart()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	PivotArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Pivot Arrow"));

	RootComponent = PivotArrow;
	Mesh->SetupAttachment(PivotArrow);

	SnapRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SnapRoot"));
	SnapRoot->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABuildingPart::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABuildingPart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABuildingPart::SetGhostMode(bool bGhost)
{
    bIsGhost = bGhost;

    if (!Mesh)
        return;

    if (bGhost)
    {
        Mesh->SetGenerateOverlapEvents(true);

        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

        Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);

        Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
        Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

        if (ValidMaterial)
        {
            Mesh->SetMaterial(0, ValidMaterial);
        }
    }
    else
    {
        Mesh->SetGenerateOverlapEvents(false);

        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        Mesh->SetCollisionObjectType(ECC_WorldDynamic);

        Mesh->SetCollisionResponseToAllChannels(ECR_Block);

        // Ignore BuildSnap channel
        Mesh->SetCollisionResponseToChannel(
            ECC_GameTraceChannel1,
            ECR_Ignore
        );

        if (MainMaterial)
        {
            Mesh->SetMaterial(0, MainMaterial);
        }
    }
}

void ABuildingPart::SetPlacementValid(bool bValid)
{
	bPlacementValid = bValid;

	if (!Mesh)
		return;

	if (bValid)
	{
		if (ValidMaterial)
		{
			Mesh->SetMaterial(0, ValidMaterial);
		}
	}
	else
	{
		if (InvalidMaterial)
		{
			Mesh->SetMaterial(0, InvalidMaterial);
		}
	}
}

TArray<USceneComponent*> ABuildingPart::GetSnapPoints() const
{
    TArray<USceneComponent*> SnapPoints;

    if (!SnapRoot)
    {
        return SnapPoints;
    }

    SnapRoot->GetChildrenComponents(false, SnapPoints);

    return SnapPoints;
}