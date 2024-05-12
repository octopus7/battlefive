// Fill out your copyright notice in the Description page of Project Settings.

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"

#include "EnemyAIController.h"

//const FName AEnemyAIController::HomePosKey(TEXT("HomePos"));
//const FName AEnemyAIController::PatrolPosKey(TEXT("PatrolPos"));

AEnemyAIController::AEnemyAIController()
{
	static ConstructorHelpers::FObjectFinder<UBlackboardData> BBObject(TEXT("BlackboardData'/Game/Player/AI/BB_Enemy.BB_Enemy'"));
	if (BBObject.Succeeded())
	{
		BBEnemy = BBObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTObject(TEXT("BehaviorTree'/Game/Player/AI/BT_Enemy.BT_Enemy'"));
	if (BTObject.Succeeded())
	{
		BTEnemy = BTObject.Object;
	}
}



void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (!RunBehaviorTree(BTEnemy))
	{
		UE_LOG(LogTemp, Error, TEXT("AIController couldn't run behavior tree!"));
	}
}