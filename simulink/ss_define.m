function value = ss_define(name, file)

if nargin < 2 || isempty(file)
    file = 'ss_config.h';
end

map = ss_config_values(file);

if isKey(map, name)
    value = map(name);
else
    value = [];
end

end
