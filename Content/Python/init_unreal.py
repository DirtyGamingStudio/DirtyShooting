import unreal

asset_path = '/Game/Game/Cinematics/MC_ParkFlythrough'
package_path, asset_name = asset_path.rsplit('/', 1)
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
config = unreal.load_asset(asset_path)
if config is None:
    config_class = unreal.load_class(None, '/Script/MovieRenderPipelineCore.MoviePipelinePrimaryConfig')
    factory_class = unreal.load_class(None, '/Script/MovieRenderPipelineEditor.MoviePipelinePrimaryConfigFactory')
    config = asset_tools.create_asset(
        asset_name, package_path, config_class, factory_class())

config.initialize_transient_settings()
output_class = unreal.load_class(None, '/Script/MovieRenderPipelineCore.MoviePipelineOutputSetting')
png_class = unreal.load_class(None, '/Script/MovieRenderPipelineRenderPasses.MoviePipelineImageSequenceOutput_PNG')
output = config.find_or_add_setting_by_class(output_class)
output.output_resolution = unreal.IntPoint(1280, 720)
output.output_directory = unreal.DirectoryPath(
    'C:/Users/bpasc/Videos/ParkMenuFrames')
output.file_name_format = 'ParkMenu_{frame_number}'
output.use_custom_playback_range = True
output.custom_start_frame = 0
output.custom_end_frame = 960
config.find_or_add_setting_by_class(png_class)
unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)
unreal.log('Park movie render configuration saved.')
