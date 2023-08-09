// Fill out your copyright notice in the Description page of Project Settings.


#include "CreatedStaticMeshActor.h"


#include "MeshData.h"
#include "MeshDescription.h"
#include "StaticMeshDescription.h"
#include "StaticMeshOperations.h"
// Sets default values
ACreatedStaticMeshActor::ACreatedStaticMeshActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
}

// Called when the game starts or when spawned
void ACreatedStaticMeshActor::BeginPlay()
{
	Super::BeginPlay();


	// 创建SMD,并在 FStaticMeshAttributes 中对 MeshDescription中存储的数据进行 Register()
	UStaticMeshDescription* SMD = UStaticMesh::CreateStaticMeshDescription();
	
	// 保存创建的Vertex返回的vertex_id
	TArray<FVertexID> vertex_id;
	// 可以先定义内存,下面直接赋值使用;
	vertex_id.AddZeroed(MeshData::Vertex.Num());


	// ReserveNewVertices 提前分配内存,可以省略
	SMD->GetMeshDescription().ReserveNewVertices(MeshData::Vertex.Num());
	for (int32 i = 0; i < MeshData::Vertex.Num(); ++i)
	{
		// 创建Vertex并保存返回的vertex_id
		vertex_id[i] = SMD->GetMeshDescription().CreateVertex();
	}

	// 获取到 Vertex 数据的引用
	TVertexAttributesRef<FVector> vertex = SMD->GetVertexPositions();
	// 获取到 Color 数据的引用
	TVertexInstanceAttributesRef<FVector4> color = SMD->GetVertexInstanceColors();
	// 对Vertex数据进行填充
	for (int32 i = 0; i < MeshData::Vertex.Num(); ++i)
	{
		// 使用vertex_id进行索引,使用自定义数据进行填充
		vertex[vertex_id[i]] = MeshData::Vertex[i];
	}
	// 创建一个三角面组
	FPolygonGroupID PolygonGroupID = SMD->CreatePolygonGroup();

	auto make_polygon = [this,&vertex_id,&SMD,PolygonGroupID](
		int32 p0, int32 p1, int32 p2)-> FPolygonID
	{
		// 缓存创建的 vertex_instance_id
		FVertexInstanceID vertex_instance_id[3];
		// 使用vertex_id 创建 vertex_instance_id
		vertex_instance_id[0] = SMD->GetMeshDescription().CreateVertexInstance(vertex_id[p0]);
		vertex_instance_id[1] = SMD->GetMeshDescription().CreateVertexInstance(vertex_id[p1]);
		vertex_instance_id[2] = SMD->GetMeshDescription().CreateVertexInstance(vertex_id[p2]);
		
		TArray<FEdgeID> edge_ids;
		edge_ids.Reserve(3);

		// 创建一个 三角面
		FPolygonID polygon_id = SMD->GetMeshDescription().CreatePolygon(PolygonGroupID, vertex_instance_id, &edge_ids);
		for (FEdgeID& it : edge_ids)
		{
			SMD->GetEdgeHardnesses()[it] = true;
		}
		return polygon_id;
	};

	TArray<FPolygonID> polygon_list;

	
	polygon_list.Add(make_polygon(MeshData::Index[0], MeshData::Index[1], MeshData::Index[2]));
	polygon_list.Add(make_polygon(MeshData::Index[3], MeshData::Index[4], MeshData::Index[5]));
	polygon_list.Add(make_polygon(MeshData::Index[6], MeshData::Index[7], MeshData::Index[8]));
	polygon_list.Add(make_polygon(MeshData::Index[9], MeshData::Index[10], MeshData::Index[11]));
	

	// 注册 PolygonAttributes 的属性,只有先注册,才能进行填充
	// 上面的Vertex和Color属性已经被SMD在初始化的时候进行Register()
	SMD->GetMeshDescription().PolygonAttributes().RegisterAttribute<FVector>(
		MeshAttribute::Polygon::Normal, 1, FVector::ZeroVector, EMeshAttributeFlags::Transient);
	SMD->GetMeshDescription().PolygonAttributes().RegisterAttribute<FVector>(
		MeshAttribute::Polygon::Tangent, 1, FVector::ZeroVector, EMeshAttributeFlags::Transient);
	SMD->GetMeshDescription().PolygonAttributes().RegisterAttribute<FVector>(
		MeshAttribute::Polygon::Binormal, 1, FVector::ZeroVector, EMeshAttributeFlags::Transient);
	SMD->GetMeshDescription().PolygonAttributes().RegisterAttribute<FVector>(
		MeshAttribute::Polygon::Center, 1, FVector::ZeroVector, EMeshAttributeFlags::Transient);
	// 通过计算填充上面注册的属性
	FStaticMeshOperations::ComputePolygonTangentsAndNormals(SMD->GetMeshDescription());
	FStaticMeshOperations::ComputeTangentsAndNormals(SMD->GetMeshDescription(),
	                                                 EComputeNTBsFlags::Normals | EComputeNTBsFlags::Tangents);



	
	// 创建 UStaticMesh
	UStaticMesh* SM = NewObject<UStaticMesh>(GetTransientPackage(),NAME_None,RF_Public|RF_Standalone);
	UStaticMesh::FBuildMeshDescriptionsParams Params;
	Params.bBuildSimpleCollision = true;
	Params.bMarkPackageDirty = true;
	Params.bUseHashAsGuid = true;
	Params.bCommitMeshDescription = true;

	SM->bAllowCPUAccess = true;
	SM->InitResources();
	SM->CreateBodySetup();
	
	if (!SM->bAllowCPUAccess)
	{
		SM->BodySetup->bNeverNeedsCookedCollisionData = true;
	}
	SM->BodySetup->bMeshCollideAll = false;
	SM->BodySetup->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseSimpleAndComplex;

	
	// 初始化UV通道
	FStaticMaterial StaticMaterial;
	FMeshUVChannelInfo UVChannelInfo = FMeshUVChannelInfo();
	UVChannelInfo.bInitialized = true;
	StaticMaterial.UVChannelData = UVChannelInfo;
	
	SM->StaticMaterials.Add(StaticMaterial);
	SM->BuildFromMeshDescriptions({&SMD->GetMeshDescription()}, Params);

	StaticMeshComponent->SetStaticMesh(SM);
}

