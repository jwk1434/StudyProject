#include "Components/SStatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Game/SGameInstance.h"
#include "Characters/SRPGCharacter.h"
#include "Game/SPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"


USStatComponent::USStatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = false;                  // 액터의 PostInitializeComponents() 함수에 대응하는 컴포넌트 함수는 InitializeComponent() 호출 유무
                                                        // 액터의 PostInitializeComponents() 함수가 호출되기 바로 전에 호출됨.
}

void USStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, MaxHP);
    DOREPLIFETIME(ThisClass, CurrentHP);
}

void USStatComponent::BeginPlay()
{
    Super::BeginPlay();

    GameInstance = Cast<USGameInstance>(GetWorld()->GetGameInstance());
    if (true == ::IsValid(GameInstance))
    {
        if (nullptr != GameInstance->GetCharacterStatDataTable() || nullptr != GameInstance->GetCharacterStatDataTableRow(1))
        {
            float NewMaxHP = GameInstance->GetCharacterStatDataTableRow(1)->MaxHP;
            SetMaxHP(NewMaxHP);
            SetCurrentHP(MaxHP);
        }
    }

    ASCharacter* OwnerPlayerCharacter = Cast<ASCharacter>(GetOwner());
    if (true == ::IsValid(OwnerPlayerCharacter))
    {
        ASPlayerState* PS = Cast<ASPlayerState>(OwnerPlayerCharacter->GetPlayerState());
        if (true == ::IsValid(PS))
        {
            if (false == PS->OnCurrentLevelChangedDelegate.IsAlreadyBound(this, &ThisClass::OnCurrentLevelChanged))
            {
                PS->OnCurrentLevelChangedDelegate.AddDynamic(this, &ThisClass::OnCurrentLevelChanged);
            }
        }
    }
}

void USStatComponent::SetMaxHP(float InMaxHP)
{
    if (true == OnMaxHPChangeDelegate.IsBound())
    {
        OnMaxHPChangeDelegate.Broadcast(MaxHP, InMaxHP);
    }

    MaxHP = FMath::Clamp<float>(InMaxHP, 0.f, 9999);
}

void USStatComponent::SetCurrentHP(float InCurrentHP)
{
    OnCurrentHPChanged_NetMulticast(CurrentHP, InCurrentHP);
    CurrentHP = FMath::Clamp<float>(InCurrentHP, 0.f, MaxHP);
    //if (true == OnCurrentHPChangeDelegate.IsBound())
    //{
    //    OnCurrentHPChangeDelegate.Broadcast(CurrentHP, InCurrentHP);
    //}

    //CurrentHP = FMath::Clamp<float>(InCurrentHP, 0.f, MaxHP);

    //if (CurrentHP < KINDA_SMALL_NUMBER)
    //{
    //    OnOutOfCurrentHPDelegate.Broadcast();
    //    CurrentHP = 0.f;
    //}

}

void USStatComponent::OnCurrentHPChanged_NetMulticast_Implementation(float InOldCurrentHP, float InNewCurrentHP)
{
    if (true == OnCurrentHPChangeDelegate.IsBound())
    {
        OnCurrentHPChangeDelegate.Broadcast(InOldCurrentHP, InNewCurrentHP);
    }

    if (InNewCurrentHP < KINDA_SMALL_NUMBER)
    {
        OnOutOfCurrentHPDelegate.Broadcast();
    }
}

void USStatComponent::OnCurrentLevelChanged(int32 InOldCurrentLevel, int32 InNewCurrentLevel)
{
    SetMaxHP(GameInstance->GetCharacterStatDataTableRow(InNewCurrentLevel)->MaxHP);
    SetCurrentHP(GameInstance->GetCharacterStatDataTableRow(InNewCurrentLevel)->MaxHP);
}