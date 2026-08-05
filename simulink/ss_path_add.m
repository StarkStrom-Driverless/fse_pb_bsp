function ss_path_add()

out = ss_model_dir();
if exist(out, 'dir')
    addpath(out);
end

mods = ss_simulink_modules();
for k = 1:numel(mods)
    addpath(mods(k).dir);
end

end
