// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StaticMeshRenderData.generated.h"

UCLASS()
class GENERATEDCOMPLEXCOLLISION_API AStaticMeshRenderData : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStaticMeshRenderData();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	UMaterialInterface * Mat = nullptr;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UStaticMeshComponent * StaticMeshComponent = nullptr;
};
