// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CaveCalibrationParser.generated.h"

USTRUCT()
struct FSpaceData{
	GENERATED_BODY()

	UPROPERTY()
	TArray<double> bottom_left;

	UPROPERTY()
	TArray<double> bottom_right;

	UPROPERTY()
	TArray<double> top_right;

	UPROPERTY()
	TArray<double> top_left;
};

USTRUCT()
struct FViewVolumeData{
	GENERATED_BODY()

	UPROPERTY()
	TArray<double> pa;

	UPROPERTY()
	TArray<double> pb;

	UPROPERTY()
	TArray<double> pc;
};

USTRUCT()
struct FCornerData{
	GENERATED_BODY()

	//UPROPERTY()
	//FSpaceData projector_space;

	UPROPERTY()
	FSpaceData camera_space;

	// Display Space Corners -> relative corners of a screen in realtion to its wall
	//	TopLeft, TopRight, BottomRight, BottomLeft
	//
	// example: (0,0), (0.5,0), (0.5,0.3), (0,0.3)
	//		
	//		x-------x-------|
	//		|Screen |		|
	//		x-------x-------|
	//		|		|		|
	//		|---------------|
	//		|		|		|
	//		|---------------|
	//			   Wall
	UPROPERTY()
	FSpaceData display_space;
};

USTRUCT()
struct FProjectorData{
	GENERATED_BODY()

	UPROPERTY()
	FString name;

	UPROPERTY()
	int id;

	UPROPERTY()
	int display;

	UPROPERTY()
	int screen;

	UPROPERTY()
	TArray<double> resolution;

	// position at the screen, gpu drivers can extend there screen over multiple displays
	// example:
	//
	//	  (0,0)                     (1920,0)
	//		X-----------------------| X---------------------|
	//		|						| |						|
	//		|		1920x1080		| |		1920x1080		|
	//		|						| |						|
	//		|						| |						|
	//		|-----------------------| |---------------------|
	//
	//		Hole screen resolution: 3840x1080
	UPROPERTY()
	TArray<double> offset;

	UPROPERTY()
	FString alphamask;

	UPROPERTY()
	FCornerData corners;
	
	UPROPERTY()
	FViewVolumeData view_volume;

	// H -> maps Projector coordinates P(xi, yi) to Camera C(u,v)
	// these are the 20 coefficients for 2 cubic equation (first 10 for x -> u, last 10 for y -> v)
	UPROPERTY()
	TArray<double> H_x;

	UPROPERTY()
	TArray<double> H_y;

	// Hi -> inverse of H 
	// these are the 20 coefficients for 2 cubic equation (first 10 for u -> x, last 10 for v -> y)
	UPROPERTY()
	TArray<double> Hi_x;

	UPROPERTY()
	TArray<double> Hi_y;
};

USTRUCT()
struct FClientData{
	GENERATED_BODY()

	UPROPERTY()
	FString name;

	UPROPERTY()
	FString ip;

	UPROPERTY()
	int id;
	
	UPROPERTY()
	TArray<FProjectorData> projectors;
};

USTRUCT()
struct FCameraData{
	GENERATED_BODY()

	// F -> maps Camera Coordinates C(u,v) to Projection Plane D(s', t')
	// should be equal for all screens of the same wall
	UPROPERTY()
	TArray<double> F;

	// Fi -> inverse of F
	// should be equal for all screens of the same wall
	UPROPERTY()
	TArray<double> Fi;
};

USTRUCT()
struct FWallBoundData{
	GENERATED_BODY()

	UPROPERTY()
	TArray<double> top_left;

	UPROPERTY()
	TArray<double> bottom_right;
};

USTRUCT()
struct FWallData{
	GENERATED_BODY()

	UPROPERTY()
	FString name;

	// Width, Height
	UPROPERTY()
	TArray<double> size;

	UPROPERTY()
	TArray<double> normal;

	UPROPERTY()
	FWallBoundData bounds;

	UPROPERTY()
	FCameraData camera;

	UPROPERTY()
	TArray<FClientData> clients;
};

USTRUCT()
struct FCalibrationData{
	GENERATED_BODY()

	UPROPERTY()
	FString system;

	UPROPERTY()
	FString date;

	UPROPERTY()
	TArray<double> eye;

	UPROPERTY()
	TArray<FWallData> walls;
};

class CaveCalibrationParser
{
public:
	static bool ParseJsonString(FString JsonString, FCalibrationData* CalibrationData);
private:
	CaveCalibrationParser(){};
	~CaveCalibrationParser(){};
};
