using UnrealBuildTool;
public class ShooterSettings : ModuleRules {
 public ShooterSettings(ReadOnlyTargetRules Target) : base(Target) {
  PCHUsage=PCHUsageMode.UseExplicitOrSharedPCHs;
  PublicDependencyModuleNames.AddRange(new[]{"Core","CoreUObject","Engine","UMG","Slate","SlateCore","InputCore","EnhancedInput"});
 }
}
