#include "ShooterSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ScrollBox.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/InputKeySelector.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"

void USettingsCallback::Click(){if(Menu)Menu->Click(Kind,Id);}
void USettingsCallback::FloatChanged(float V){if(Menu)Menu->ChangeFloat(Id,V);}
void USettingsCallback::CheckChanged(bool V){if(Menu)Menu->ChangeCheck(V);}
void USettingsCallback::OptionChanged(FString V,ESelectInfo::Type T){if(Menu)Menu->ChangeOption(Id,V);}
void USettingsCallback::KeyChanged(FInputChord C){if(Menu)Menu->ChangeKey(Id,C);}
UShooterSettingsSubsystem* USettingsMenu::Settings() const{return GetGameInstance()?GetGameInstance()->GetSubsystem<UShooterSettingsSubsystem>():nullptr;}
USettingsCallback* USettingsMenu::Callback(const FString& Kind,int32 Id){auto* C=NewObject<USettingsCallback>(this);C->Menu=this;C->Kind=Kind;C->Id=Id;Callbacks.Add(C);return C;}
UTextBlock* USettingsMenu::Text(const FString& Value,int32 Size){auto* T=WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Value));auto F=T->GetFont();F.Size=Size;T->SetFont(F);T->SetColorAndOpacity(FSlateColor(FLinearColor(.94f,.96f,1.f)));return T;}
UButton* USettingsMenu::Button(const FString& Label,const FString& Kind,int32 Id){auto* B=WidgetTree->ConstructWidget<UButton>();B->SetBackgroundColor(FLinearColor(.12f,.19f,.27f));auto* T=Text(Label,22);auto* S=CastChecked<UButtonSlot>(B->AddChild(T));S->SetPadding(FMargin(16,10));S->SetHorizontalAlignment(HAlign_Center);S->SetVerticalAlignment(VAlign_Center);B->OnClicked.AddDynamic(Callback(Kind,Id),&USettingsCallback::Click);return B;}
UHorizontalBox* USettingsMenu::Row(UVerticalBox* Parent,const FString& Label){auto* H=WidgetTree->ConstructWidget<UHorizontalBox>();auto* Box=WidgetTree->ConstructWidget<USizeBox>();Box->SetHeightOverride(48);Box->AddChild(H);auto* S=Parent->AddChildToVerticalBox(Box);S->SetPadding(FMargin(8,2));auto* Name=WidgetTree->ConstructWidget<USizeBox>();Name->SetWidthOverride(360);Name->AddChild(Text(Label));H->AddChildToHorizontalBox(Name)->SetVerticalAlignment(VAlign_Center);return H;}
TSharedRef<SWidget> USettingsMenu::RebuildWidget(){Build();return Super::RebuildWidget();}
void USettingsMenu::Build(){
 if(!WidgetTree)WidgetTree=NewObject<UWidgetTree>(this,TEXT("WidgetTree"));Callbacks.Empty();Readouts.Empty();Sliders.Empty();KeySelectors.Empty();TabButtons.Empty();VideoCombos.Empty();
 auto* Root=WidgetTree->ConstructWidget<UOverlay>();WidgetTree->RootWidget=Root;
 auto* Shade=WidgetTree->ConstructWidget<UBorder>();Shade->SetBrushColor(FLinearColor(.015f,.022f,.034f,.98f));auto* BG=Root->AddChildToOverlay(Shade);BG->SetHorizontalAlignment(HAlign_Fill);BG->SetVerticalAlignment(VAlign_Fill);
 auto* Scale=WidgetTree->ConstructWidget<UScaleBox>();Scale->SetStretch(EStretch::ScaleToFit);auto* SS=Root->AddChildToOverlay(Scale);SS->SetHorizontalAlignment(HAlign_Fill);SS->SetVerticalAlignment(VAlign_Fill);
 auto* Frame=WidgetTree->ConstructWidget<USizeBox>();Frame->SetWidthOverride(1120);Frame->SetHeightOverride(760);Scale->AddChild(Frame);
 auto* FramePadding=WidgetTree->ConstructWidget<UBorder>();FramePadding->SetBrushColor(FLinearColor::Transparent);FramePadding->SetPadding(FMargin(40,24));Frame->AddChild(FramePadding);
 auto* Layout=WidgetTree->ConstructWidget<UVerticalBox>();FramePadding->AddChild(Layout);
 auto* Title=Text(TEXT("SETTINGS"),38);Title->SetJustification(ETextJustify::Center);Layout->AddChildToVerticalBox(Title)->SetPadding(FMargin(0,0,0,18));
 auto* Tabs=WidgetTree->ConstructWidget<UHorizontalBox>();Layout->AddChildToVerticalBox(Tabs)->SetPadding(FMargin(0,0,0,16));
 for(int32 I=0;I<3;++I){auto* B=Button(I==0?TEXT("Video"):I==1?TEXT("Gameplay"):TEXT("Controls"),TEXT("Tab"),I);TabButtons.Add(B);auto* S=Tabs->AddChildToHorizontalBox(B);S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));S->SetPadding(FMargin(4,0));}
 Pages=WidgetTree->ConstructWidget<UWidgetSwitcher>();Layout->AddChildToVerticalBox(Pages)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
 for(int32 Page=0;Page<3;++Page){
  auto* PageLayout=WidgetTree->ConstructWidget<UVerticalBox>();Pages->AddChild(PageLayout);
  auto* Scroll=WidgetTree->ConstructWidget<UScrollBox>();PageLayout->AddChildToVerticalBox(Scroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));auto* Rows=WidgetTree->ConstructWidget<UVerticalBox>();Scroll->AddChild(Rows);
  if(Page==0){
   const TCHAR* Names[]={TEXT("Window Mode"),TEXT("Resolution"),TEXT("Shadow Quality"),TEXT("Texture Quality"),TEXT("Shader Quality"),TEXT("Anti Aliasing"),TEXT("VSync"),TEXT("Frame Rate Cap")};
   for(int32 I=0;I<8;++I){auto* R=Row(Rows,Names[I]);auto* C=WidgetTree->ConstructWidget<UComboBoxString>();VideoCombos.Add(C);
    if(I==0){for(const auto* O:{TEXT("Fullscreen"),TEXT("Borderless"),TEXT("Windowed")})C->AddOption(O);}
    else if(I==1){TArray<FIntPoint> Modes;UKismetSystemLibrary::GetSupportedFullscreenResolutions(Modes);for(auto R0:{FIntPoint(1280,720),FIntPoint(1600,900),FIntPoint(1920,1080)})Modes.AddUnique(R0);Modes.Sort([](FIntPoint A,FIntPoint B){return A.X*A.Y<B.X*B.Y;});for(auto M:Modes)if(M.X>=800&&M.Y>=600&&M.X<=1920&&M.Y<=1080)C->AddOption(FString::Printf(TEXT("%d x %d"),M.X,M.Y));}
    else if(I<6){for(const auto* O:{TEXT("Low"),TEXT("Medium"),TEXT("High"),TEXT("Epic"),TEXT("Cinematic")})C->AddOption(O);}
    else if(I==6){C->AddOption(TEXT("Off"));C->AddOption(TEXT("On"));}
    else{for(const auto* O:{TEXT("Unlimited"),TEXT("30"),TEXT("60"),TEXT("90"),TEXT("120"),TEXT("144"),TEXT("165"),TEXT("180"),TEXT("240")})C->AddOption(O);}
    C->OnSelectionChanged.AddDynamic(Callback(TEXT("Video"),I),&USettingsCallback::OptionChanged);R->AddChildToHorizontalBox(C)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
   }
  }else if(Page==1){
   const TCHAR* Labels[]={TEXT("Master Volume"),TEXT("Mouse Sensitivity"),TEXT("Field of View")};
   for(int32 I=0;I<3;++I){auto* R=Row(Rows,Labels[I]);auto* Slider=WidgetTree->ConstructWidget<USlider>();Sliders.Add(Slider);Slider->SetMinValue(I==0?0:I==1?.1f:60);Slider->SetMaxValue(I==0?1:I==1?3:120);Slider->SetStepSize(I==0?.01f:I==1?.05f:1);Slider->OnValueChanged.AddDynamic(Callback(TEXT("Slider"),I),&USettingsCallback::FloatChanged);auto* S=R->AddChildToHorizontalBox(Slider);S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));S->SetVerticalAlignment(VAlign_Center);S->SetPadding(FMargin(10,0,20,0));auto* Box=WidgetTree->ConstructWidget<USizeBox>();Box->SetWidthOverride(100);auto* Value=Text(TEXT(""));Value->SetJustification(ETextJustify::Right);Readouts.Add(Value);Box->AddChild(Value);R->AddChildToHorizontalBox(Box)->SetVerticalAlignment(VAlign_Center);}
   auto* R=Row(Rows,TEXT("Invert Y-axis"));InvertCheck=WidgetTree->ConstructWidget<UCheckBox>();InvertCheck->OnCheckStateChanged.AddDynamic(Callback(TEXT("Invert"),0),&USettingsCallback::CheckChanged);R->AddChildToHorizontalBox(InvertCheck)->SetVerticalAlignment(VAlign_Center);
   Rows->AddChildToVerticalBox(Text(TEXT("Changes take effect when you select Apply Settings."),18))->SetPadding(FMargin(8,24));
  }else{
   Rows->AddChildToVerticalBox(Text(TEXT("Select a binding, then press a key or mouse button. Esc cancels."),18))->SetPadding(FMargin(8,4,8,12));
   const auto& Bindings=UShooterSettingsSubsystem::Bindings();
   for(int32 I=0;I<Bindings.Num();++I){auto* R=Row(Rows,Bindings[I].Label);auto* K=WidgetTree->ConstructWidget<UInputKeySelector>();K->SetAllowGamepadKeys(false);K->SetAllowModifierKeys(false);K->SetKeySelectionText(FText::FromString(TEXT("Press a key...")));K->OnKeySelected.AddDynamic(Callback(TEXT("Key"),I),&USettingsCallback::KeyChanged);KeySelectors.Add(K);R->AddChildToHorizontalBox(K)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));}
   Rows->AddChildToVerticalBox(Text(TEXT("Pause / resume: Esc (reserved). Gamepad bindings are preserved."),18))->SetPadding(FMargin(8,12));
  }
  auto* Footer=WidgetTree->ConstructWidget<UHorizontalBox>();PageLayout->AddChildToVerticalBox(Footer)->SetPadding(FMargin(0,18,0,8));
  for(int32 I=0;I<2;++I){auto* B=Button(I?TEXT("Apply Settings"):TEXT("Reset to Defaults"),I?TEXT("Apply"):TEXT("Reset"),Page);auto* S=Footer->AddChildToHorizontalBox(B);S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));S->SetPadding(FMargin(4,0));}
 }
 Status=Text(TEXT(""),18);Status->SetJustification(ETextJustify::Center);Layout->AddChildToVerticalBox(Status)->SetPadding(FMargin(0,8));
 auto* NavigationRow=WidgetTree->ConstructWidget<UHorizontalBox>(); Layout->AddChildToVerticalBox(NavigationRow)->SetHorizontalAlignment(HAlign_Center);
 auto* CloseButton=Button(TEXT("Back"),TEXT("Close"),0); CloseLabel=Cast<UTextBlock>(CloseButton->GetContent()); NavigationRow->AddChildToHorizontalBox(CloseButton)->SetPadding(FMargin(4,0));
 MainMenuButton=Button(TEXT("Return to Main Menu"),TEXT("MainMenu"),0); NavigationRow->AddChildToHorizontalBox(MainMenuButton)->SetPadding(FMargin(4,0));
 SetIsFocusable(true);
}
void USettingsMenu::NativeConstruct(){
 Super::NativeConstruct();bPause=UGameplayStatics::IsGamePaused(this);
 MainMenuButton->SetVisibility(bPause?ESlateVisibility::Visible:ESlateVisibility::Collapsed); CloseLabel->SetText(FText::FromString(bPause?TEXT("Resume Game"):TEXT("Back")));
 if(auto* S=Settings())Draft=DuplicateObject<UShooterPreferences>(S->Preferences,this);else Draft=NewObject<UShooterPreferences>(this);
 if(auto* G=UGameUserSettings::GetGameUserSettings()){WindowMode=(int32)G->GetFullscreenMode();Resolution=G->GetScreenResolution();if(Resolution.X>1920||Resolution.Y>1080)Resolution=FIntPoint(1920,1080);Quality[0]=G->GetShadowQuality();Quality[1]=G->GetTextureQuality();Quality[2]=G->GetShadingQuality();Quality[3]=G->GetAntiAliasingQuality();VSync=G->IsVSyncEnabled();FrameCap=G->GetFrameRateLimit();}
 Refresh();Click(TEXT("Tab"),0);
 if(auto* PC=GetOwningPlayer()?GetOwningPlayer():UGameplayStatics::GetPlayerController(this,0)){FInputModeUIOnly Mode;Mode.SetWidgetToFocus(TakeWidget());Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);PC->SetInputMode(Mode);PC->bShowMouseCursor=true;SetUserFocus(PC);}
}
void USettingsMenu::Refresh(){
 if(!Draft)return;bRefreshing=true;
 VideoCombos[0]->SetSelectedIndex(WindowMode);
 FString R=FString::Printf(TEXT("%d x %d"),Resolution.X,Resolution.Y);if(VideoCombos[1]->FindOptionIndex(R)==INDEX_NONE)VideoCombos[1]->AddOption(R);VideoCombos[1]->SetSelectedOption(R);
 for(int32 I=0;I<4;++I)VideoCombos[I+2]->SetSelectedIndex(Quality[I]);VideoCombos[6]->SetSelectedIndex(VSync?1:0);
 FString Cap=FrameCap<=0?TEXT("Unlimited"):FString::FromInt(FMath::RoundToInt(FrameCap));if(VideoCombos[7]->FindOptionIndex(Cap)==INDEX_NONE)VideoCombos[7]->AddOption(Cap);VideoCombos[7]->SetSelectedOption(Cap);
 Sliders[0]->SetValue(Draft->MasterVolume);Sliders[1]->SetValue(Draft->Sensitivity);Sliders[2]->SetValue(Draft->FieldOfView);InvertCheck->SetIsChecked(Draft->InvertY);
 Readouts[0]->SetText(FText::FromString(FString::Printf(TEXT("%d%%"),FMath::RoundToInt(Draft->MasterVolume*100))));Readouts[1]->SetText(FText::FromString(FString::Printf(TEXT("%.2fx"),Draft->Sensitivity)));Readouts[2]->SetText(FText::FromString(FString::Printf(TEXT("%d°"),FMath::RoundToInt(Draft->FieldOfView))));
 const auto& B=UShooterSettingsSubsystem::Bindings();for(int32 I=0;I<KeySelectors.Num();++I)KeySelectors[I]->SetSelectedKey(FInputChord(Draft->Keys.Contains(B[I].Id)?Draft->Keys[B[I].Id]:B[I].DefaultKey));bRefreshing=false;
}
void USettingsMenu::Click(const FString& Kind,int32 Id){if(Kind==TEXT("Tab")){ActiveTab=Id;Pages->SetActiveWidgetIndex(Id);for(int I=0;I<TabButtons.Num();++I)TabButtons[I]->SetBackgroundColor(I==Id?FLinearColor(.12f,.43f,.62f):FLinearColor(.12f,.19f,.27f));}else if(Kind==TEXT("Apply"))Apply();else if(Kind==TEXT("Reset"))Reset();else if(Kind==TEXT("Close"))Close();else if(Kind==TEXT("MainMenu"))ReturnToMainMenu();}
void USettingsMenu::ChangeFloat(int32 Id,float V){if(bRefreshing||!Draft)return;if(Id==0)Draft->MasterVolume=V;else if(Id==1)Draft->Sensitivity=V;else Draft->FieldOfView=FMath::RoundToFloat(V);Refresh();Status->SetText(FText::FromString(TEXT("Unsaved changes — select Apply Settings.")));}
void USettingsMenu::ChangeCheck(bool V){if(bRefreshing||!Draft)return;Draft->InvertY=V;Status->SetText(FText::FromString(TEXT("Unsaved changes — select Apply Settings.")));}
void USettingsMenu::ChangeOption(int32 Id,const FString& V){if(bRefreshing)return;if(Id==0)WindowMode=VideoCombos[Id]->FindOptionIndex(V);else if(Id==1){FString X,Y;if(V.Split(TEXT(" x "),&X,&Y))Resolution=FIntPoint(FCString::Atoi(*X),FCString::Atoi(*Y));}else if(Id<6)Quality[Id-2]=VideoCombos[Id]->FindOptionIndex(V);else if(Id==6)VSync=V==TEXT("On");else FrameCap=V==TEXT("Unlimited")?0:FCString::Atof(*V);Status->SetText(FText::FromString(TEXT("Unsaved changes — select Apply Settings.")));}
void USettingsMenu::ChangeKey(int32 Id,FInputChord Chord){
 if(bRefreshing||!Draft)return;const auto& B=UShooterSettingsSubsystem::Bindings();FKey K=Chord.Key;
 if(!K.IsValid()||K==EKeys::Escape||K.IsGamepadKey()||K.IsAxis1D()||K.IsAxis2D()||K.IsAxis3D()||Chord.bCtrl||Chord.bAlt||Chord.bShift||Chord.bCmd){Status->SetText(FText::FromString(TEXT("Choose a single keyboard key or mouse button; Esc is reserved.")));Refresh();return;}
 // Reject duplicates rather than silently changing another action.
 for(int I=0;I<B.Num();++I)if(I!=Id&&Draft->Keys.FindRef(B[I].Id)==K){Status->SetText(FText::FromString(TEXT("That key is already assigned to ")+B[I].Label+TEXT(".")));Refresh();return;}
 if(K==EKeys::Up||K==EKeys::Down||K==EKeys::Left||K==EKeys::Right){Status->SetText(FText::FromString(TEXT("Arrow keys are reserved for alternate movement.")));Refresh();return;}
 Draft->Keys.Add(B[Id].Id,K);Status->SetText(FText::FromString(TEXT("Binding changed — select Apply Settings.")));
}
void USettingsMenu::Apply(){
 auto* S=Settings();if(!S||!Draft)return;
 if(ActiveTab==0){auto* G=UGameUserSettings::GetGameUserSettings();if(!G)return;G->SetFullscreenMode((EWindowMode::Type)WindowMode);G->SetScreenResolution(Resolution);G->SetShadowQuality(Quality[0]);G->SetTextureQuality(Quality[1]);G->SetShadingQuality(Quality[2]);G->SetAntiAliasingQuality(Quality[3]);G->SetVSyncEnabled(VSync);G->SetFrameRateLimit(FrameCap);G->ApplySettings(false);G->ConfirmVideoMode();G->SaveSettings();}
 else if(ActiveTab==1){S->Preferences->MasterVolume=Draft->MasterVolume;S->Preferences->Sensitivity=Draft->Sensitivity;S->Preferences->FieldOfView=Draft->FieldOfView;S->Preferences->InvertY=Draft->InvertY;S->ApplyGameplay();}
 else{S->Preferences->Keys=Draft->Keys;S->ApplyBindings();}
 Status->SetText(FText::FromString(S->Save()?TEXT("Settings applied and saved."):TEXT("Applied, but saving failed. Check available disk space.")));
}
void USettingsMenu::Reset(){
 if(!Draft)return;if(ActiveTab==0){WindowMode=2;Resolution=FIntPoint(1280,720);for(auto& Q:Quality)Q=0;VSync=false;FrameCap=0;}
 else if(ActiveTab==1){Draft->MasterVolume=1;Draft->Sensitivity=1;Draft->FieldOfView=90;Draft->InvertY=false;}
 else{Draft->Keys.Empty();for(const auto& B:UShooterSettingsSubsystem::Bindings())Draft->Keys.Add(B.Id,B.DefaultKey);}
 Refresh();Status->SetText(FText::FromString(TEXT("Defaults restored for this tab — select Apply Settings to save.")));
}
FReply USettingsMenu::NativeOnKeyDown(const FGeometry& G,const FKeyEvent& E){if(E.GetKey()==EKeys::Escape){Close();return FReply::Handled();}return Super::NativeOnKeyDown(G,E);}
void USettingsMenu::Close(){
 if(bPause){if(auto* S=Settings())S->ClosePause();else RemoveFromParent();return;}
 auto* PC=GetOwningPlayer()?GetOwningPlayer():UGameplayStatics::GetPlayerController(this,0);
 if(PC){auto* Class=LoadClass<UUserWidget>(nullptr,TEXT("/Game/Game/UI/MainMenu/BP_MainMenu.BP_MainMenu_C"));if(Class){auto* Menu=CreateWidget<UUserWidget>(PC,Class);if(Menu)Menu->AddToViewport();}}
 RemoveFromParent();
}


void USettingsMenu::ReturnToMainMenu(){
 if(!bPause)return;
 if(auto* S=Settings())S->ClosePause();
 UGameplayStatics::SetGamePaused(this,false);
 UWidgetLayoutLibrary::RemoveAllWidgets(this);
 UGameplayStatics::OpenLevel(this,FName(TEXT("/Game/Game/UI/MainMenu/UI_MainMenu")));
}
