function build_model(mdl)

if nargin < 1 || isempty(mdl)
    mdl = ss_model_name();
end

out = ss_model_dir();
mdl_path = fullfile(out, [mdl '.slx']);

if ~exist(mdl_path, 'file')
    error('Model not found: %s\nRun ./ss matlab_init first.', mdl_path);
end

Simulink.fileGenControl('set', 'CodeGenFolder', out, 'CacheFolder', out, ...
                        'createDir', true);

load_system(mdl_path);
guard = onCleanup(@() close_system(mdl, 0));

ss_model_config(mdl);
slbuild(mdl);

fprintf('\nGenerated into: %s\n', fullfile(out, [mdl '_ert_rtw']));

end
