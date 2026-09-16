#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Blueprint/UserWidget.h"
#include "InputModifiers.h"
#include "Components/ComboBoxString.h"
#include "ShooterSettings.generated.h"

class UInputMappingContext;
class UInputComponent;
class UTextBlock;
class UVerticalBox;
class UHorizontalBox;
class UWidgetSwitcher;
class UButton;
class UInputKeySelector;
class USettingsMenu;

UCLASS()
class SHOOTERSETTINGS_API UShooterPreferences : public USaveGame {
 GENERATED_BODY()
public:
 UPROPERTY(SaveGame) float MasterVolume=1.f;
 UPROPERTY(SaveGame) float Sensitivity=1.f;
 UPROPERTY(SaveGame) float FieldOfView=90.f;
 UPROPERTY(SaveGame) bool InvertY=false;
 UPROPERTY(SaveGame) bool bVideoDefaultsInitialized=false;
 UPROPERTY(SaveGame) TMap<FName,FKey> Keys;
};

struct FShooterBinding { FName Id; FString Label; FKey DefaultKey; };

UCLASS()
class SHOOTERSETTINGS_API UShooterSettingsSubsystem : public UGameInstanceSubsystem, public FTickableGameObject {
 GENERATED_BODY()
public:
 virtual void Initialize(FSubsystemCollectionBase& Collection) override;
 virtual void Deinitialize() override;
 virtual UWorld* GetWorld() const override;
 virtual void Tick(float DeltaTime) override;
 virtual bool IsTickable() const override;
 virtual bool IsTickableWhenPaused() const override { return true; }
 virtual TStatId GetStatId() const override;
 virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
 UPROPERTY() TObjectPtr<UShooterPreferences> Preferences;
 UPROPERTY() TObjectPtr<USettingsMenu> PauseMenu;
 UPROPERTY() TMap<TObjectPtr<UInputMappingContext>,TObjectPtr<UInputMappingContext>> ContextCopies;
 static const TArray<FShooterBinding>& Bindings();
 static FName BindingForKey(FKey Key);
 void ApplyGameplay();
 void ApplyBindings();
 bool Save();
 UFUNCTION() void TogglePause();
 void ClosePause();
private:
 TWeakObjectPtr<APlayerController> CurrentController;
 TWeakObjectPtr<APawn> CurrentPawn;
 TWeakObjectPtr<UInputComponent> BoundInput;
 bool bInitialized=false;
 void UpdateContexts();
};

UCLASS()
class SHOOTERSETTINGS_API UShooterLookModifier : public UInputModifier {
 GENERATED_BODY()
 virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* Input, FInputActionValue Value,float DeltaTime) override;
};

UCLASS()
class SHOOTERSETTINGS_API USettingsCallback : public UObject {
 GENERATED_BODY()
public:
 UPROPERTY() TObjectPtr<USettingsMenu> Menu;
 int32 Id=0;
 FString Kind;
 UFUNCTION() void Click();
 UFUNCTION() void FloatChanged(float Value);
 UFUNCTION() void CheckChanged(bool Value);
 UFUNCTION() void OptionChanged(FString Value,ESelectInfo::Type SelectionType);
 UFUNCTION() void KeyChanged(FInputChord Chord);
};

UCLASS(Blueprintable)
class SHOOTERSETTINGS_API USettingsMenu : public UUserWidget {
 GENERATED_BODY()
public:
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual void NativeConstruct() override;
 // Compatibility with the existing main menu's initialization call.
 UFUNCTION(BlueprintCallable, Category="Settings") void OnStart() {}
 virtual FReply NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
 void Click(const FString& Kind,int32 Id);
 void ChangeFloat(int32 Id,float Value);
 void ChangeCheck(bool Value);
 void ChangeOption(int32 Id,const FString& Value);
 void ChangeKey(int32 Id,FInputChord Chord);
 void Close();
 void ReturnToMainMenu();
 UPROPERTY() TObjectPtr<UButton> MainMenuButton;
 UPROPERTY() TObjectPtr<UTextBlock> CloseLabel;
 UPROPERTY() TArray<TObjectPtr<USettingsCallback>> Callbacks;
 UPROPERTY() TObjectPtr<UWidgetSwitcher> Pages;
 UPROPERTY() TObjectPtr<UTextBlock> Status;
 UPROPERTY() TArray<TObjectPtr<UTextBlock>> Readouts;
 UPROPERTY() TArray<TObjectPtr<UInputKeySelector>> KeySelectors;
 UPROPERTY() TArray<TObjectPtr<UButton>> TabButtons;
 UPROPERTY() TObjectPtr<UShooterPreferences> Draft;
private:
 bool bPause=false;
 bool bRefreshing=false;
 int32 ActiveTab=0;
 int32 WindowMode=2;
 FIntPoint Resolution=FIntPoint(1280,720);
 int32 Quality[4]={0,0,0,0};
 bool VSync=false;
 float FrameCap=0;
 UPROPERTY() TArray<TObjectPtr<UComboBoxString>> VideoCombos;
 UPROPERTY() TArray<TObjectPtr<class USlider>> Sliders;
 UPROPERTY() TObjectPtr<class UCheckBox> InvertCheck;
 UShooterSettingsSubsystem* Settings() const;
 USettingsCallback* Callback(const FString& Kind,int32 Id);
 UTextBlock* Text(const FString& Value,int32 Size=24);
 UButton* Button(const FString& Label,const FString& Kind,int32 Id);
 UHorizontalBox* Row(UVerticalBox* Parent,const FString& Label);
 void Build();
 void Refresh();
 void Apply();
 void Reset();
};
