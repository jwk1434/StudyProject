// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"

void USGameInstance::Init()
{
    Super::Init(); // ���� ������Ʈ ��ƾ�� ��Ű�� ���ؼ�, �𸮾� �����Ͼ �ۼ��� �ڵ尡 ���� ����ǰԲ� �ϱ� ����.

    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Init() has been called."));
    UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Init() has been called."));
}

void USGameInstance::Shutdown()
{
    Super::Shutdown();

    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Shutdown() has been called."));
}