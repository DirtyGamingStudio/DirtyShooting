import unreal


@unreal.uclass()
class ParkFlythroughExecutor(unreal.MoviePipelinePythonHostExecutor):
    active_pipeline = unreal.uproperty(unreal.MoviePipeline)

    def _post_init(self):
        self.active_pipeline = None

    @unreal.ufunction(override=True)
    def execute_delayed(self, unused_queue):
        tokens, switches, parameters = unreal.SystemLibrary.parse_command_line(
            unreal.SystemLibrary.get_command_line())
        sequence_path = parameters.get('LevelSequence')
        if not sequence_path:
            unreal.log_error('Park fly-through: missing LevelSequence argument.')
            self.on_executor_errored()
            return

        queue = unreal.new_object(unreal.MoviePipelineQueue, outer=self)
        job = queue.allocate_new_job(unreal.MoviePipelineExecutorJob)
        job.job_name = 'Park Fly-through'
        job.sequence = unreal.SoftObjectPath(sequence_path)
        settings = job.get_configuration()

        output = settings.find_or_add_setting_by_class(unreal.MoviePipelineOutputSetting)
        output.output_directory = unreal.DirectoryPath(
            'C:/Users/bpasc/OneDrive/Desktop/Pasta do Projeto/ProjectUnreal/Saved/VideoRenders/ParkFlythrough')
        output.file_name_format = 'ParkFlythrough.{frame_number}'
        output.output_resolution = unreal.IntPoint(1280, 720)
        output.use_custom_frame_rate = True
        output.output_frame_rate = unreal.FrameRate(24, 1)
        output.zero_pad_frame_numbers = 4

        settings.find_or_add_setting_by_class(unreal.MoviePipelineDeferredPassBase)
        settings.find_or_add_setting_by_class(unreal.MoviePipelineImageSequenceOutput_PNG)
        aa = settings.find_or_add_setting_by_class(unreal.MoviePipelineAntiAliasingSetting)
        aa.spatial_sample_count = 1
        aa.temporal_sample_count = 1
        aa.engine_warm_up_count = 16
        aa.render_warm_up_count = 8
        settings.initialize_transient_settings()

        self.active_pipeline = unreal.new_object(
            self.target_pipeline_class,
            outer=self.get_last_loaded_world(),
            base_type=unreal.MoviePipeline)
        self.active_pipeline.on_movie_pipeline_finished_delegate.add_function_unique(
            self, 'on_movie_pipeline_finished')
        self.active_pipeline.initialize(job)

    @unreal.ufunction(override=True)
    def on_begin_frame(self):
        super(ParkFlythroughExecutor, self).on_begin_frame()

    @unreal.ufunction(override=True)
    def is_rendering(self):
        return self.active_pipeline is not None

    @unreal.ufunction(ret=None, params=[unreal.MoviePipeline, bool])
    def on_movie_pipeline_finished(self, pipeline, success):
        unreal.log('Park fly-through render finished: ' + str(success))
        self.active_pipeline = None
        self.on_executor_finished_impl()
