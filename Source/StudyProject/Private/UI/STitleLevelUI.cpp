#include "UI/STitleLevelUI.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Game/SPlayerStateSave.h"
#include "Game/SPlayerState.h"

USTitleLevelUI::USTitleLevelUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USTitleLevelUI::NativeConstruct()
{
	NewGameButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnNewGameButtonClicked);
	ExitGameButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnExitGameButtonClicked);

	LoadGameButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnLoadGameButtonClicked);

	LoadGameButton->SetIsEnabled(false);
	const ASPlayerState* PS = GetDefault<ASPlayerState>();
	if (true == ::IsValid(PS))
	{
		USPlayerStateSave* PlayerStateSave = Cast<USPlayerStateSave>(UGameplayStatics::LoadGameFromSlot(PS->SaveSlotName, 0));
		if (true == ::IsValid(PlayerStateSave))
		{
			LoadGameButton->SetIsEnabled(true);
		}
	}
}

void USTitleLevelUI::OnNewGameButtonClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Example"));

	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Loading")), true, FString(TEXT("NextLevel=Lobby?Saved=false")));
	// NextLevel�� Key, Example�� Value��. �׷� Loading ���������� NextLevel�� �Ľ��ؼ� Example ���� ���� ��.
}

void USTitleLevelUI::OnExitGameButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void USTitleLevelUI::OnLoadGameButtonClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Loading")), true, FString(TEXT("NextLevel=Example?Saved=true")));
}