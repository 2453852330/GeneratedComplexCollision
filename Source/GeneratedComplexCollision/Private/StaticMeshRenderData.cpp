// Fill out your copyright notice in the Description page of Project Settings.


#include "StaticMeshRenderData.h"
#include "MeshData.h"


// Sets default values
AStaticMeshRenderData::AStaticMeshRenderData()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
}

// Called when the game starts or when spawned
void AStaticMeshRenderData::BeginPlay()
{
	Super::BeginPlay();

	// 1.Create UStaticMesh
	// Outer没有具体指向,也会导致没有复杂碰撞
	UStaticMesh* StaticMesh = NewObject<UStaticMesh>(StaticMeshComponent);
	StaticMesh->bAllowCPUAccess = true;
	
	// 2.Create Render Data
	if (StaticMesh->RenderData.IsValid())
	{
		StaticMesh->RenderData->ReleaseResources();
	}
	StaticMesh->RenderData = MakeUnique<FStaticMeshRenderData>();

	FStaticMeshRenderData* RenderData = StaticMesh->RenderData.Get();


	// 3.Allocate LOD Resources
	RenderData->AllocateLODResources(1);

	// 4. Other Settings
	bool bReverseTangents = false;
	const float TangentsDirection = bReverseTangents ? 1 : -1;

	// 5.Fill Data , only has one LOD data
	// Get StaticMeshLODResources
	FStaticMeshLODResources& LODResources = RenderData->LODResources[0];
	LODResources.bBuffersInlined = true;

	// Get StaticMeshSectionArray
	FStaticMeshLODResources::FStaticMeshSectionArray& Sections = LODResources.Sections;

	// 6.Create Indices Array
	TArray<uint32> LODIndices;
	int32 NumUVs = 1;

	// 7.Get Mesh Data Index Num
	int32 IndexNum = MeshData::Index.Num();

	// 8.Create Vertex Buffer Array . has N Index has N Vertex
	TArray<FStaticMeshBuildVertex> StaticMeshBuildVertices;
	StaticMeshBuildVertices.SetNum(IndexNum);

	FBox BoundingBox;
	BoundingBox.Init();

	/** --------------------------------------------------------------------- */
	// 9.Create StaticMaterial
	StaticMesh->AddMaterial(Mat);

	// 10.Add StaticMesh Section
	FStaticMeshSection& Section = Sections.AddDefaulted_GetRef();

	// 11.Fill Section Data
	Section.NumTriangles = IndexNum / 3;
	Section.FirstIndex = 0;
	Section.bEnableCollision = true;
	Section.bCastShadow = false;
	Section.MaterialIndex = 0;
	Section.MinVertexIndex = 0;
	Section.MaxVertexIndex = IndexNum - 1;
	

	LODIndices.AddUninitialized(IndexNum);
	LODResources.bHasColorVertexData = true;

	// 12.Geometry generation
	for (int32 i = 0; i < IndexNum; i++)
	{
		// fill index buffer
		uint32 VertexIndex = MeshData::Index[i];
		LODIndices[i] = i;
		FStaticMeshBuildVertex& StaticMeshVertex = StaticMeshBuildVertices[i];
		// fill vertex buffer
		StaticMeshVertex.Position = MeshData::Vertex[VertexIndex];
		// calc bounding box
		BoundingBox += StaticMeshVertex.Position;
		// calc Tangent
		FVector4 TangentX = FVector4(0, 0, 0, 1);

		StaticMeshVertex.TangentX = TangentX;
		StaticMeshVertex.TangentZ = FVector::ZeroVector;
		StaticMeshVertex.TangentY = (StaticMeshVertex.TangentZ ^ StaticMeshVertex.TangentX) * TangentX.W *
			TangentsDirection;

		// fill uv

		// fill vertex_color buffer
		StaticMeshVertex.Color = FColor::White;

		// calc normals
		if ((IndexNum % 3) == 0)
		{
			for (int32 m = 0; m < IndexNum;
			     m += 3)
			{
				FStaticMeshBuildVertex& StaticMeshVertex0 = StaticMeshBuildVertices[m];
				FStaticMeshBuildVertex& StaticMeshVertex1 = StaticMeshBuildVertices[m + 1];
				FStaticMeshBuildVertex& StaticMeshVertex2 = StaticMeshBuildVertices[m + 2];

				FVector SideA = StaticMeshVertex1.Position - StaticMeshVertex0.Position;
				FVector SideB = StaticMeshVertex2.Position - StaticMeshVertex0.Position;
				FVector NormalFromCross = FVector::CrossProduct(SideB, SideA).GetSafeNormal();

				StaticMeshVertex0.TangentZ = NormalFromCross;
				StaticMeshVertex1.TangentZ = NormalFromCross;
				StaticMeshVertex2.TangentZ = NormalFromCross;
			}
		}
	}
	/** --------------------------------------------------------------------- */
	if (StaticMesh->bAllowCPUAccess)
	{
		LODResources.IndexBuffer = FRawStaticIndexBuffer(true);
	}
	LODResources.IndexBuffer.SetIndices(LODIndices, EIndexBufferStride::AutoDetect);

	
	LODResources.VertexBuffers.PositionVertexBuffer.Init(StaticMeshBuildVertices, StaticMesh->bAllowCPUAccess);
	LODResources.VertexBuffers.StaticMeshVertexBuffer.Init(StaticMeshBuildVertices, NumUVs, StaticMesh->bAllowCPUAccess);
	LODResources.VertexBuffers.ColorVertexBuffer.Init(StaticMeshBuildVertices, StaticMesh->bAllowCPUAccess);

	


	// 13.Init Resources
	StaticMesh->InitResources();

	RenderData->Bounds = BoundingBox;
	StaticMesh->CalculateExtendedBounds();

	StaticMesh->RenderData->ScreenSize[0].Default = 1.f;

	if (!StaticMesh->BodySetup)
	{
		StaticMesh->CreateBodySetup();
	}

	if (!StaticMesh->bAllowCPUAccess)
	{
		StaticMesh->BodySetup->bNeverNeedsCookedCollisionData = true;
	}

	StaticMesh->BodySetup->bMeshCollideAll = false;
	StaticMesh->BodySetup->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseSimpleAndComplex;

	// create simple collision
	StaticMesh->BodySetup->InvalidatePhysicsData();
	
	FKBoxElem BoxElem;
	BoxElem.Center = RenderData->Bounds.Origin;
	BoxElem.X = RenderData->Bounds.BoxExtent.X * 2.0f;
	BoxElem.Y = RenderData->Bounds.BoxExtent.Y * 2.0f;
	BoxElem.Z = RenderData->Bounds.BoxExtent.Z * 2.0f;
	StaticMesh->BodySetup->AggGeom.BoxElems.Add(BoxElem);
	
	StaticMesh->BodySetup->CreatePhysicsMeshes();
	

	StaticMeshComponent->SetStaticMesh(StaticMesh);
}
