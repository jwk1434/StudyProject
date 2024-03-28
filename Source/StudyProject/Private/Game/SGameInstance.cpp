// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"

void USGameInstance::Init()
{
    Super::Init(); // ���� ������Ʈ ��ƾ�� ��Ű�� ���ؼ�, �𸮾� �����Ͼ �ۼ��� �ڵ尡 ���� ����ǰԲ� �ϱ� ����.

    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Init() has been called."));
    UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Init() has been called."));

    if (false == ::IsValid(CharacterStatDataTable) || CharacterStatDataTable->GetRowMap().Num() <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Not enuough data in CharacterStatDataTable."));
    }
    else
    {
        for (int32 i = 1; i <= CharacterStatDataTable->GetRowMap().Num(); ++i)
        {
            check(nullptr != GetCharacterStatDataTableRow(i));
        }
    }
}

FSStatTableRow* USGameInstance::GetCharacterStatDataTableRow(int32 InLevel)
{
    if (true == ::IsValid(CharacterStatDataTable))
    {
        return CharacterStatDataTable->FindRow<FSStatTableRow>(*FString::FromInt(InLevel), TEXT(""));
    }

    return nullptr;
}

void USGameInstance::Shutdown()
{
    Super::Shutdown();

    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Shutdown() has been called."));
}