// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplePlayer.h"

#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

// 생성자
ASimplePlayer::ASimplePlayer()
{	
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f); // 컬라이더용 컴포넌트 생성

	// 캐릭터 컨트롤러 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true; // 인풋 방향으로 이동
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 

	// 무브먼트 컴포넌트는 자주 변경하면서 세팅는 특성상 블루 프린트 제어 더 효율적이므로 코드 수정보다는 에디터에서 조정
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// 플레이어 카메라 설정
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // 카메라 암 거리
	CameraBoom->bUsePawnControlRotation = true; // 컨트롤러 기반으로 암회전
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 카메라붐 끝에 카메라 부착
	FollowCamera->bUsePawnControlRotation = false; // 카메라는 암에 대해 상대회전 끔

	PrimaryActorTick.bCanEverTick = true;
}


void ASimplePlayer::BeginPlay()
{
	Super::BeginPlay();
}

// 엔진 틱 업데이트 
void ASimplePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LockCamera)
	{
		// 카메라가 튀지 않게 부드럽게 보간 
		FRotator currentRot = GetControlRotation();
		FRotator lookAtRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetEnemy->GetActorLocation());
		FRotator targetRot = FRotator(0, lookAtRot.Yaw, 0);
		UKismetMathLibrary::RInterpTo(currentRot, targetRot, DeltaTime, 6.0f);

		APlayerController* OurPlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (OurPlayerController)
		{
			OurPlayerController->SetControlRotation(FRotator(targetRot.Pitch, targetRot.Yaw, currentRot.Roll));
		}
	}
}

// 인헨스
void ASimplePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// 인풋 매핑 컨텍스트 
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// 액션 각각 바인딩 : 점프,공격,구르기,마우스회전,이동,
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASimplePlayer::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ASimplePlayer::Attack);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASimplePlayer::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASimplePlayer::Look);
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &ASimplePlayer::Roll);
		EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Started, this, &ASimplePlayer::Lock);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Enhanced Input fail!"), *GetNameSafe(this));
	}
}

void ASimplePlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		// 포워드 방향 로테이션 얻고 지면기준수직축(Z) 회전(=Yaw)을 제외한 성분 제거 
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// 지면 수평 기준 포워드, 라이트 백터 성분에 
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// 인풋 X Y에 각각 매핑 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASimplePlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{		
		AddControllerYawInput(LookAxisVector.X); // X =  yaw
		AddControllerPitchInput(LookAxisVector.Y); // Y = pitch 에 매핑
	}
}

void ASimplePlayer::Jump(const FInputActionValue& Value)
{	
	ACharacter::Jump(); // 추가 구현 필요 없으므로 엔진내장 캐릭터 점프 사용
}

void ASimplePlayer::Lock(const FInputActionValue& Value)
{
	// 카메라 고정 기능 (토글)
	// 	
	if (!LockCamera)
	{		
		GetClosestEnemy(); // 최인접 적 찾아서
		UE_LOG(LogTemplateCharacter, Display, TEXT("Lock"));
		if (TargetEnemy != nullptr)
		{
			// 있으면 락 처리
			UE_LOG(LogTemplateCharacter, Display, TEXT("Target '%s'"), *GetNameSafe(TargetEnemy));
			LockCamera = true;			
		}
	}
	else
	{
		// 이미 락 중이면 락해제 
		LockCamera = false;
		TargetEnemy = nullptr;
		UE_LOG(LogTemplateCharacter, Display, TEXT("Unlock"));
	}

	bUseControllerRotationYaw = LockCamera;
	GetCharacterMovement()->bOrientRotationToMovement = !LockCamera;
}

void ASimplePlayer::GetClosestEnemy()
{
	FVector Center = GetActorLocation();
	float fRadius = 3000.0f;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
	ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn)); // 폰만 
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this); // 자신 제외

	float fMinDistance = 9999.0f;
	TargetEnemy = nullptr;

	TArray<AActor*> overlappedActors;
	if (UKismetSystemLibrary::SphereOverlapActors(GetWorld(), Center, fRadius, ObjectTypesArray, nullptr, IgnoredActors, overlappedActors))
	{
		for(AActor *pActor : overlappedActors)
		{
			FHitResult hitResult;

			if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), Center, pActor->GetActorLocation(),
				UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Pawn), 
				false, IgnoredActors, EDrawDebugTrace::ForDuration, hitResult, true))
			{
				UE_LOG(LogTemplateCharacter, Display, TEXT("foreach d2 '%s'"), *GetNameSafe(pActor));

				auto pHitActor = hitResult.GetActor();
				if (pHitActor == pActor)
				{
					float fDistance = GetDistanceTo(pHitActor);
					if (fMinDistance > fDistance)
					{
						fMinDistance = fDistance;
						TargetEnemy = pActor;
					}
				}
			}
		}
	}
}

void ASimplePlayer::Attack(const FInputActionValue& Value)
{
	if(MontageAttacks.IsEmpty()) return;
	if(MontageAttacks[AttackComboIndex] == nullptr) return;

	PlayAnimMontage(MontageAttacks[AttackComboIndex]);

	// 공격 애니 몽타주 0 1 2 3 0 로테이션 처리
	AttackComboIndex++;
	if (AttackComboIndex >= AttackComboIndexMax) AttackComboIndex = 0;

	UE_LOG(LogTemplateCharacter, Display, TEXT("ATK idx %d"), AttackComboIndex);

	// 캐릭터 앞 공격 판정용 파라미터 세팅
	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation + GetActorForwardVector() * 200.0f; // 2미터 앞
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
	ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn)); // 폰만 
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this); // 자신 제외

	// 적에 대한 구형 트레이스 판정
	FHitResult OutHit;
	bool bHasHit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), StartLocation, EndLocation, 50.f, ObjectTypesArray, false, IgnoredActors, EDrawDebugTrace::ForDuration, OutHit, true);
	if (bHasHit)
	{
		auto pHitActor = OutHit.GetActor();
		if (pHitActor != nullptr)
		{
			// 적에게 데미지 부여
			UGameplayStatics::ApplyDamage(pHitActor, 1, GetController(), nullptr, NULL);
			UE_LOG(LogTemplateCharacter, Display, TEXT("HIT '%s'"), *GetNameSafe(pHitActor));
		}		
	}
}

void ASimplePlayer::ResetAttack_Implementation()
{
	UE_LOG(LogTemplateCharacter, Display, TEXT("ATK idx 0"));
	AttackComboIndex = 0;
}

void ASimplePlayer::Fire(const FInputActionValue& Value)
{

}

void ASimplePlayer::Roll(const FInputActionValue& Value)
{
	PlayAnimMontage(MontageRoll); // 몽타주 플레이 
}


