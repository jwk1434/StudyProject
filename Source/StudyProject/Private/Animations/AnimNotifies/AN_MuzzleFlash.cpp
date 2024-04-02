// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/AnimNotifies/AN_MuzzleFlash.h"
#include "Characters/STPSCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

void UAN_MuzzleFlash::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (true == ::IsValid(MeshComp))
    {
        ASTPSCharacter* AttackingCharacter = Cast<ASTPSCharacter>(MeshComp->GetOwner());
        if (true == ::IsValid(AttackingCharacter))
        {
            FVector MuzzleLocation = AttackingCharacter->GetWeaponComponent()->GetSocketLocation(FName("MuzzleSocket"));
            UParticleSystemComponent* Effect = UGameplayStatics::SpawnEmitterAtLocation(AttackingCharacter->GetWorld(), ParticleSystem, MuzzleLocation);

            UCameraComponent* CameraComponent = AttackingCharacter->GetCameraComponent();

            if (true == ::IsValid(CameraComponent) && true == ::IsValid(Effect))
            {
                FVector CameraForwardVector = CameraComponent->GetForwardVector();
                Effect->SetWorldRotation(FRotationMatrix::MakeFromX(CameraForwardVector).Rotator());
            }
        } 

    }
}
