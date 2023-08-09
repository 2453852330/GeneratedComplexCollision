// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProcMeshActor.generated.h"

class UProceduralMeshComponent;

UCLASS()
class GENERATEDCOMPLEXCOLLISION_API AProcMeshActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProcMeshActor();
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UProceduralMeshComponent * ProceduralMeshComponent = nullptr;
	
};
