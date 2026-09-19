function map = ss_config_values(file)

if nargin < 1 || isempty(file)
    file = 'config.h';
end

map = containers.Map('KeyType', 'char', 'ValueType', 'double');

[~, root] = ss_model_dir();
path = fullfile(root, 'usr', 'inc', file);

if ~exist(path, 'file')
    return;
end

lines = regexp(fileread(path), '\r?\n', 'split');

for k = 1:numel(lines)
    tok = regexp(lines{k}, '^\s*#\s*define\s+([A-Za-z_]\w*)\s+([^/]+?)\s*(?:/[/*].*)?$', ...
                 'tokens', 'once');

    if isempty(tok)
        continue;
    end

    pin = regexp(tok{2}, '^PIN\s*\(\s*''([A-Za-z])''\s*,\s*(\d+)\s*\)$', 'tokens', 'once');

    if ~isempty(pin)
        map(tok{1}) = (double(upper(pin{1})) - double('A')) * 16 + str2double(pin{2});
        continue;
    end

    value = str2double(regexprep(tok{2}, '[fFuUlL]+$', ''));

    if ~isnan(value)
        map(tok{1}) = value;
    end
end

end
