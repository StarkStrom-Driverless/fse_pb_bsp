function mdl_path = ss_model_create(mdl)

if nargin < 1 || isempty(mdl)
    mdl = ss_model_name();
end

out = ss_model_dir();
if ~exist(out, 'dir')
    mkdir(out);
end

mdl_path = fullfile(out, [mdl '.slx']);

if exist(mdl_path, 'file')
    fprintf('model already exists: %s\n', mdl_path);
    return;
end

if bdIsLoaded(mdl)
    close_system(mdl, 0);
end

new_system(mdl, 'Model');
ss_model_config(mdl);
save_system(mdl, mdl_path);
close_system(mdl, 0);

fprintf('model created: %s\n', mdl_path);

end
