function mods = ss_simulink_modules()

here = fileparts(mfilename('fullpath'));
d = dir(here);

mods = {};
for k = 1:numel(d)
    if ~d(k).isdir || d(k).name(1) == '.'
        continue;
    end
    if exist(fullfile(here, d(k).name, ['lc_' d(k).name '.m']), 'file')
        mods{end+1} = d(k).name;
    end
end

end
