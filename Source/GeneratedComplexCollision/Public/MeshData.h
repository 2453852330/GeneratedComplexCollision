#pragma once

namespace MeshData
{
	const TArray<FVector> Vertex = {
		FVector(-50.f,-50.f,0.f),FVector(50.f,-50.f,0.f),
		FVector(0.f,50.f,0.f),FVector(0.f,0.f,100.f)
	};
	const TArray<uint32> Index = {
		0,1,2, 
		0,3,1,
		2,3,0,
		1,3,2
	};
	const TArray<FLinearColor> Color = {
		FLinearColor::White,FLinearColor::White,FLinearColor::White,
		FLinearColor::Blue,FLinearColor::Blue,FLinearColor::Blue,
		FLinearColor::Green,FLinearColor::Green,FLinearColor::Green,
		FLinearColor::Red,FLinearColor::Red,FLinearColor::Red
	};
}
