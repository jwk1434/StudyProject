// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"

void USGameInstance::Init()
{
    Super::Init(); // 엔진 업데이트 루틴을 지키기 위해서, 언리얼 엔지니어가 작성한 코드가 먼저 실행되게끔 하기 위함.

    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Init() has been called."));
    UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Init() has been called."));
}

void USGameInstance::Shutdown()
{
    Super::Shutdown();

    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Shutdown() has been called."));
}