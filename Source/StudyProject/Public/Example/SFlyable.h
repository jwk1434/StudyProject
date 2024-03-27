// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SFlyable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USFlyable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STUDYPROJECT_API ISFlyable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Fly() = 0; // ISFlyable에서 구현해도 되고 안해도됨. 안하는게 국룰.
};
