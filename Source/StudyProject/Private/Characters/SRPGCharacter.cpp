#include "Characters/SRPGCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Inputs/SInputConfigData.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animations/SAnimInstance.h"
#include "Engine/EngineTypes.h"
#include "Engine/DamageEvents.h"
#include "Particles/ParticleSystemComponent.h"
#include "Game/SPlayerState.h"
#include "SPlayerCharacterSettings.h"
#include "Game/SGameInstance.h"
#include "Engine/StreamableManager.h"

ASRPGCharacter::ASRPGCharacter()
    : bIsAttacking(false)
{
    PrimaryActorTick.bCanEverTick = false;

    CameraComponent->SetRelativeLocation(FVector(0.f, 100.f, 0.f));

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    SpringArmComponent->bUsePawnControlRotation = true;
    SpringArmComponent->bDoCollisionTest = true;
    SpringArmComponent->bInheritPitch = true;
    SpringArmComponent->bInheritYaw = true;
    SpringArmComponent->bInheritRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 480.f, 0.f);

    ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetupAttachment(GetRootComponent());
    ParticleSystemComponent->SetAutoActivate(false);

    const USPlayerCharacterSettings* CDO = GetDefault<USPlayerCharacterSettings>();
    if (0 < CDO->PlayerCharacterMeshPaths.Num())
    {
        for (FSoftObjectPath PlayerCharacterMeshPath : CDO->PlayerCharacterMeshPaths)
        {
            UE_LOG(LogTemp, Warning, TEXT("Path: %s"), *(PlayerCharacterMeshPath.ToString()));
        }
    }
}

void ASRPGCharacter::BeginPlay()
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

    USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
    if (true == ::IsValid(AnimInstance))
    {
        AnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::OnAttackMontageEnded);
        AnimInstance->OnCheckHitDelegate.AddDynamic(this, &ThisClass::CheckHit);
        AnimInstance->OnCheckCanNextComboDelegate.AddDynamic(this, &ThisClass::CheckCanNextCombo);
    }

    ASPlayerState* PS = GetPlayerState<ASPlayerState>();
    if (true == ::IsValid(PS))
    {
        if (false == PS->OnCurrentLevelChangedDelegate.IsAlreadyBound(this, &ThisClass::OnCurrentLevelChanged))
        {
            PS->OnCurrentLevelChangedDelegate.AddDynamic(this, &ThisClass::OnCurrentLevelChanged);
        }
    }

    const USPlayerCharacterSettings* CDO = GetDefault<USPlayerCharacterSettings>();
    int32 RandIndex = FMath::RandRange(0, CDO->PlayerCharacterMeshPaths.Num() - 1);
    CurrentPlayerCharacterMeshPath = CDO->PlayerCharacterMeshPaths[RandIndex];

    USGameInstance* SGI = Cast<USGameInstance>(GetGameInstance());
    if (true == ::IsValid(SGI))
    {
        AssetStreamableHandle = SGI->StreamableManager.RequestAsyncLoad(
            CurrentPlayerCharacterMeshPath,
            FStreamableDelegate::CreateUObject(this, &ThisClass::OnAssetLoaded)
        );

        /*FStreamableDelegate 형식으로 델리게이트 속성을 선언하고 넘겨줄 수 있지만,
        델리게이트에서 제공하는 CreateUObject() 함수를 사용해서 델리게이트를 새로 생성해서
        넘겨주는 방식도 괜찮음.*/
    }

}

void ASRPGCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
    bIsAttacking = false;
}

void ASRPGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (true == ::IsValid(EnhancedInputComponent))
    {
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->AttackAction, ETriggerEvent::Started, this, &ThisClass::Attack);
    }
}

void ASRPGCharacter::Move(const FInputActionValue& InValue)
{
    FVector2D MovementVector = InValue.Get<FVector2D>();
    ForwardInputValue = MovementVector.X;
    RightInputValue = MovementVector.Y;

    const FRotator CurrentControlRotation = GetController()->GetControlRotation();
    const FRotator CurrentControlRotationYaw(0.f, CurrentControlRotation.Yaw, 0.f);

    FVector ForwardDirection = FRotationMatrix(CurrentControlRotationYaw).GetUnitAxis(EAxis::X);
    FVector RightDirection = FRotationMatrix(CurrentControlRotationYaw).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDirection, MovementVector.X);
    AddMovementInput(RightDirection, MovementVector.Y);
}

void ASRPGCharacter::Look(const FInputActionValue& InValue)
{
    FVector2D LookAxisVector = InValue.Get<FVector2D>();

    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
}

void ASRPGCharacter::Attack(const FInputActionValue& InValue)
{
    if (0 == CurrentComboCount)
    {
        BeginCombo();
        return;
    }
    else
    {
        ensure(FMath::IsWithinInclusive<int32>(CurrentComboCount, 1, MaxComboCount));
        bIsAttackKeyPressed = true;
    }
}

void ASRPGCharacter::BeginCombo()
{
    USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
    if (false == ::IsValid(AnimInstance))
    {
        return;
    }

    CurrentComboCount = 1;

    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

    AnimInstance->PlayAttackAnimMontage();

    FOnMontageEnded OnMontageEndedDelegate;
    OnMontageEndedDelegate.BindUObject(this, &ThisClass::EndCombo);     //특정 몽타주에 바인딩할때 쓰는 함수
    AnimInstance->Montage_SetEndDelegate(OnMontageEndedDelegate, AnimInstance->AttackAnimMontage);
}

void ASRPGCharacter::CheckCanNextCombo()
{
    USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
    if (false == ::IsValid(AnimInstance))
    {
        return;
    }

    if (true == bIsAttackKeyPressed)
    {
        CurrentComboCount = FMath::Clamp(CurrentComboCount + 1, 1, MaxComboCount);

        FName NextSectionName = *FString::Printf(TEXT("%s%d"), *AttackAnimMontageSectionName, CurrentComboCount);
        AnimInstance->Montage_JumpToSection(NextSectionName, AnimInstance->AttackAnimMontage);
        bIsAttackKeyPressed = false;
    }
}

void ASRPGCharacter::EndCombo(UAnimMontage* InAnimMontage, bool bInterrupted)
{
    ensure(0 != CurrentComboCount);
    CurrentComboCount = 0;
    bIsAttackKeyPressed = false;
    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void ASRPGCharacter::CheckHit()
{
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CheckHit() has been called.")));

    FHitResult HitResult;
    FCollisionQueryParams Params(NAME_None, false, this);

    bool bResult = GetWorld()->SweepSingleByChannel(
        HitResult,
        GetActorLocation(),
        GetActorLocation() + 200 * GetActorForwardVector(),
        FQuat::Identity,
        ECC_GameTraceChannel2,
        FCollisionShape::MakeSphere(50.f),
        Params
    );

    if (true == bResult)
    {
        if (true == ::IsValid(HitResult.GetActor()))
        {
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Actor Name: %s"), *HitResult.GetActor()->GetName()));

            FDamageEvent DamageEvent;
            HitResult.GetActor()->TakeDamage(50.f, DamageEvent, GetController(), this);
        }
    }

#pragma region CollisionDebugDrawing
    FVector TraceVec = GetActorForwardVector() * AttackRange;
    FVector Center = GetActorLocation() + TraceVec * 0.5f;
    float HalfHeight = AttackRange * 0.5f + AttackRadius;
    FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
    FColor DrawColor = true == bResult ? FColor::Green : FColor::Red;
    float DebugLifeTime = 5.f;

    DrawDebugCapsule(
        GetWorld(),
        Center,
        HalfHeight,
        AttackRadius,
        CapsuleRot,
        DrawColor,
        false,
        DebugLifeTime
    );
#pragma endregion
}

float ASRPGCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float FinalDamageAmount = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

    /*CurrentHP = FMath::Clamp(CurrentHP - FinalDamageAmount, 0.f, MaxHP);

    if (CurrentHP < KINDA_SMALL_NUMBER)
    {
        bIsDead = true;
        CurrentHP = 0.f;
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
    }

    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s [%.1f / %.1f]"), *GetName(), CurrentHP, MaxHP));*/

    return FinalDamageAmount;
}

void ASRPGCharacter::OnCurrentLevelChanged(int32 InOldCurrentLevel, int32 InNewCurrentLevel)
{
    ParticleSystemComponent->Activate(true);
}

void ASRPGCharacter::OnAssetLoaded()
{
    AssetStreamableHandle->ReleaseHandle();
    TSoftObjectPtr<USkeletalMesh> LoadedAsset(CurrentPlayerCharacterMeshPath);
    if (true == LoadedAsset.IsValid())
    {
        GetMesh()->SetSkeletalMesh(LoadedAsset.Get());
    }
}

//void ASRPGCharacter::SetCurrentEXP(float InCurrentEXP)
//{
//    CurrentEXP = FMath::Clamp(CurrentEXP + InCurrentEXP, 0.f, MaxEXP);
//    if (MaxEXP - KINDA_SMALL_NUMBER < CurrentEXP)
//    {
//        CurrentEXP = 0.f;
//        ParticleSystemComponent->Activate(true);
//    }
//}