function mods = ss_simulink_modules()

here = fileparts(mfilename('fullpath'));
roots = {here, ss_model_dir()};

mods = struct('name', {}, 'dir', {});

for r = 1:numel(roots)
    if ~exist(roots{r}, 'dir')
        continue;
    end

    d = dir(roots{r});
    for k = 1:numel(d)
        if ~d(k).isdir || d(k).name(1) == '.'
            continue;
        end

        mdir = fullfile(roots{r}, d(k).name);
        if ~exist(fullfile(mdir, ['lc_' d(k).name '.m']), 'file')
            continue;
        end

        mods(end + 1).name = d(k).name;
        mods(end).dir = mdir;
    end
end

end
