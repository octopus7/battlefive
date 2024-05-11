// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplePlayer.h"

#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

// ������
ASimplePlayer::ASimplePlayer()
{
	// ������Ʈ ���� �ʱ�ȭ	
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// ����
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // ��ǲ �������� �̵�
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// �����Ʈ ������Ʈ�� ���� �����ϸ鼭 ���ô� Ư���� ��� ����Ʈ ���� �� ȿ�����̹Ƿ� �ڵ� �������ٴ� �����Ϳ��� ����
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// �÷��̾� ī�޶� ����
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // ī�޶� �� �Ÿ�
	CameraBoom->bUsePawnControlRotation = true; // ��Ʈ�ѷ� ������� ��ȸ��
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // ī�޶�� ���� ī�޶� ����
	FollowCamera->bUsePawnControlRotation = false; // ī�޶�� �Ͽ� ���� ���ȸ�� ��

	// �̰ű��� �ڵ�� ¥���ϳ� �ͱ⵵ ������ �ϴ� ������ ������
	// PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASimplePlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASimplePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASimplePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASimplePlayer::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ASimplePlayer::Attack);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASimplePlayer::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASimplePlayer::Look);
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &ASimplePlayer::Roll);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Enhanced Input fail!"), *GetNameSafe(this));
	}
}

void ASimplePlayer::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASimplePlayer::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASimplePlayer::Jump(const FInputActionValue& Value)
{
	ACharacter::Jump();
	//UE_LOG(LogTemplateCharacter, Display, TEXT("'%s' JUMP"), *GetNameSafe(this));
}

void ASimplePlayer::Lock(const FInputActionValue& Value)
{

}

void ASimplePlayer::Attack(const FInputActionValue& Value)
{
	if (MontageAttacks.IsEmpty()) return;
	if(MontageAttacks[AttackComboIndex] == nullptr) return;

	PlayAnimMontage(MontageAttacks[AttackComboIndex]);
	AttackComboIndex++;
	if (AttackComboIndex >= AttackComboIndexMax) AttackComboIndex = 0;
	//UE_LOG(LogTemplateCharacter, Display, TEXT("'%s' ATTACK"), *GetNameSafe(this));
}

void ASimplePlayer::Fire(const FInputActionValue& Value)
{

}

void ASimplePlayer::Roll(const FInputActionValue& Value)
{
	PlayAnimMontage(MontageRoll);
	//UE_LOG(LogTemplateCharacter, Display, TEXT("'%s' ROLL"), *GetNameSafe(this));
}

