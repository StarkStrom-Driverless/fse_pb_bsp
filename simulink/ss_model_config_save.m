function ss_model_config_save(mdl)

if nargin < 1 || isempty(mdl)
    mdl = ss_model_name();
end

out = ss_model_dir();
mdl_path = fullfile(out, [mdl '.slx']);

if ~exist(mdl_path, 'file')
    error('Model not found: %s\nRun ./ss matlab_init first.', mdl_path);
end

was_loaded = bdIsLoaded(mdl);
if ~was_loaded
    load_system(mdl_path);
end

ss_model_config(mdl);
save_system(mdl, mdl_path);

if ~was_loaded
    close_system(mdl, 0);
end

fprintf('config saved into %s\n', mdl_path);

end
