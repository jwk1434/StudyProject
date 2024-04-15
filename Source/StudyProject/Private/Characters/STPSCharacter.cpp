#include "Characters/STPSCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "Controllers/SPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Inputs/SInputConfigData.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraShakeBase.h"
#include "WorldStatic/SLandMine.h"
#include "Components/SStatComponent.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ASTPSCharacter::ASTPSCharacter()
    : ASCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->SetCollisionProfileName(TEXT("SCharacter"));

    CameraComponent->SetRelativeLocation(FVector(0.f, 60.f, 20.f)); // TPS 방식의 슈팅 게임 특징.

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    SpringArmComponent->bUsePawnControlRotation = true;
    SpringArmComponent->bDoCollisionTest = true;
    SpringArmComponent->bInheritPitch = true;
    SpringArmComponent->bInheritYaw = true;
    SpringArmComponent->bInheritRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 480.f, 0.f);

    WeaponSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSkeletalMeshComponent"));

    TimeBetweenFire = 60.f / FirePerMinute;
}

void ASTPSCharacter::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (true == ::IsValid(PlayerController))
    {
        UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
        if (true == ::IsValid(Subsystem))
        {
            Subsystem->AddMappingContext(PlayerCharacterInputMappingContext, 0);
        }
    }

    FName EquipmentSocketName = FName(TEXT("EquipmentSocket"));
    if (true == GetMesh()->DoesSocketExist(EquipmentSocketName))
    {
        WeaponSkeletalMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, EquipmentSocketName);
    }
}

void ASTPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, ForwardInputValue);
    DOREPLIFETIME(ThisClass, RightInputValue);
    DOREPLIFETIME(ThisClass, CurrentAimPitch);
    DOREPLIFETIME(ThisClass, CurrentAimYaw);
}

void ASTPSCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaSeconds, 35.f);
    CameraComponent->SetFieldOfView(CurrentFOV);

    if (true == ::IsValid(GetController()))
    {
        FRotator ControlRotation = GetController()->GetControlRotation();
        CurrentAimPitch = ControlRotation.Pitch;
        CurrentAimYaw = ControlRotation.Yaw;
    }

    if (true == bIsNowRagdollBlending)
    {
        CurrentRagDollBlendWeight = FMath::FInterpTo(CurrentRagDollBlendWeight, TargetRagDollBlendWeight, DeltaSeconds, 10.f);

        FName PivotBoneName = FName(TEXT("spine_01"));
        GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(PivotBoneName, CurrentRagDollBlendWeight);

        if (CurrentRagDollBlendWeight - TargetRagDollBlendWeight < KINDA_SMALL_NUMBER)
        {
            GetMesh()->SetAllBodiesBelowSimulatePhysics(PivotBoneName, false);
            bIsNowRagdollBlending = false;
        }

        if (true == ::IsValid(GetStatComponent()) && GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
        {
            GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(FName(TEXT("root")), 1.f);
            // 모든 본에 렉돌 가중치
            GetMesh()->SetSimulatePhysics(true);
            bIsNowRagdollBlending = false;
        }
    }
}

float ASTPSCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

    //if (false == ::IsValid(GetStatComponent()))
    //{
    //    return ActualDamage;
    //}

    //if (GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
    //{
    //    GetMesh()->SetSimulatePhysics(true);
    //}
    //else
    //{
    //    FName PivotBoneName = FName(TEXT("spine_01"));
    //    GetMesh()->SetAllBodiesBelowSimulatePhysics(PivotBoneName, true);
    //    //float BlendWeight = 1.f; // 랙돌 포즈에 완전 치우쳐지게끔 가중치를 1.f로 지정.
    //    //GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(PivotBoneName, BlendWeight);
    //    TargetRagDollBlendWeight = 1.f;

    //    HittedRagdollRestoreTimerDelegate.BindUObject(this, &ThisClass::OnHittedRagdollRestoreTimerElapsed);
    //    GetWorld()->GetTimerManager().SetTimer(HittedRagdollRestoreTimer, HittedRagdollRestoreTimerDelegate, 1.f, false);
    //}


    PlayRagdoll_NetMulticast();

    return ActualDamage;
}

void ASTPSCharacter::PlayRagdoll_NetMulticast_Implementation()
{
    if (false == ::IsValid(GetStatComponent()))
    {
        return;
    }

    if (GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
    {
        GetMesh()->SetSimulatePhysics(true);
    }
    else
    {
        FName PivotBoneName = FName(TEXT("spine_01"));
        GetMesh()->SetAllBodiesBelowSimulatePhysics(PivotBoneName, true);
        TargetRagDollBlendWeight = 1.f;
        HittedRagdollRestoreTimerDelegate.BindUObject(this, &ThisClass::OnHittedRagdollRestoreTimerElapsed);
        GetWorld()->GetTimerManager().SetTimer(HittedRagdollRestoreTimer, HittedRagdollRestoreTimerDelegate, 1.f, false);
    }
}

void ASTPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (true == ::IsValid(EnhancedInputComponent))
    {
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->AttackAction, ETriggerEvent::Started, this, &ThisClass::Attack);

        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->IronSightAction, ETriggerEvent::Started, this, &ThisClass::StartIronSight);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->IronSightAction, ETriggerEvent::Completed, this, &ThisClass::EndIronSight);

        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->TriggerAction, ETriggerEvent::Started, this, &ThisClass::ToggleTrigger);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->AttackAction, ETriggerEvent::Started, this, &ThisClass::StartFire);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->AttackAction, ETriggerEvent::Completed, this, &ThisClass::StopFire);

        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->LandMineAction, ETriggerEvent::Started, this, &ThisClass::SpawnLandMine);
    }
}

void ASTPSCharacter::Move(const FInputActionValue& InValue)
{
    FVector2D MovementVector = InValue.Get<FVector2D>();
    ForwardInputValue = MovementVector.X;
    RightInputValue = MovementVector.Y;

    if (ForwardInputValue > 0.f)
        GetCharacterMovement()->MaxWalkSpeed = 500.f;
    else
        GetCharacterMovement()->MaxWalkSpeed = 200.f;

    const FRotator CurrentControlRotation = GetController()->GetControlRotation();
    const FRotator CurrentControlRotationYaw(0.f, CurrentControlRotation.Yaw, 0.f);

    FVector ForwardDirection = FRotationMatrix(CurrentControlRotationYaw).GetUnitAxis(EAxis::X);
    FVector RightDirection = FRotationMatrix(CurrentControlRotationYaw).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDirection, MovementVector.X);
    AddMovementInput(RightDirection, MovementVector.Y);
}

void ASTPSCharacter::Look(const FInputActionValue& InValue)
{
    FVector2D LookAxisVector = InValue.Get<FVector2D>();

    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
}

void ASTPSCharacter::Attack(const FInputActionValue& InValue)
{
    if (false == bIsTriggerToggle)
    {
        Fire();
    }
}

void ASTPSCharacter::Fire()
{
    if (true == HasAuthority() || GetOwner() != UGameplayStatics::GetPlayerController(this, 0))
    {
        return;
    }

    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Fire() has been called.")));

    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (false == ::IsValid(PlayerController))
    {
        return;
    }

    FHitResult HitResult;

    FVector CameraStartLocation = CameraComponent->GetComponentLocation();
    FVector CameraEndLocation = CameraStartLocation + CameraComponent->GetForwardVector() * 5000.f;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredComponent((const UPrimitiveComponent*)(CameraComponent));
    QueryParams.bTraceComplex = true;

    FVector MuzzleLocation = WeaponSkeletalMeshComponent->GetSocketLocation(FName("MuzzleSocket"));
    bool bIsCollide = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleLocation, CameraEndLocation, ECC_GameTraceChannel2, QueryParams);

    if (true == bIsCollide)
    {
        //DrawDebugLine(GetWorld(), MuzzleLocation, HitResult.Location, FColor(255, 255, 255, 64), true, 0.1f, 0U, 0.5f);

        ASCharacter* HittedCharacter = Cast<ASCharacter>(HitResult.GetActor());
        if (true == ::IsValid(HittedCharacter))
        {
            FDamageEvent DamageEvent;
            //HittedCharacter->TakeDamage(10.f, DamageEvent, GetController(), this);

            FString BoneNameString = HitResult.BoneName.ToString();
            UKismetSystemLibrary::PrintString(this, BoneNameString);
            DrawDebugSphere(GetWorld(), HitResult.Location, 3.f, 16, FColor(255, 0, 0, 255), true, 20.f, 0U, 5.f);

            if (true == BoneNameString.Equals(FString(TEXT("HEAD")), ESearchCase::IgnoreCase))
            {
                //HittedCharacter->TakeDamage(100.f, DamageEvent, GetController(), this);
                ApplyDamageAndDrawLine_Server(MuzzleLocation, HitResult.Location, HittedCharacter, 100.f, DamageEvent, GetController(), this);
            }
            else
            {
                //HittedCharacter->TakeDamage(10.f, DamageEvent, GetController(), this);
                ApplyDamageAndDrawLine_Server(MuzzleLocation, HitResult.Location, HittedCharacter, 10.f, DamageEvent, GetController(), this);
            }
        }
    }
    else
    {
        //DrawDebugLine(GetWorld(), MuzzleLocation, CameraEndLocation, FColor(255, 255, 255, 64), false, 0.1f, 0U, 0.5f);
        FDamageEvent DamageEvent;
        ApplyDamageAndDrawLine_Server(MuzzleLocation, CameraEndLocation, nullptr, 0.f, DamageEvent, GetController(), this);
    }

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (false == ::IsValid(AnimInstance))
    {
        return;
    }

    if (false == AnimInstance->Montage_IsPlaying(RifleFireAnimMontage))
    {
        AnimInstance->Montage_Play(RifleFireAnimMontage);
        PlayAttackMontage_Server();
    }

    if (true == ::IsValid(FireShake))
    {
        if (GetOwner() == UGameplayStatics::GetPlayerController(this, 0))
        { // 다른 클라가 사격했는데, 내 PC 화면이 흔들리지 않게끔 함.
            PlayerController->ClientStartCameraShake(FireShake);
        }
    }
}

void ASTPSCharacter::ToggleTrigger(const FInputActionValue& InValue)
{
    bIsTriggerToggle = !bIsTriggerToggle;
}

void ASTPSCharacter::StartFire(const FInputActionValue& InValue)
{
    if (true == bIsTriggerToggle)
    {
        GetWorldTimerManager().SetTimer(BetweenShotsTimer, this, &ThisClass::Fire, TimeBetweenFire, true);
    }
}

void ASTPSCharacter::StopFire(const FInputActionValue& InValue)
{
    GetWorldTimerManager().ClearTimer(BetweenShotsTimer);
}

void ASTPSCharacter::StartIronSight(const FInputActionValue& InValue)
{
    TargetFOV = 45.f;
}

void ASTPSCharacter::EndIronSight(const FInputActionValue& InValue)
{
    TargetFOV = 70.f;
}

void ASTPSCharacter::OnHittedRagdollRestoreTimerElapsed()
{
    FName PivotBoneName = FName(TEXT("spine_01"));
    //GetMesh()->SetAllBodiesBelowSimulatePhysics(PivotBoneName, false);
    //float BlendWeight = 0.f;
    //GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(PivotBoneName, BlendWeight);
    TargetRagDollBlendWeight = 0.f;
    CurrentRagDollBlendWeight = 1.f;        //0 : Anim, 1 : Ragdoll
    bIsNowRagdollBlending = true;
}

void ASTPSCharacter::SpawnLandMine(const FInputActionValue& InValue)
{
    /*if (true == ::IsValid(LandMineClass))
    {
        FVector SpawnedLocation = (GetActorLocation() + GetActorForwardVector() * 300.f) - FVector(0.f, 0.f, 90.f);
        ASLandMine* SpawnedLandMine = GetWorld()->SpawnActor<ASLandMine>(LandMineClass, SpawnedLocation, FRotator::ZeroRotator);
        SpawnedLandMine->SetOwner(GetController());
    }*/

    SpawnLandMine_Server();
}

bool ASTPSCharacter::SpawnLandMine_Server_Validate()
{
    return true;
}

void ASTPSCharacter::SpawnLandMine_Server_Implementation()
{
    if (true == ::IsValid(LandMineClass))
    {
        FVector SpawnedLocation = (GetActorLocation() + GetActorForwardVector() * 200.f) - FVector(0.f, 0.f, 90.f);
        ASLandMine* SpawnedLandMine = GetWorld()->SpawnActor<ASLandMine>(LandMineClass, SpawnedLocation, FRotator::ZeroRotator);
        SpawnedLandMine->SetOwner(GetController());
    }
}

void ASTPSCharacter::UpdateInputValue_Server_Implementation(const float& InForwardInputValue, const float& InRightInputValue)
{
    ForwardInputValue = InForwardInputValue;
    RightInputValue = InRightInputValue;
}

void ASTPSCharacter::UpdateAimValue_Server_Implementation(const float& InAimPitchValue, const float& InAimYawValue)
{
    CurrentAimPitch = InAimPitchValue;
    CurrentAimYaw = InAimYawValue;
}

void ASTPSCharacter::PlayAttackMontage_Server_Implementation()
{
    PlayAttackMontage_NetMulticast();
}

void ASTPSCharacter::PlayAttackMontage_NetMulticast_Implementation()
{
    if (false == HasAuthority() && GetOwner() != UGameplayStatics::GetPlayerController(this, 0))
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (false == ::IsValid(AnimInstance))
        {
            return;
        }

        if (false == AnimInstance->Montage_IsPlaying(RifleFireAnimMontage))
        {
            AnimInstance->Montage_Play(RifleFireAnimMontage);
        }
    }
}

void ASTPSCharacter::ApplyDamageAndDrawLine_Server_Implementation(const FVector& InDrawStart, const FVector& InDrawEnd, ACharacter* InHittedCharacter, float InDamage, FDamageEvent const& InDamageEvent, AController* InEventInstigator, AActor* InDamageCauser)
{
    if (true == ::IsValid(InHittedCharacter))
    {
        InHittedCharacter->TakeDamage(InDamage, InDamageEvent, InEventInstigator, InDamageCauser);
    }

    DrawLine_NetMulticast(InDrawStart, InDrawEnd);
}

void ASTPSCharacter::DrawLine_NetMulticast_Implementation(const FVector& InDrawStart, const FVector& InDrawEnd)
{
    DrawDebugLine(GetWorld(), InDrawStart, InDrawEnd, FColor(255, 255, 255, 64), false, 0.1f, 0U, 0.5f);
}