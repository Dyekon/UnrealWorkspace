// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Student.h"
#include "JsonObjectConverter.h"
#include "UObject/SavePackage.h"

const FString UMyGameInstance::PackageName = TEXT("/Game/Student"); //Game폴더는 에셋들을 모아둔 대표 폴더, 파일 이름은 Student
const FString UMyGameInstance::AssetName = TEXT("TopStudent"); //Student라는 패키지가 메인으로 관리할 에셋 이름

void PrintStudentInfo(const UStudent* InStudent, const FString& InTag) //학생 정보를 출력하는 함수
{
	UE_LOG(LogTemp, Log, TEXT("[%s] 이름 %s 순번 %d"), *InTag, *InStudent->GetName(), InStudent->GetOrder());
}

UMyGameInstance::UMyGameInstance()
{
	//생성자에서 패키지를 불러옴. 만약 존재하지 않을 경우 강력한 경고와 에러 메세지를 불러오므로 조심
	const FString TopSoftObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);
	//언리얼에서 제공하는 ConstructorHelpers를 사용하여 원하는 에셋을 찾아 불러옴
	static ConstructorHelpers::FObjectFinder<UStudent> UASSET_TopStudet(*TopSoftObjectPath);
	if (UASSET_TopStudet.Succeeded())
	{
		PrintStudentInfo(UASSET_TopStudet.Object, TEXT("Constructor"));
	}
}

void UMyGameInstance::Init()
{
	Super::Init();

	FStudentData RawDataSrc(16, TEXT("이득우"));	//구조체 선언

	const FString SavedDir = FPaths::Combine(FPlatformMisc::ProjectDir(), TEXT("Saved")); //구조체 데이터를 파일로 저장하기 위한 파일 주소
	UE_LOG(LogTemp, Log, TEXT("저장할 파일 폴더 : %s"), *SavedDir);

	{
		const FString RawDataFileName(TEXT("RawData.bin")); //저장할 파일 이름 지정
		FString RawDataAbsolutePath = FPaths::Combine(*SavedDir, *RawDataFileName);
		UE_LOG(LogTemp, Log, TEXT("저장할 파일 전체 경로 : %s"), *RawDataAbsolutePath);
		FPaths::MakeStandardFilename(RawDataAbsolutePath); //파일 경로를 나타내는 String을 보기 좋게 바꿔줌
		UE_LOG(LogTemp, Log, TEXT("변경할 파일 전체 경로 : %s"), *RawDataAbsolutePath);

		//IFileManager라는 인터페이스에서 CreateFileWriter를 통해 RawData.bin파일을 쓸 수 있는 FArchive클래스를 생성할 수 있음
		FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*RawDataAbsolutePath);
		if (nullptr != RawFileWriterAr) //생성이 잘 완료됬는지 확인
		{
			*RawFileWriterAr << RawDataSrc; //데이터 입력
			RawFileWriterAr->Close(); //전송이 완료되면 파일 닫기
			delete RawFileWriterAr; //파일 삭제
			RawFileWriterAr = nullptr; //초기화
		}

		FStudentData RawDataDest;
		//IFileManager라는 인터페이스에서 CreateFileWriter를 통해 RawData.bin파일을 읽을 수 있는 FArchive클래스를 생성할 수 있음
		FArchive* RawFileReaderAr = IFileManager::Get().CreateFileReader(*RawDataAbsolutePath);
		if (nullptr != RawFileReaderAr)
		{
			*RawFileReaderAr << RawDataDest; //알아서 자동으로 진행되기 때문에 불러오든 저장하든 <<operator를 사용하면 됨(이 경우는 거꾸로 불러줌)
			RawFileReaderAr->Close();
			delete RawFileReaderAr;
			RawFileReaderAr = nullptr;

			UE_LOG(LogTemp, Log, TEXT("[RawData] 이름 %s 순번 %d"), *RawDataDest.Name, RawDataDest.Order)
		}
	}

	StudentSrc = NewObject<UStudent>();
	StudentSrc->SetName(TEXT("이득우"));
	StudentSrc->SetOrder(59);

	{
		const FString ObjectDataFileName(TEXT("ObjectData.bin")); //저장할 파일 이름 지정
		FString ObjectDataAbsolutePath = FPaths::Combine(*SavedDir, *ObjectDataFileName); //데이터를 파일로 저장하기 위한 파일 주소
		FPaths::MakeStandardFilename(ObjectDataAbsolutePath); //파일 경로를 나타내는 String을 보기 좋게 바꿔줌

		TArray<uint8> BufferArray; //직렬화를 위한 Buffer선언
		FMemoryWriter MemoryWriterAr(BufferArray); //메모리 아카이브 클래스 중 하나이며 선언한 Buffer와 연동되는 FMemoryWriter를 선언
		StudentSrc->Serialize(MemoryWriterAr); //직렬화

		//스마터 포인터 라이브러리인 TUniquePtr를 이용하여 삭제, 초기화 없이 간편하게 선언할 수 있음
		if (TUniquePtr<FArchive> FileWriterAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(*ObjectDataAbsolutePath)))
		{
			*FileWriterAr << BufferArray;
			FileWriterAr->Close();
		}

		TArray<uint8> BufferArrayFromFile;
		//파일을 읽기 위하여 선언
		if (TUniquePtr<FArchive> FileReaderAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileReader(*ObjectDataAbsolutePath)))
		{
			*FileReaderAr << BufferArrayFromFile;
			FileReaderAr->Close();
		}

		//메모리 아카이브 클래스 중 하나이며 선언한 Buffer와 연동되는 FMemoryReader를 선언
		//Buffer에 있는 데이터를 다시 메모리로 전송
		FMemoryReader MemoryReaderAr(BufferArrayFromFile); 
		UStudent* StudentDest = NewObject<UStudent>();
		//직렬화를 시키면 StudentDest라는 새로운 객체가 BufferArrayFromFile의 내용으로 덮어씌워지게 됨
		StudentDest->Serialize(MemoryReaderAr);
		PrintStudentInfo(StudentDest, TEXT("ObjectData"));

	}

	{
		//unresolved external symbol이라는 에러는 ***.Build.cs 파일에서
		//"Json" "JsonUtilities"같은 Json에 관련된 라이브러리를 연동시켜줌으로서 해결할 수 있음

		const FString JsonDataFileName(TEXT("StudentJsonData.txt")); //저장할 파일 이름 지정
		FString JsonDataAbsolutePath = FPaths::Combine(*SavedDir, *JsonDataFileName); //데이터를 파일로 저장하기 위한 파일 주소
		FPaths::MakeStandardFilename(JsonDataAbsolutePath); //파일 경로를 나타내는 String을 보기 좋게 바꿔줌

		//언리얼 오브젝트를 Json오브젝트를 변환시켜야 함
		//JsonObjectSrc는 Null이 아님을 보장받기 위하여 MakeShared라는 API를 사용함
		TSharedRef<FJsonObject> JsonObjectSrc = MakeShared<FJsonObject>();
		//UStruct의 정보를 JsonObject로 넘겨줌
		FJsonObjectConverter::UStructToJsonObject(StudentSrc->GetClass(), StudentSrc, JsonObjectSrc);

		FString JsonOutString;
		//JsonWriterFactory에 의해서 JsonWriterAr라는 Json으로 써주는 Archive가 만들어짐
		TSharedRef<TJsonWriter<TCHAR>> JsonWriterAr = TJsonWriterFactory<TCHAR>::Create(&JsonOutString);
		if (FJsonSerializer::Serialize(JsonObjectSrc, JsonWriterAr)) //Json직렬화 진행
		{
			//FFileHelper를 사용하면 인코딩 신경쓰지 않아도 운영체제에 맞게 알아서 잘 저장해줌
			FFileHelper::SaveStringToFile(JsonOutString, *JsonDataAbsolutePath);
		}

		FString JsonInString;
		FFileHelper::LoadFileToString(JsonInString, *JsonDataAbsolutePath); //Json파일의 문자열을 불러옴

		//Json을 읽는 ReaderArchive가 생성됨
		TSharedRef<TJsonReader<TCHAR>> JsonReaderAr = TJsonReaderFactory<TCHAR>::Create(JsonInString);

		//Reader로부터 실제로 변환할 JsonObject 선언
		//문자열이 이상한 것이 들어오면 만들어지지 않을 수 있고 Null이 들어올 수 있으므로 포인터로 선언
		TSharedPtr<FJsonObject> JsonObjectDest;
		if (FJsonSerializer::Deserialize(JsonReaderAr, JsonObjectDest)) //읽기 위하여 직렬화를 해제
		{
			UStudent* JsonStudentDest = NewObject<UStudent>(); //새로운 언리얼 오브젝트 생성
			//JsonObject의 정보를 UStruct로 넘겨줌
			//공유 레퍼런스를 얻기 위해 공유 포인터에 ToSharedRef를 이용하여 얻어줌
			if (FJsonObjectConverter::JsonObjectToUStruct(JsonObjectDest.ToSharedRef(), JsonStudentDest->GetClass(), JsonStudentDest))
			{
				PrintStudentInfo(JsonStudentDest, TEXT("JsonData"));
			}
		}
	}
	SaveStudentPackage();
	//LoadStudentPackage();
	//LoadStudentObject();


	const FString TopSoftObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName); //에셋의 오브젝트 경로 생성
	Handle = StreamableManager.RequestAsyncLoad(TopSoftObjectPath,	//비동기 방식으로 원하는 에셋을 불러옴
		[&]()//람다 함수를 이용함
		{
			if (Handle.IsValid() && Handle->HasLoadCompleted()) //핸들이 유효한지, 로딩이 완료되었는지 확인
			{
				//Cast를 이용하여 언리얼 오브젝트를 받아와 형변환을 시켜 UStudent인지 확인
				UStudent* TopStudent = Cast<UStudent>(Handle->GetLoadedAsset());
				if (TopStudent)
				{
					PrintStudentInfo(TopStudent, TEXT("AssyncLoad"));

					//핸들을 다 사용했다면 닫아줌
					Handle->ReleaseHandle();
					Handle.Reset();
				}
			}
		}
	);
}

/////////////////////////////////////언리얼 오브젝트 패키지

void UMyGameInstance::SaveStudentPackage() const
{
	UPackage* StudentPackage = ::LoadPackage(nullptr, *PackageName, LOAD_None); //이미 패키지가 있을 경우 미리 불러온 후 입력함
	if (StudentPackage)
	{
		StudentPackage->FullyLoad();
	}
	
	StudentPackage = CreatePackage(*PackageName); //패키지 생성
	EObjectFlags ObjectFlag = RF_Public | RF_Standalone; //패키지를 저장하는 옵션 설정, Enum설정

	//학생 클래스 생성, Student패키지 안에 저장해야 하기 때문에 평소와 다르게 필요한 옵션들을 입력함
	UStudent* TopStudent = NewObject<UStudent>(StudentPackage, UStudent::StaticClass(), *AssetName, ObjectFlag);
	TopStudent->SetName(TEXT("이득우")); //내용 입력
	TopStudent->SetOrder(36);

	const int32 NumfSubs = 10;
	for (int32 ix = 1; ix <= NumfSubs; ++ix) //TopStudent에 대한 서브 오브젝트를 10개 생성
	{
		FString SubObjectName = FString::Printf(TEXT("Student%d"), ix); //서브 오브젝트들의 이름
		//Student패키지 안의 SubObjectName으로 생성
		UStudent* SubStudent = NewObject<UStudent>(TopStudent, UStudent::StaticClass(), *SubObjectName, ObjectFlag);
		SubStudent->SetName(FString::Printf(TEXT("학생%d"), ix));
		SubStudent->SetOrder(ix);
	}
	//패키지들을 저장하기 위해 경로와 확장자를 부여해야함. FPackageName::GetAssetPackageExtension은 uasset파일 확장자를 만들어줌
	const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = ObjectFlag;

	if (UPackage::SavePackage(StudentPackage, nullptr, *PackageFileName, SaveArgs)) //패키지 저장
	{
		UE_LOG(LogTemp, Log, TEXT("패키지가 성공적으로 저장되었습니다."));
	}

}

void UMyGameInstance::LoadStudentPackage() const
{
	UPackage* StudentPackage = ::LoadPackage(nullptr, *PackageName, LOAD_None); //저장한 패키지를 불러옴. LOAD_None을 적으면 기본 옵션으로 설정
	if (nullptr == StudentPackage) //만약 없다면 파일을 찾을 수 없다고 로그 출력
	{
		UE_LOG(LogTemp, Warning, TEXT("패키지를 찾을 수 없습니다."));
		return;
	}

	StudentPackage->FullyLoad(); //안에 있는 에셋을 전부 불러와옴

	UStudent* TopStudent = FindObject<UStudent>(StudentPackage, *AssetName); //원하는 오브젝트를 찾음
	PrintStudentInfo(TopStudent, TEXT("FindObject Asset"));

}

void UMyGameInstance::LoadStudentObject() const //오브젝트 경로를 이용하여 불러오기
{
	const FString TopSoftObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName); //에셋의 오브젝트 경로 생성

	UStudent* TopStudent = LoadObject<UStudent>(nullptr, *TopSoftObjectPath); //경로를 입력하여 원하는 정보를 불러옴
	PrintStudentInfo(TopStudent, TEXT("LoadObjestAsset"));

}
