#include "WorldStatic/SLandMine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ASLandMine::ASLandMine()
    : bIsExploded(false)
{
    PrimaryActorTick.bCanEverTick = true;

    BodyBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BodyBoxComponent"));
    SetRootComponent(BodyBoxComponent);

    BodyStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyStaticMeshComponent"));
    BodyStaticMeshComponent->SetupAttachment(GetRootComponent());

    ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetupAttachment(GetRootComponent());
    ParticleSystemComponent->SetAutoActivate(false);
}

void ASLandMine::BeginPlay()
{
    Super::BeginPlay();

    if (false == OnActorBeginOverlap.IsAlreadyBound(this, &ThisClass::OnLandMineBeginOverlap))
    {
        OnActorBeginOverlap.AddDynamic(this, &ThisClass::OnLandMineBeginOverlap);
    }

    //UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::BeginPlay() has been called.")));

    if (true == HasAuthority())
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::BeginPlay() has been called in Server PC.")));
    }
    else
    {
        if (GetOwner() == UGameplayStatics::GetPlayerController(this, 0))
        {
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::BeginPlay() has been called in OwningClient PC.")));
        }
        else
        {
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::BeginPlay() has been called in OtherClient PC.")));
        }
    }
}

void ASLandMine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, bIsExploded);
}

void ASLandMine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (true == OnActorBeginOverlap.IsAlreadyBound(this, &ThisClass::OnLandMineBeginOverlap))
    {
        OnActorBeginOverlap.RemoveDynamic(this, &ThisClass::OnLandMineBeginOverlap);
    }

    //UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::EndPlay() has been called.")));

    if (true == HasAuthority())
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::EndPlay() has been called in Server PC.")));
    }
    else
    {
        if (GetOwner() == UGameplayStatics::GetPlayerController(this, 0))
        {
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::EndPlay() has been called in OwningClient PC.")));
        }
        else
        {
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::EndPlay() has been called in OtherClient PC.")));
        }
    }
}


void ASLandMine::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ASLandMine::OnLandMineBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (true == HasAuthority())
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::OnLandMineBeginOverlap(%d) has been called in Server PC."), bIsExploded));
    }
    else
    {
        if (GetOwner() == UGameplayStatics::GetPlayerController(this, 0))
        {
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::OnLandMineBeginOverlap(%d) has been called in OwningClient PC."), bIsExploded));
        }
        else
        {
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::OnLandMineBeginOverlap(%d) has been called in OtherClient PC."), bIsExploded));
        }
    }

    if (true == HasAuthority() && false == bIsExploded)
    {
        SpawnEffect_NetMulticast();
        bIsExploded = true;
    }
}

void ASLandMine::SpawnEffect_NetMulticast_Implementation()
{
    ParticleSystemComponent->Activate(true);
}

void ASLandMine::OnRep_IsExploded()
{
    if (true == bIsExploded)
    {
        BodyStaticMeshComponent->SetMaterial(0, ExplodedMaterial);
    }
}
