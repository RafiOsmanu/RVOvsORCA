// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AgentInterface.h"
#include "Components/ActorComponent.h"
#include "CollisionAvoidenceComponent.generated.h"



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

	float m_MaxTimeRelavancy = 3.f;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void CalculateVelocityObject(const TArray<TScriptInterface<IAgentInterface>>& agents);

private:
	TArray<FVector> m_VelocityObject;

	
		
};
