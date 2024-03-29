// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controllers/SUIPlayerController.h"
#include "SLoadingPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API ASLoadingPlayerController : public ASUIPlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
};
