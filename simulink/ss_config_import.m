function ss_config_import(file)

if nargin < 1 || isempty(file)
    file = 'config.h';
end

map = ss_config_values(file);
names = keys(map);

for k = 1:numel(names)
    assignin('base', names{k}, map(names{k}));
end

fprintf('imported %d defines from usr/inc/%s\n', numel(names), file);

end
