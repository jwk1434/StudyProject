// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_MuzzleFlash.generated.h"

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API UAN_MuzzleFlash : public UAnimNotify
{
	GENERATED_BODY()

private:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MuzzleFlash", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystem> ParticleSystem;
};
