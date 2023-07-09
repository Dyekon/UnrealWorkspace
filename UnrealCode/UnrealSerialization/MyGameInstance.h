// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "MyGameInstance.generated.h"

struct FStudentData
{
	FStudentData() {}
	FStudentData(int32 InOrder, const FString& InName) : Order(InOrder), Name(InName) {}

	friend FArchive& operator<<(FArchive& Ar, FStudentData& InStudentData) //원하는 데이터를 편하게 넣기 위해 오버로딩된 함수를 재구현
	{
		Ar << InStudentData.Order;
		Ar << InStudentData.Name;
		return Ar;
	}

	int32 Order = -1;
	FString Name = TEXT("홍길동");
};

/**
 * 
 */
UCLASS()
class UNREALSERIALIZATION_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UMyGameInstance();

	virtual void Init() override;

	void SaveStudentPackage() const;
	void LoadStudentPackage() const;
	void LoadStudentObject() const;

private:

	static const FString PackageName;
	static const FString AssetName;

	UPROPERTY()
	TObjectPtr<class UStudent> StudentSrc;

	FStreamableManager StreamableManager; //비동기 로딩을 위한 스트리밍 매니저
	TSharedPtr<FStreamableHandle> Handle; //스트리밍 된 에셋을 관리할 수 있는 핸들
};
