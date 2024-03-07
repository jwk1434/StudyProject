// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API USGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	USGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

private:

	void StudyForSerialize();

	UPROPERTY()
	FString Name;

	UPROPERTY()
	TObjectPtr<class USPigeon> SerializedPigeon;
	// 헤더 파일에서 다른 헤더 파일을 참조하면, 다른 헤더 파일이 수정되었을 때 이 헤더 파일도 함께 컴파일 됨.
	// 따라서 꼭 필요한 경우가 아니라면 헤더 파일에서 다른 헤더파일을 인클루드 하지 않고 전방선언을 활용함.
};
