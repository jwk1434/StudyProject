#include "UI/SW_HPBar.h"
#include "Components/ProgressBar.h"
#include "Characters/SCharacter.h"
#include "Components/SStatComponent.h"

void USW_HPBar::SetMaxHP(float InMaxHP)
{
    SetMaxFigure(InMaxHP);
}

void USW_HPBar::InitializeHPBarWidget(USStatComponent* NewStatComponent)
{
    OnCurrentHPChange(0, NewStatComponent->GetCurrentHP());
}

void USW_HPBar::OnMaxHPChange(float InOldMaxHP, float InNewMaxHP)
{
    SetMaxFigure(InNewMaxHP);

    OnCurrentHPChange(0, InNewMaxHP);
}

void USW_HPBar::OnCurrentHPChange(float InOldHP, float InNewHP)
{
    if (true == ::IsValid(Bar))
    {
        if (KINDA_SMALL_NUMBER < MaxFigure)
        {
            Bar->SetPercent(InNewHP / MaxFigure);
        }
        else
        {
            Bar->SetPercent(0.f);
        }
    }
}

void USW_HPBar::NativeConstruct()
{
    Super::NativeConstruct();

    ASCharacter* OwningCharacter = Cast<ASCharacter>(OwningActor);
    if (true == ::IsValid(OwningCharacter))
    {
        OwningCharacter->SetWidget(this);
    }

    // UI 관련 컴포넌트는 액터의 BeginPlay 이후에 호출 됨
    // 생성시 위젯컴포넌트의 InitWidget 함수와 NativeConstruct 함수를 호출
    // 유저 위젯과 스탯 컴포넌트를 연결시킬 타이밍 필요
}