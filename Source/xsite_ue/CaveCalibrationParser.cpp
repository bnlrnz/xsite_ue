// Fill out your copyright notice in the Description page of Project Settings.

#include "CaveCalibrationParser.h"
#include "xsite_ue.h"
#include "JsonObjectConverter.h"

bool CaveCalibrationParser::ParseJsonString(const FString& JsonString, FCalibrationData *CalibrationData)
{
    if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, CalibrationData, 0, 0))
    {
        UE_LOG(LogCave, Error, TEXT("Could not parse calibration file (json)"));
        return false;
    }
    return true;
}
