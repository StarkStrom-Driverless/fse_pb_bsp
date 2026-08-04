function blkStruct = slblocks

here = fileparts(mfilename('fullpath'));
mods = ss_simulink_modules();

Browser = struct('Library', {}, 'Name', {}, 'IsFlat', {});

for k = 1:numel(mods)
    lib = [mods{k} '_lib'];
    if ~exist(fullfile(here, mods{k}, [lib '.slx']), 'file')
        continue;
    end
    Browser(end+1).Library = lib;
    Browser(end).Name      = upper(strrep(mods{k}, '_', ' '));
    Browser(end).IsFlat    = 1;
end

blkStruct.Browser = Browser;

end
