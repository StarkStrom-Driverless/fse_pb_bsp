function ss_config_load()

out = ss_model_dir();
f = fullfile(out, 'config.m');

if ~exist(f, 'file')
    return;
end

old = pwd;
guard = onCleanup(@() cd(old));
cd(out);

evalin('base', 'config;');

fprintf('loaded %s\n', f);

end
