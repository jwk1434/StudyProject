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
    // CDO의 Name 속성에 저장됨.
    // 중단점을 걸어보면 언리얼 에디터가 실행되기 전에 호출됨을 알 수 있음.
}

void USGameInstance::Init()
{
    Super::Init(); // 엔진 업데이트 루틴을 지키기 위해서, 언리얼 엔지니어가 작성한 코드가 먼저 실행되게끔 하기 위함.

    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Init() has been called."));
    //UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Init() has been called."));


    // #1 : CDO 값 변경 확인
    UClass* RuntimeClassInfo = GetClass();
    UClass* CompiletimeClassInfo = StaticClass();

    //check(RuntimeClassInfo != CompiletimeClassInfo); 주석 풀어서 결과 확인 필요.
    //ensure(RuntimeClassInfo != CompiletimeClassInfo);
    //ensureMsgf(RuntimeClassInfo != CompiletimeClassInfo, TEXT("Intentional Error"));

    UE_LOG(LogTemp, Log, TEXT("Class Name: %s"), *RuntimeClassInfo->GetName());     //프로퍼티(리플렉션) 시스템으로 인해 런타임상에서 클래스명을 체크할 수 있다.

    Name = TEXT("USGameInstance Object");
    // CDO를 통해 생성된 개체의 Name 속성에 새롭게 대입되는 값.

    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Name: %s"), *(GetClass()->GetDefaultObject<USGameInstance>()->Name));
    UE_LOG(LogTemp, Log, TEXT("USGameInstance::Name: %s"), *Name);



    // #2 : 프로퍼티(리플렉션) 시스템으로 인해 런타임에서 클래스에 접근(네이밍)하여 변수 및 함수를 가져올 수 있다.

    USUnrealObjectClass* USObject1 = NewObject<USUnrealObjectClass>();
    // 언리얼은 이런식으로 new 키워드를 안쓰고 NewObject<>() API를 사용해야 함.

    UE_LOG(LogTemp, Log, TEXT("USObject1's Name: %s"), *USObject1->GetName());
    // 우리가 정의한 Getter()

    FProperty* NameProperty = USUnrealObjectClass::StaticClass()->FindPropertyByName(TEXT("Name"));
    // 프로퍼티 시스템을 활용한 Getter()

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
    // 현업에서 인터페이스 개념은 대부분 이런식으로 업캐스팅 하기 위함.

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

        ObjectDataFileWriterAr = nullptr; //delete ObjectDataFileWriterAr; 와 같은 효과.
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
