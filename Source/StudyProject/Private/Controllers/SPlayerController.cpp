// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/SPlayerController.h"

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
}
