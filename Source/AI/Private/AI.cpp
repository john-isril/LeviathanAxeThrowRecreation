#include "AI.h"

DEFINE_LOG_CATEGORY(AI);

#define LOCTEXT_NAMESPACE "FAI"

void FAI::StartupModule()
{
	UE_LOG(AI, Warning, TEXT("AI module has been loaded"));
}

void FAI::ShutdownModule()
{
	UE_LOG(AI, Warning, TEXT("AI module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAI, AI)