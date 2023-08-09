// Fill out your copyright notice in the Description page of Project Settings.


#include "RenderDataExample.h"

// Sets default values
ARenderDataExample::ARenderDataExample()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ARenderDataExample::BeginPlay()
{
	Super::BeginPlay();


	UStaticMeshComponent* mesh = NewObject<UStaticMeshComponent>(this);
	mesh->SetFlags(RF_Transient);
	// 创建一个 UStaticMesh
	UStaticMesh* pStaticMesh = NewObject<UStaticMesh>(mesh);
	mesh->SetStaticMesh(pStaticMesh);
	pStaticMesh->NeverStream = true;
	// 创建 FStaticMeshRenderData
	FStaticMeshRenderData* RenderData = new FStaticMeshRenderData();
	// 初始化LOD信息
	RenderData->AllocateLODResources(1);
	// 获取 之前创建 RenderData 的LOD(上一步申请的)
	FStaticMeshLODResources& LODResources = RenderData->LODResources[0];

	// 构造索引数组
	TArray<uint32> indices;
	indices.SetNum(3);
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	// 应用索引数组
	LODResources.IndexBuffer.SetIndices(indices, EIndexBufferStride::Type::AutoDetect);

	LODResources.bHasDepthOnlyIndices = false;
	LODResources.bHasReversedIndices = false;
	LODResources.bHasReversedDepthOnlyIndices = false;
	//LODResources.bHasAdjacencyInfo = false;

	// 构造顶点数据数组
	TArray<FStaticMeshBuildVertex> StaticMeshBuildVertices;
	StaticMeshBuildVertices.SetNum(indices.Num());

	FStaticMeshBuildVertex& vertex0 = StaticMeshBuildVertices[0];
	vertex0.Position = FVector(0, 0, 0);
	vertex0.Color = FColor(255, 0, 0);
	vertex0.UVs[0] = FVector2D(0, 0);
	vertex0.TangentX = FVector(0, 1, 0);
	vertex0.TangentY = FVector(1, 0, 0);
	vertex0.TangentY = FVector(0, 0, 1);

	FStaticMeshBuildVertex& vertex1 = StaticMeshBuildVertices[1];
	vertex1.Position = FVector(100, 0, 0);
	vertex1.Color = FColor(255, 0, 0);
	vertex1.UVs[0] = FVector2D(1, 0);
	vertex1.TangentX = FVector(0, 1, 0);
	vertex1.TangentY = FVector(1, 0, 0);
	vertex1.TangentY = FVector(0, 0, 1);

	FStaticMeshBuildVertex& vertex2 = StaticMeshBuildVertices[2];
	vertex2.Position = FVector(0, 100, 0);
	vertex2.Color = FColor(255, 0, 0);
	vertex2.UVs[0] = FVector2D(1, 0);
	vertex2.TangentX = FVector(0, 1, 0);
	vertex2.TangentY = FVector(1, 0, 0);
	vertex2.TangentY = FVector(0, 0, 1);

	// 顶点颜色
	LODResources.bHasColorVertexData = false;

	// 构造BoundingBox
	FBoxSphereBounds BoundingBoxAndSphere;
	BoundingBoxAndSphere.Origin = FVector(0, 0, 0);
	BoundingBoxAndSphere.SphereRadius = 100.0f;
	BoundingBoxAndSphere.BoxExtent = FVector(50.f);
	// 应用BoundingBox
	RenderData->Bounds = BoundingBoxAndSphere;

	// 应用顶点数组
	LODResources.VertexBuffers.PositionVertexBuffer.Init(StaticMeshBuildVertices);

	// 应用顶点颜色
	FColorVertexBuffer& ColorVertexBuffer = LODResources.VertexBuffers.ColorVertexBuffer;
	ColorVertexBuffer.Init(StaticMeshBuildVertices);

	//法线，切线，贴图坐标buffer
	LODResources.VertexBuffers.StaticMeshVertexBuffer.Init(StaticMeshBuildVertices, 1);

	// 获取 LODResources 的 Sections 数组
	FStaticMeshLODResources::FStaticMeshSectionArray& Sections = LODResources.Sections;

	// 构造 FStaticMeshSection
	FStaticMeshSection& section = Sections.AddDefaulted_GetRef();

	section.bEnableCollision = false;
	// 计算三角面数 = 顶点数 / 3 ;
	section.NumTriangles = StaticMeshBuildVertices.Num() / 3;

	// 设置索引范围 0 - (3 - 1) -> 0-2
	section.FirstIndex = 0;
	section.MinVertexIndex = 0;
	section.MaxVertexIndex = StaticMeshBuildVertices.Num() - 1;

	section.bEnableCollision = true;
	//section.bCastShadow = true;

	// 将创建的RenderData传递给UStaticMesh
	pStaticMesh->RenderData = TUniquePtr<FStaticMeshRenderData>(RenderData);

	// 获取材质
	UMaterial* Material = UMaterial::GetDefaultMaterial(MD_Surface);
	Material->TwoSided = true;
	// Material->Wireframe = true;

	// 给FStaticMeshSection添加材质
	section.MaterialIndex = 0;
	pStaticMesh->AddMaterial(Material);

	// 调用UStaticMesh的修改数据接口
	pStaticMesh->InitResources();
	// 重新计算Bounds
	pStaticMesh->CalculateExtendedBounds();

	pStaticMesh->RenderData->ScreenSize[0].Default = 1.0f;

	pStaticMesh->CreateBodySetup();

	// UStaticMeshComponent相关接口
	mesh->SetMobility(EComponentMobility::Movable);
	mesh->SetupAttachment(this->RootComponent);
	mesh->RegisterComponent();
}
