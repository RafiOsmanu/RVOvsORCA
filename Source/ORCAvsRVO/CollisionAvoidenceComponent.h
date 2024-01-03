// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AgentInterface.h"
#include "Components/ActorComponent.h"
#include "CollisionAvoidenceComponent.generated.h"

class AORCAvsRVOCharacter;



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ORCAVSRVO_API UCollisionAvoidenceComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this component's properties
	UCollisionAvoidenceComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void CalculateVelocityObject(const AORCAvsRVOCharacter* agentToAvoid, const bool avoidCollision = true);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AvoidanceInfo")
	float m_MaxTimeRelavancy = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AvoidanceInfo")
	bool m_DrawDebug = false;

private:
	TArray<FVector2D> m_VelocityObject;

	FVector2D CalcVelocityFromAngleAndSpeed(double angle, double speed);
	bool IsIntersecting(FVector2D futurePos1, FVector2D futurePos2, float radius1, float radius2);

	bool IsOnCollisionCourse(const AORCAvsRVOCharacter* agentToAvoid);

	void AvoidCollision(const AORCAvsRVOCharacter* avoidanceAgent);

	
		
};
