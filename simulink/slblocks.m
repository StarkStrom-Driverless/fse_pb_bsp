function blkStruct = slblocks

mods = ss_simulink_modules();

Browser = struct('Library', {}, 'Name', {}, 'IsFlat', {});

for k = 1:numel(mods)
    lib = [mods(k).name '_lib'];
    if ~exist(fullfile(mods(k).dir, [lib '.slx']), 'file')
        continue;
    end
    Browser(end+1).Library = lib;
    Browser(end).Name      = upper(strrep(mods(k).name, '_', ' '));
    Browser(end).IsFlat    = 1;
end

blkStruct.Browser = Browser;

end
