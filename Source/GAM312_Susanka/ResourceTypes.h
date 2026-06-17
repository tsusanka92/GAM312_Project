#pragma once

#include "CoreMinimal.h"
#include "ResourceTypes.generated.h"

class ABuildingPart;

UENUM(BlueprintType)
enum class EResourceCategory : uint8
{
    Material UMETA(DisplayName = "Material"),
    Food UMETA(DisplayName = "Food"),
    Blank UMETA(DisplayName = "Blank")
};

USTRUCT(BlueprintType)
struct FFoodEffects
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Hunger = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Health = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Stamina = 0.f;
};

USTRUCT(BlueprintType)
struct FMaterialEffects
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Durability = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Structural = 0.f;
};

USTRUCT(BlueprintType)
struct FHarvestResource
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName ResourceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int HarvestAmount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
        int MaximumReserve = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        int CurrentReserve = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EResourceCategory Category = EResourceCategory::Blank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Category == EResourceCategory::Food"))
        FFoodEffects FoodEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Category == EResourceCategory::Material"))
        FMaterialEffects MaterialEffects;
};

USTRUCT(BlueprintType)
struct FInventoryResource
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName ResourceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int Amount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EResourceCategory Category = EResourceCategory::Blank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Category == EResourceCategory::Food"))
        FFoodEffects FoodEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Category == EResourceCategory::Material"))
        FMaterialEffects MaterialEffects;
};

USTRUCT(BlueprintType)
struct FBuildCost
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName ResourceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int Amount = 0;
};

USTRUCT(BlueprintType)
struct FBuildingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName BuildingID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TSubclassOf<ABuildingPart> BuildingClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FBuildCost> Costs;
};

USTRUCT(BlueprintType)
struct FBuildingInventory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName BuildingID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int Amount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TSubclassOf<ABuildingPart> BuildingClass;
};