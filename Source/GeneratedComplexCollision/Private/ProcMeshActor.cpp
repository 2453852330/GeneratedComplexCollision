// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcMeshActor.h"

#include "ProceduralMeshComponent.h"
#include "MeshData.h"

// Sets default values
AProcMeshActor::AProcMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
	RootComponent = ProceduralMeshComponent;
}

// Called when the game starts or when spawned
void AProcMeshActor::BeginPlay()
{
	Super::BeginPlay();
	TArray<int32> index;
	for (uint32 it : MeshData::Index)
	{
		index.Add(it);
	} 
	ProceduralMeshComponent->CreateMeshSection_LinearColor(0,MeshData::Vertex,index,{},{},MeshData::Color,{},true);
}

