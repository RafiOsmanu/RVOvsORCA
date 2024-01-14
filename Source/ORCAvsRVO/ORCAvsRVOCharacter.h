// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AgentInterface.h"
#include "CollisionAvoidenceComponent.h"
#include "ORCAvsRVOCharacter.generated.h"

UENUM(BlueprintType)
enum class AvoidanceType : uint8
{
	RVO,
	ORCA
};


UCLASS(Blueprintable)
class AORCAvsRVOCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AORCAvsRVOCharacter();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual FVector GetPosition() const;
	virtual FVector GetVelocity() const;
	FVector2D GetPosition2D() const;
	FVector2D GetVelocity2D() const;
	FVector2D GetDestinationPos2D() const;
	virtual float GetRadius() const;

	//COMPONENT
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AvoidanceComponent")
	UCollisionAvoidenceComponent* m_AvoidanceComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AgentInfo")
	float m_AgentRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AgentInfo")
	AActor* m_DestinationActor;

	UFUNCTION(BlueprintCallable, Category = "CollisionAvoidence")
	void CalculateVelocityObject();

	UFUNCTION(BlueprintCallable, Category = "CollisionAvoidence")
	void OnBeginOverlap(AActor* OtherActor);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AgentInfo")
	bool m_TrackComputationSpeed = false;

	UFUNCTION(BlueprintCallable, Category = "PerformanceTrace")
	void StartPerformanceTrace();

	//for experiment
	int m_HitCount{};
	TArray<long long> m_ComputationTimes;

	int m_TraceAmount;
	int m_MaxTraceAmount{10};
	bool m_StartTrackingComputation;

	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CollisionAvoidence")
	AvoidanceType m_AvoidenceType{ AvoidanceType::RVO };


private:

	//members
	TArray<AORCAvsRVOCharacter*> m_NeighbouringAgents;

	

	


	//methods
	void InitializeNeighbours();
	void PrintTotalCollisionCount();

	
	
};

