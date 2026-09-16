#include "ShooterSettings.h"
#include "Modules/ModuleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/InputSettings.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "UObject/UnrealType.h"
#include "UObject/Package.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

IMPLEMENT_MODULE(FDefaultModuleImpl,ShooterSettings)
static const TCHAR* PreferenceSlot=TEXT("ShooterSettingsV2");

const TArray<FShooterBinding>& UShooterSettingsSubsystem::Bindings(){
 static TArray<FShooterBinding> Items={
  {"Forward","Move forward",EKeys::W},{"Backward","Move backward",EKeys::S},
  {"Left","Move left",EKeys::A},{"Right","Move right",EKeys::D},
  {"Jump","Jump",EKeys::SpaceBar},{"Sprint","Sprint",EKeys::LeftShift},
  {"Crouch","Crouch",EKeys::LeftControl},{"Interact","Interact",EKeys::E},
  {"Flashlight","Flashlight",EKeys::F},{"Fire","Fire weapon",EKeys::LeftMouseButton}};
 return Items;
}
FName UShooterSettingsSubsystem::BindingForKey(FKey Key){for(const auto& B:Bindings())if(B.DefaultKey==Key)return B.Id;return NAME_None;}
void UShooterSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection){
 Super::Initialize(Collection);
 Preferences=Cast<UShooterPreferences>(UGameplayStatics::LoadGameFromSlot(PreferenceSlot,0));
 if(!Preferences){
  Preferences=NewObject<UShooterPreferences>(this);
  // Migrate the established volume/sensitivity save without modifying it.
  if(auto* Old=UGameplayStatics::LoadGameFromSlot(TEXT("UserPreferences"),0)){
   if(auto* P=FindFProperty<FFloatProperty>(Old->GetClass(),TEXT("MasterVolume")))Preferences->MasterVolume=P->GetPropertyValue_InContainer(Old);
   if(auto* P=FindFProperty<FFloatProperty>(Old->GetClass(),TEXT("MouseSensitivity")))Preferences->Sensitivity=P->GetPropertyValue_InContainer(Old);
  }
 }
 Preferences->MasterVolume=FMath::Clamp(Preferences->MasterVolume,0.f,1.f);
 Preferences->Sensitivity=FMath::Clamp(Preferences->Sensitivity,.1f,3.f);
 Preferences->FieldOfView=FMath::Clamp(Preferences->FieldOfView,60.f,120.f);
 for(const auto& B:Bindings())if(!Preferences->Keys.Contains(B.Id)||!Preferences->Keys[B.Id].IsValid())Preferences->Keys.Add(B.Id,B.DefaultKey);
 if(!Preferences->bVideoDefaultsInitialized){
  if(auto* Settings=UGameUserSettings::GetGameUserSettings()){
   Settings->SetOverallScalabilityLevel(0);
   Settings->SetScreenResolution(FIntPoint(1280,720));
   Settings->SetFullscreenMode(EWindowMode::Windowed);
   Settings->SetVSyncEnabled(false);
   Settings->ApplySettings(false);
   Settings->ConfirmVideoMode();
   Settings->SaveSettings();
  }
  Preferences->bVideoDefaultsInitialized=true;
  Save();
 }
 bInitialized=true;
}
void UShooterSettingsSubsystem::Deinitialize(){bInitialized=false;PauseMenu=nullptr;ContextCopies.Empty();Super::Deinitialize();}
UWorld* UShooterSettingsSubsystem::GetWorld() const{return GetGameInstance()?GetGameInstance()->GetWorld():nullptr;}
bool UShooterSettingsSubsystem::IsTickable() const{return bInitialized&&!IsTemplate()&&GetWorld()&&GetWorld()->IsGameWorld();}
TStatId UShooterSettingsSubsystem::GetStatId() const{RETURN_QUICK_DECLARE_CYCLE_STAT(ShooterSettings,STATGROUP_Tickables);}
bool UShooterSettingsSubsystem::Save(){return UGameplayStatics::SaveGameToSlot(Preferences,PreferenceSlot,0);}
void UShooterSettingsSubsystem::Tick(float DeltaTime){
 auto* PC=GetWorld()?GetWorld()->GetFirstPlayerController():nullptr;
 if(!PC||!PC->IsLocalController())return;
 if(CurrentController.Get()!=PC){CurrentController=PC;CurrentPawn=nullptr;BoundInput=nullptr;ContextCopies.Empty();ApplyGameplay();ApplyBindings();}
 if(PC->InputComponent&&BoundInput.Get()!=PC->InputComponent){
  BoundInput=PC->InputComponent;auto& B=PC->InputComponent->BindKey(EKeys::Escape,IE_Pressed,this,&UShooterSettingsSubsystem::TogglePause);B.bExecuteWhenPaused=true;B.bConsumeInput=true;
 }
 if(CurrentPawn.Get()!=PC->GetPawn()){CurrentPawn=PC->GetPawn();ApplyGameplay();ApplyBindings();}
 UpdateContexts();
}
void UShooterSettingsSubsystem::ApplyGameplay(){
 if(!Preferences||!GetWorld())return;
 auto* Mix=LoadObject<USoundMix>(nullptr,TEXT("/Game/Game/Settings/SM_MasterSettings.SM_MasterSettings"));
 auto* Master=LoadObject<USoundClass>(nullptr,TEXT("/Engine/EngineSounds/Master.Master"));
 if(Mix&&Master){UGameplayStatics::SetSoundMixClassOverride(this,Mix,Master,Preferences->MasterVolume,1.f,0.f,true);UGameplayStatics::SetBaseSoundMix(this,Mix);}
 // Keep the existing GameInstance's startup hooks consistent with the new save.
 auto* GI=GetGameInstance();
 if(auto* P=FindFProperty<FObjectProperty>(GI->GetClass(),TEXT("Preferences")))if(auto* Old=P->GetObjectPropertyValue_InContainer(GI)){
  if(auto* V=FindFProperty<FFloatProperty>(Old->GetClass(),TEXT("MasterVolume")))V->SetPropertyValue_InContainer(Old,Preferences->MasterVolume);
  if(auto* V=FindFProperty<FFloatProperty>(Old->GetClass(),TEXT("MouseSensitivity")))V->SetPropertyValue_InContainer(Old,Preferences->Sensitivity);
 }
 if(auto* PC=GetWorld()->GetFirstPlayerController())if(auto* Pawn=PC->GetPawn()){
  TArray<UCameraComponent*> Cameras;Pawn->GetComponents(Cameras);for(auto* Camera:Cameras)Camera->SetFieldOfView(Preferences->FieldOfView);
 }
}
void UShooterSettingsSubsystem::UpdateContexts(){
 auto* PC=CurrentController.Get();auto* LP=PC?PC->GetLocalPlayer():nullptr;
 auto* Input=LP?LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>():nullptr;if(!Input||!Input->GetPlayerInput())return;
 
 for(const TCHAR* Path:{TEXT("/Game/Game/Characters/Player/IMC_Default.IMC_Default"),TEXT("/Game/Game/Characters/Player/IMC_Weapons.IMC_Weapons"),TEXT("/Game/Game/Characters/Player/IMC_MouseLook.IMC_MouseLook")}){
  auto* Original=LoadObject<UInputMappingContext>(nullptr,Path);int32 Priority=0;if(!Original||!Input->HasMappingContext(Original,Priority))continue;
  // Only replace this game's contexts, preserving their priority and all triggers/modifiers.
  if(!Original->GetPathName().StartsWith(TEXT("/Game/Game/Characters/Player/")))continue;
  UInputMappingContext* Copy=ContextCopies.FindRef(Original);
  if(!Copy){Copy=DuplicateObject<UInputMappingContext>(Original,GetTransientPackage());ContextCopies.Add(Original,Copy);
   for(int32 I=0;I<Copy->GetMappings().Num();++I){auto& Mapping=Copy->GetMapping(I);const FName Id=BindingForKey(Mapping.Key);if(Id!=NAME_None)Mapping.Key=Preferences->Keys.FindRef(Id);
    if(Mapping.Key==EKeys::Mouse2D){Mapping.Modifiers.RemoveAll([](const auto& M){return M&&M->GetClass()->GetName().Contains(TEXT("MouseSensitivityModifier"));});Mapping.Modifiers.Add(NewObject<UShooterLookModifier>(Copy));}
   }
  }
  Input->RemoveMappingContext(Original);Input->AddMappingContext(Copy,Priority);
 }
}
void UShooterSettingsSubsystem::ApplyBindings(){
 auto* PC=CurrentController.Get();if(!PC)return;
 if(auto* LP=PC->GetLocalPlayer())if(auto* Input=LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()){
  
  for(const auto& Pair:ContextCopies){int32 Priority=0;if(Input->HasMappingContext(Pair.Value,Priority)) {Input->RemoveMappingContext(Pair.Value);Input->AddMappingContext(Pair.Key,Priority);}}
  ContextCopies.Empty();UpdateContexts();
 }
 // FireWeapon is a legacy action in this project. Override only this local player's mapping.
 if(PC->PlayerInput){
  auto Maps=PC->PlayerInput->ActionMappings;for(const auto& M:Maps)if(M.ActionName==TEXT("FireWeapon")&&!M.Key.IsGamepadKey())PC->PlayerInput->RemoveActionMapping(M);
  PC->PlayerInput->AddActionMapping(FInputActionKeyMapping(TEXT("FireWeapon"),Preferences->Keys.FindRef(TEXT("Fire"))));PC->PlayerInput->ForceRebuildingKeyMaps(false);
 }
}
void UShooterSettingsSubsystem::TogglePause(){
 if(PauseMenu&&PauseMenu->IsInViewport()){ClosePause();return;}
 auto* PC=CurrentController.Get();if(!PC||!PC->GetPawn()||UGameplayStatics::IsGamePaused(this))return;
 // Menu maps do not own gameplay pause. Existing modal widgets also retain control.
 TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this,Widgets,UUserWidget::StaticClass(),true);
 for(auto* W:Widgets)if(W->IsVisible()&&(W->GetClass()->GetName().Contains(TEXT("MainMenu"))||W->GetClass()->GetName().Contains(TEXT("GameOver"))||W->GetClass()->GetName().Contains(TEXT("Upgrade"))||W->IsA<USettingsMenu>()))return;
 auto* Class=LoadClass<UUserWidget>(nullptr,TEXT("/Game/Game/UI/MainMenu/BP_Settings.BP_Settings_C"));if(!Class)return;
 UGameplayStatics::SetGamePaused(this,true);PauseMenu=CreateWidget<USettingsMenu>(PC,Class);if(PauseMenu)PauseMenu->AddToViewport(100);else UGameplayStatics::SetGamePaused(this,false);
}
void UShooterSettingsSubsystem::ClosePause(){
 if(PauseMenu){PauseMenu->RemoveFromParent();PauseMenu=nullptr;}UGameplayStatics::SetGamePaused(this,false);
 if(auto* PC=CurrentController.Get()){PC->SetInputMode(FInputModeGameOnly());PC->bShowMouseCursor=false;PC->FlushPressedKeys();}
}
FInputActionValue UShooterLookModifier::ModifyRaw_Implementation(const UEnhancedPlayerInput* Input,FInputActionValue Value,float DeltaTime){
 auto* PC=Input?Cast<APlayerController>(Input->GetOuter()):nullptr;auto* GI=PC?PC->GetGameInstance():nullptr;auto* S=GI?GI->GetSubsystem<UShooterSettingsSubsystem>():nullptr;
 if(S&&S->Preferences){auto V=Value.Get<FVector>();V.X*=S->Preferences->Sensitivity;V.Y*=S->Preferences->Sensitivity*(S->Preferences->InvertY?-1.f:1.f);return FInputActionValue(Value.GetValueType(),V);}return Value;
}


