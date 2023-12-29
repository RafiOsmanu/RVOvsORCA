// Fill out your copyright notice in the Description page of Project Settings.


#include "AgentManager.h"

UAgentManager* UAgentManager::GetInstance()
{
    static UAgentManager* Instance = nullptr;

    if (!Instance)
    {
        Instance = NewObject<UAgentManager>(GetTransientPackage(), UAgentManager::StaticClass());
        Instance->AddToRoot(); // Ensure the singleton is not garbage collected
    }

    return Instance;
}

void UAgentManager::RegisterAgent(const TScriptInterface<IAgentInterface>& agent)
{
    m_Agents.Add(agent);
}
