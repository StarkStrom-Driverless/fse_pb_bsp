function ss_path_install()

here = fileparts(mfilename('fullpath'));

up = userpath;
if isempty(up)
    userpath('reset');
    up = userpath;
end
if isempty(up)
    error('userpath is empty, cannot install startup.m');
end
if ~exist(up, 'dir')
    mkdir(up);
end

tag  = '% ss_simulink_path';
line = sprintf('addpath(genpath(''%s'')); addpath(''%s''); %s', ...
               here, ss_model_dir(), tag);

f = fullfile(up, 'startup.m');
keep = {};
if exist(f, 'file')
    lines = strsplit(fileread(f), newline);
    for k = 1:numel(lines)
        if isempty(strfind(lines{k}, tag)) && ~isempty(strtrim(lines{k}))
            keep{end+1} = lines{k};
        end
    end
end

fid = fopen(f, 'w');
if fid < 0
    error('Cannot write %s', f);
end
fprintf(fid, '%s\n', keep{:});
fprintf(fid, '%s\n', line);
fclose(fid);

addpath(genpath(here));
if exist(ss_model_dir(), 'dir')
    addpath(ss_model_dir());
end

fprintf('startup.m updated: %s\n', f);

end
