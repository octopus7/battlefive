// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"


UCLASS()
class BATTLEFIVE_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

private:
	UPROPERTY()
	class UBlackboardData* BBEnemy;

	UPROPERTY()
	class UBehaviorTree* BTEnemy;

public:
	AEnemyAIController();

	void OnPossess(APawn* InPawn);
	
};
