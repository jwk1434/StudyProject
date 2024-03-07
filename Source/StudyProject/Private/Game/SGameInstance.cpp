// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SUnrealObjectClass.h"
#include "Examples/SFlyable.h"
#include "Examples/SPigeon.h"

USGameInstance::USGameInstance()
{
    UE_LOG(LogTemp, Log, TEXT("USGameInstance::USGameInstance() has been called."));

    Name = TEXT("USGameInstance Class Default Object");
    // CDO�� Name �Ӽ��� �����.
    // �ߴ����� �ɾ�� �𸮾� �����Ͱ� ����Ǳ� ���� ȣ����� �� �� ����.
}

void USGameInstance::Init()
{
    Super::Init(); // ���� ������Ʈ ��ƾ�� ��Ű�� ���ؼ�, �𸮾� �����Ͼ �ۼ��� �ڵ尡 ���� ����ǰԲ� �ϱ� ����.

    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Init() has been called."));
    //UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Init() has been called."));


    // #1 : CDO �� ���� Ȯ��
    UClass* RuntimeClassInfo = GetClass();
    UClass* CompiletimeClassInfo = StaticClass();

    //check(RuntimeClassInfo != CompiletimeClassInfo); �ּ� Ǯ� ��� Ȯ�� �ʿ�.
    //ensure(RuntimeClassInfo != CompiletimeClassInfo);
    //ensureMsgf(RuntimeClassInfo != CompiletimeClassInfo, TEXT("Intentional Error"));

    UE_LOG(LogTemp, Log, TEXT("Class Name: %s"), *RuntimeClassInfo->GetName());     //������Ƽ(���÷���) �ý������� ���� ��Ÿ�ӻ󿡼� Ŭ�������� üũ�� �� �ִ�.

    Name = TEXT("USGameInstance Object");
    // CDO�� ���� ������ ��ü�� Name �Ӽ��� ���Ӱ� ���ԵǴ� ��.

    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Name: %s"), *(GetClass()->GetDefaultObject<USGameInstance>()->Name));
    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Name: %s"), *Name);



    // #2 : ������Ƽ(���÷���) �ý������� ���� ��Ÿ�ӿ��� Ŭ������ ����(���̹�)�Ͽ� ���� �� �Լ��� ������ �� �ִ�.

    USUnrealObjectClass* USObject1 = NewObject<USUnrealObjectClass>();
    // �𸮾��� �̷������� new Ű���带 �Ⱦ��� NewObject<>() API�� ����ؾ� ��.

    UE_LOG(LogTemp, Log, TEXT("USObject1's Name: %s"), *USObject1->GetName());
    // �츮�� ������ Getter()

    FProperty* NameProperty = USUnrealObjectClass::StaticClass()->FindPropertyByName(TEXT("Name"));
    // ������Ƽ �ý����� Ȱ���� Getter()

    FString CompiletimeUSObjectName;
    if (nullptr != NameProperty)
    {
        NameProperty->GetValue_InContainer(USObject1, &CompiletimeUSObjectName);
        UE_LOG(LogTemp, Log, TEXT("CompiletimeUSObjectName: %s"), *CompiletimeUSObjectName);
    }

    USObject1->HelloUnreal();

    UFunction* HelloUnrealFunction = USObject1->GetClass()->FindFunctionByName(TEXT("HelloUnreal"));
    if (nullptr != HelloUnrealFunction)
    {
        USObject1->ProcessEvent(HelloUnrealFunction, nullptr);
    }


    USPigeon* Pigeon1 = NewObject<USPigeon>();
    ISFlyable* Bird1 = Cast<ISFlyable>(Pigeon1);
    // �������� �������̽� ������ ��κ� �̷������� ��ĳ���� �ϱ� ����.

    if (nullptr != Bird1)
    {
        Bird1->Fly();
    }

    StudyForSerialize();

}

void USGameInstance::Shutdown()
{
    Super::Shutdown();

    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Shutdown() has been called."));
}

void USGameInstance::StudyForSerialize()
{
    FBirdData SrcRawData(TEXT("Pigeon17"), 17);
    UE_LOG(LogTemp, Log, TEXT("[SrcRawData] Name: %s, ID: %d"), *SrcRawData.Name, SrcRawData.ID);

    const FString SavedDir = FPaths::Combine(FPlatformMisc::ProjectDir(), TEXT("Saved"));
    UE_LOG(LogTemp, Log, TEXT("SavedDir: %s"), *SavedDir);

    const FString RawDataFileName(TEXT("RawData.bin"));
    FString AbsolutePathForRawData = FPaths::Combine(*SavedDir, *RawDataFileName);
    UE_LOG(LogTemp, Log, TEXT("Relative path for saved file: %s"), *AbsolutePathForRawData);
    FPaths::MakeStandardFilename(AbsolutePathForRawData);
    UE_LOG(LogTemp, Log, TEXT("Absolute path for saved file: %s"), *AbsolutePathForRawData);

    FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*AbsolutePathForRawData);
    if (nullptr != RawFileWriterAr)
    {
        *RawFileWriterAr << SrcRawData;
        RawFileWriterAr->Close();
        delete RawFileWriterAr;
        RawFileWriterAr = nullptr;
    }

    FBirdData DstRawData;
    FArchive* RawFileReaderAr = IFileManager::Get().CreateFileReader(*AbsolutePathForRawData);
    if (nullptr != RawFileReaderAr)
    {
        *RawFileReaderAr << DstRawData;
        RawFileReaderAr->Close();
        delete RawFileReaderAr;
        RawFileReaderAr = nullptr;

        UE_LOG(LogTemp, Log, TEXT("[DstRawData] Name: %s, ID: %d"), *DstRawData.Name, DstRawData.ID);
    }

    SerializedPigeon = NewObject<USPigeon>();
    SerializedPigeon->SetName(TEXT("Pigeon76"));
    SerializedPigeon->SetID(76);
    UE_LOG(LogTemp, Log, TEXT("[SerializedPigeon] Name: %s, ID: %d"), *SerializedPigeon->GetName(), SerializedPigeon->GetID());

    const FString ObjectDataFileName(TEXT("ObjectData.bin"));
    FString AbsolutePathForObjectData = FPaths::Combine(*SavedDir, *ObjectDataFileName);
    FPaths::MakeStandardFilename(AbsolutePathForObjectData);

    TArray<uint8> BufferArray;
    FMemoryWriter MemoryWriterAr(BufferArray);
    SerializedPigeon->Serialize(MemoryWriterAr);

    TUniquePtr<FArchive> ObjectDataFileWriterAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(*AbsolutePathForObjectData));
    if (nullptr != ObjectDataFileWriterAr)
    {
        *ObjectDataFileWriterAr << BufferArray;
        ObjectDataFileWriterAr->Close();

        ObjectDataFileWriterAr = nullptr; //delete ObjectDataFileWriterAr; �� ���� ȿ��.
    }

    TArray<uint8> BufferArrayFromObjectDataFile;
    TUniquePtr<FArchive> ObjectDataFileReaderAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileReader(*AbsolutePathForObjectData));
    if (nullptr != ObjectDataFileReaderAr)
    {
        *ObjectDataFileReaderAr << BufferArrayFromObjectDataFile;
        ObjectDataFileReaderAr->Close();

        ObjectDataFileReaderAr = nullptr;
    }

    FMemoryReader MemoryReaderAr(BufferArrayFromObjectDataFile);
    USPigeon* Pigeon77 = NewObject<USPigeon>();
    Pigeon77->Serialize(MemoryReaderAr);
    UE_LOG(LogTemp, Log, TEXT("[Pigeon77] Name: %s, ID: %d"), *Pigeon77->GetName(), Pigeon77->GetID());
}
