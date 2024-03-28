// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SAIController.generated.h"

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API ASAIController : public AAIController
{
	GENERATED_BODY()

public:
    ASAIController();

    void BeginAI(APawn* InPawn);
    void EndAI();
protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    //FTimerHandle PatrolTimerHandle = FTimerHandle();
    //static const float PatrolRepeatInterval;

    static const float PatrolRadius;

    static const FName StartPatrolPositionKey;
    static const FName EndPatrolPositionKey;
    static const FName TargetActorKey;

private:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ASAIController, meta = (AllowPrivateAccess))
    TObjectPtr<class UBlackboardData> BlackboardDataAsset;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ASAIController, meta = (AllowPrivateAccess))
    TObjectPtr<class UBehaviorTree> BehaviorTree;
};
