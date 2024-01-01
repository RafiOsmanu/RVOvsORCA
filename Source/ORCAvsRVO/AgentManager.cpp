// Fill out your copyright notice in the Description page of Project Settings.


#include "AgentManager.h"

//static variable set to nullptr
//UAgentManager* UAgentManager::Instance = nullptr;

UAgentManager::UAgentManager()
{
    //Instance = this;
}

void UAgentManager::RegisterAgent(const AORCAvsRVOCharacter* agent)
{/*
    if (m_Agents.Contains(agent)) return;
    m_Agents.Add(agent);*/
}

UAgentManager* UAgentManager::GetInstance()
{
    return nullptr;
}
