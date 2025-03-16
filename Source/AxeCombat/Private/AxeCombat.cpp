#include "AxeCombat.h"

DEFINE_LOG_CATEGORY(AxeCombat);

#define LOCTEXT_NAMESPACE "FAxeCombat"

void FAxeCombat::StartupModule()
{
	UE_LOG(AxeCombat, Warning, TEXT("AxeCombat module has been loaded"));
}

void FAxeCombat::ShutdownModule()
{
	UE_LOG(AxeCombat, Warning, TEXT("AxeCombat module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAxeCombat, AxeCombat)