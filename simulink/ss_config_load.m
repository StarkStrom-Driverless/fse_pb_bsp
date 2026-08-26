function ss_config_load()

out = ss_model_dir();
f = fullfile(out, 'config.m');

if ~exist(f, 'file')
    return;
end

mdl = ss_model_name();
mdl_path = fullfile(out, [mdl '.slx']);

if ~bdIsLoaded(mdl) && exist(mdl_path, 'file')
    load_system(mdl_path);
end

old = pwd;
guard = onCleanup(@() cd(old));
cd(out);

evalin('base', 'config;');

fprintf('loaded %s\n', f);

end
