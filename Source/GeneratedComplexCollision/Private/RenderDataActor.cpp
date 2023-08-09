// Fill out your copyright notice in the Description page of Project Settings.


#include "RenderDataActor.h"
#include "MeshData.h"
// Sets default values
ARenderDataActor::ARenderDataActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
}

// Called when the game starts or when spawned
void ARenderDataActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 创建一个 UStaticMesh
	UStaticMesh* SM = NewObject<UStaticMesh>(StaticMeshComponent);
	SM->bAllowCPUAccess = true;
	// pStaticMesh->NeverStream = true;
	// 创建 FStaticMeshRenderData
	FStaticMeshRenderData* RenderData = new FStaticMeshRenderData();
	// 初始化LOD信息
	RenderData->AllocateLODResources(1);
	// 获取 之前创建 RenderData 的LOD(上一步申请的)
	FStaticMeshLODResources& LODResources = RenderData->LODResources[0];
	LODResources.bBuffersInlined = true;
	
	// 构造索引数组
	TArray<uint32> tmp_index;
	tmp_index.AddZeroed(MeshData::Index.Num());
	// 构造顶点数据数组
	TArray<FStaticMeshBuildVertex> vertex_list;
	vertex_list.SetNum(MeshData::Index.Num());
	
	// 构造BoundingBox
	FBox BoundBox;
	BoundBox.Init();
	
	for (int32 i = 0; i < MeshData::Index.Num(); ++i)
	{
		tmp_index[i] = i;
		
		FStaticMeshBuildVertex& vertex_ref = vertex_list[i];
		vertex_ref.Position = MeshData::Vertex[MeshData::Index[i]];
		vertex_ref.Color = FColor::White;
		vertex_ref.UVs[0] = FVector2D(0, 0);
		vertex_ref.TangentX = FVector(0, 1, 0);
		vertex_ref.TangentY = FVector(1, 0, 0);
		vertex_ref.TangentY = FVector(0, 0, 1);

		BoundBox += vertex_ref.Position;
	}
	// 应用索引数组
	// 必须使用 FRawStaticIndexBuffer ,不然打包后没有复杂碰撞
	if (SM->bAllowCPUAccess)
	{
		LODResources.IndexBuffer = FRawStaticIndexBuffer(true);
	}
	LODResources.IndexBuffer.SetIndices(tmp_index, EIndexBufferStride::Type::Force16Bit);

	
	LODResources.bHasDepthOnlyIndices = false;
	LODResources.bHasReversedIndices = false;
	LODResources.bHasReversedDepthOnlyIndices = false;
	//LODResources.bHasAdjacencyInfo = false;

	
	// 顶点颜色
	LODResources.bHasColorVertexData = true;


	
	// 应用BoundingBox
	RenderData->Bounds = BoundBox;

	// 应用顶点数组
	LODResources.VertexBuffers.PositionVertexBuffer.Init(vertex_list);

	// 应用顶点颜色
	FColorVertexBuffer& ColorVertexBuffer = LODResources.VertexBuffers.ColorVertexBuffer;
	ColorVertexBuffer.Init(vertex_list);

	//法线，切线，贴图坐标buffer
	LODResources.VertexBuffers.StaticMeshVertexBuffer.Init(vertex_list, 1);

	// 获取 LODResources 的 Sections 数组
	FStaticMeshLODResources::FStaticMeshSectionArray& Sections = LODResources.Sections;

	// 构造 FStaticMeshSection
	FStaticMeshSection& section = Sections.AddDefaulted_GetRef();

	// 计算三角面数 = 顶点数 / 3 ;
	section.NumTriangles = vertex_list.Num() / 3;

	// 设置索引范围 0 - (3 - 1) -> 0-2
	section.FirstIndex = 0;
	section.MinVertexIndex = 0;
	section.MaxVertexIndex = vertex_list.Num() - 1;

	section.bEnableCollision = true;
	//section.bCastShadow = true;

	// 将创建的RenderData传递给UStaticMesh
	SM->RenderData = TUniquePtr<FStaticMeshRenderData>(RenderData);

	// 获取材质
	UMaterial* Material = UMaterial::GetDefaultMaterial(MD_Surface);
	// Material->TwoSided = true;
	// Material->Wireframe = true;

	// 给FStaticMeshSection添加材质
	section.MaterialIndex = 0;
	SM->AddMaterial(Material);

	// 调用UStaticMesh的修改数据接口
	SM->InitResources();
	// 重新计算Bounds
	SM->CalculateExtendedBounds();

	SM->RenderData->ScreenSize[0].Default = 1.0f;

	SM->CreateBodySetup();


	// create simple collision
	// Box 或者 Sphere 只能有一种,后面的会覆盖前面的
	SM->BodySetup->InvalidatePhysicsData();
	FKBoxElem BoxElem;
	BoxElem.Center = SM->RenderData->Bounds.Origin;
	BoxElem.X = SM->RenderData->Bounds.BoxExtent.X * 2.f;
	BoxElem.Y = SM->RenderData->Bounds.BoxExtent.Y * 2.f;
	BoxElem.Z = SM->RenderData->Bounds.BoxExtent.Z * 2.f;
	BoxElem.Rotation = FRotator::ZeroRotator;
	SM->BodySetup->AggGeom.BoxElems.Add(BoxElem);
	
	// FKSphereElem SphereElem;
	// SphereElem.Center = BoundingBoxAndSphere.Origin;
	// SphereElem.Radius = 50.f;
	// SM->BodySetup->AggGeom.SphereElems.Add(SphereElem);
	
	SM->BodySetup->CreatePhysicsMeshes();
	
	StaticMeshComponent->SetStaticMesh(SM);
}

