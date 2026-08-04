function ss_simulink_build(varargin)

here = fileparts(mfilename('fullpath'));
mods = ss_simulink_modules();

if ~isempty(varargin)
    mods = intersect(mods, varargin, 'stable');
end

if isempty(mods)
    error('No simulink modules found in %s', here);
end

orig_dir = pwd;
guard = onCleanup(@() cd(orig_dir));

for k = 1:numel(mods)
    fprintf('building %s\n', mods{k});
    build_module(here, mods{k});
end

sl_refresh_customizations;

fprintf('done: %s\n', strjoin(mods, ', '));

end


function build_module(root, name)

mdir = fullfile(root, name);
cd(mdir);

m = feval(['lc_' name]);

legacy_code('sfcn_cmex_generate', m.defs);
legacy_code('compile', m.defs);
legacy_code('sfcn_tlc_generate', m.defs);

lib_name = [name '_lib'];
lib_path = fullfile(mdir, [lib_name '.slx']);

if bdIsLoaded(lib_name)
    close_system(lib_name, 0);
end
if exist(lib_path, 'file')
    delete(lib_path);
end

new_system(lib_name, 'Library');
legacy_code('slblock_generate', m.defs, lib_name);

for k = 1:numel(m.blocks)
    finish_block(lib_name, m.blocks(k));
end

set_param(lib_name, 'EnableLBRepository', 'on');
save_system(lib_name, lib_path);
close_system(lib_name, 0);

end


function finish_block(lib_name, spec)

blocks = find_system(lib_name, 'SearchDepth', 1, 'Type', 'Block');
hit = '';
for k = 1:numel(blocks)
    if strcmp(get_param(blocks{k}, 'Name'), spec.sfcn)
        hit = blocks{k};
        break;
    end
end
if isempty(hit)
    error('No block named %s in %s. Present: %s', spec.sfcn, lib_name, ...
          strjoin(cellfun(@(b) get_param(b, 'Name'), blocks, 'UniformOutput', false), ', '));
end

if isfield(spec, 'params')
    for k = 1:numel(spec.params)
        ss_simulink_mask_param(hit, spec.params(k).idx, ...
                               spec.params(k).prompt, spec.params(k).value);
    end
end

if spec.pin > 0
    ss_simulink_mask_pin(hit, spec.pin);
end

set_param(hit, 'Name', spec.name);

end
