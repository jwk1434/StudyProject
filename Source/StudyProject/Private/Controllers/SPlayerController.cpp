// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/SPlayerController.h"
#include "UI/SHUD.h"
#include "Game/SPlayerState.h"
#include "Components/SStatComponent.h"
#include "Characters/SRPGCharacter.h"


ASPlayerController::ASPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    // ������ Tick�� ���ƾ� �Էµ� ó���� �� ����.
}

void ASPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
}

void ASPlayerController::BeginPlay()
{
    Super::BeginPlay();

    FInputModeGameOnly InputModeGameOnly;
    SetInputMode(InputModeGameOnly);

    if (true == ::IsValid(HUDWidgetClass))
    {
        HUDWidget = CreateWidget<USHUD>(this, HUDWidgetClass);
        if (true == ::IsValid(HUDWidget))
        {
            HUDWidget->AddToViewport();

            ASPlayerState* SPlayerState = GetPlayerState<ASPlayerState>();
            if (true == ::IsValid(SPlayerState))
            {
                HUDWidget->BindPlayerState(SPlayerState);
            }

            ASCharacter* PC = GetPawn<ASCharacter>();
            if (true == ::IsValid(PC))
            {
                USStatComponent* StatComponent = PC->GetStatComponent();
                if (true == ::IsValid(StatComponent))
                {
                    HUDWidget->BindStatComponent(StatComponent);
                }
            }
        }
    }

    if (true == ::IsValid(MenuUIClass))
    {
        MenuUIInstance = CreateWidget<UUserWidget>(this, MenuUIClass);
        if (true == ::IsValid(MenuUIInstance))
        {
            MenuUIInstance->AddToViewport(3); // ������ ���.

            MenuUIInstance->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void ASPlayerController::ToggleMenu()
{
    if (false == bIsMenuOn)
    {
        MenuUIInstance->SetVisibility(ESlateVisibility::Visible);

        FInputModeUIOnly Mode;
        Mode.SetWidgetToFocus(MenuUIInstance->GetCachedWidget());
        SetInputMode(Mode);
        SetPause(true);

        bShowMouseCursor = true;
    }
    else
    {
        MenuUIInstance->SetVisibility(ESlateVisibility::Collapsed);

        FInputModeGameOnly InputModeGameOnly;
        SetInputMode(InputModeGameOnly);
        SetPause(false);

        bShowMouseCursor = false;
    }

    bIsMenuOn = !bIsMenuOn;
}