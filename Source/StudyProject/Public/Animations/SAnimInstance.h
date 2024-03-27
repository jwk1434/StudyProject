// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SAnimInstance.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheckHitDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheckCanNextComboDelegate);

UCLASS()
class STUDYPROJECT_API USAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

    friend class ASRPGCharacter;
public:
    USAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;        //Tick ÇÔ¼ö

protected:
    FOnCheckHitDelegate OnCheckHitDelegate;
    FOnCheckCanNextComboDelegate OnCheckCanNextComboDelegate;

private:
    void PlayAttackAnimMontage();

    UFUNCTION()
    void AnimNotify_CheckHit();

    UFUNCTION()
    void AnimNotify_CheckCanNextCombo();
protected:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USAnimInstance", Meta = (AllowPrivateAccess))
    float CurrentSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USAnimInstance")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USAnimInstance")
    uint8 bIsFalling : 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USAnimInstance")
    uint8 bIsCrouching : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USAnimInstance", Meta = (AllowPrivateAccess))
    uint8 bIsDead : 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USAnimInstance", Meta = (AllowPrivateAccess))
    TObjectPtr<class UAnimMontage> AttackAnimMontage;
};
