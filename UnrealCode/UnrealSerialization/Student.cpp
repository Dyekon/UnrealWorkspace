// Fill out your copyright notice in the Description page of Project Settings.


#include "Student.h"

UStudent::UStudent()
{
	Order = -1;
	Name = TEXT("홍길동");
}

void UStudent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar); //오버라이드를 진행하면 Super를 적어주기

	Ar << Order;
	Ar << Name;
}
