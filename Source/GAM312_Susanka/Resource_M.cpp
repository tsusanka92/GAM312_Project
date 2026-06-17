// Fill out your copyright notice in the Description page of Project Settings.


#include "Resource_M.h"

// Sets default values
AResource_M::AResource_M()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ResourceNameTxt = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Text Render"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = Mesh;

	ResourceNameTxt->SetupAttachment(Mesh);
}

// Called when the game starts or when spawned
void AResource_M::BeginPlay()
{
	Super::BeginPlay();

	SyncResourceReserves();
	UpdateTotalMaxResource();
	UpdateTotalCurrentResource();

	FString DisplayText;

	for (const FHarvestResource& Resource : ResourceInfo)
	{
		if (!DisplayText.IsEmpty())
		{
			DisplayText += TEXT(", ");
		}

		DisplayText += Resource.ResourceID.ToString();
	}

	ResourceNameTxt->SetText(FText::FromString(DisplayText));
}

// Called every frame
void AResource_M::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AResource_M::UpdateTotalMaxResource()
{
	TotalResourceMax = 0;

	for (const FHarvestResource& Resource : ResourceInfo)
	{
		TotalResourceMax += Resource.MaximumReserve;
	}
}

void AResource_M::UpdateTotalCurrentResource()
{
	TotalResourceCurrent = 0;

	for (const FHarvestResource& Resource : ResourceInfo)
	{
		TotalResourceCurrent += Resource.CurrentReserve;
	}
}

void AResource_M::SyncResourceReserves()
{
	for (FHarvestResource& Resource : ResourceInfo)
	{
		Resource.CurrentReserve = Resource.MaximumReserve;
	}
}

#if WITH_EDITOR
void AResource_M::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	SyncResourceReserves();
	UpdateTotalMaxResource();
	UpdateTotalCurrentResource();
}
#endif