function map = ss_pin_defines()

persistent cached stamp

[~, root] = ss_model_dir();
inc = fullfile(root, 'usr', 'inc');

headers = dir(fullfile(inc, '*.h'));

sig = '';
for k = 1:numel(headers)
    sig = [sig headers(k).name sprintf('%.8f', headers(k).datenum)];
end

if ~isempty(cached) && strcmp(stamp, sig)
    map = cached;
    return;
end

map = containers.Map('KeyType', 'char', 'ValueType', 'double');
alias = containers.Map('KeyType', 'char', 'ValueType', 'char');

for k = 1:numel(headers)
    read_header(fullfile(inc, headers(k).name), map, alias);
end

ks = keys(alias);
for pass = 1:4
    for i = 1:numel(ks)
        key = ks{i};
        if ~isKey(map, key) && isKey(map, alias(key))
            map(key) = map(alias(key));
        end
    end
end

cached = map;
stamp = sig;

end


function read_header(path, map, alias)

txt = fileread(path);

t = regexp(txt, '#\s*define\s+(\w+)\s+PIN\s*\(\s*''([A-Za-z])''\s*,\s*(\d+)\s*\)', 'tokens');
for i = 1:numel(t)
    map(upper(t{i}{1})) = (double(upper(t{i}{2})) - double('A')) * 16 + str2double(t{i}{3});
end

t = regexp(txt, '^\s*#\s*define\s+(\w+)\s+(\w+)\s*$', 'tokens', 'lineanchors');
for i = 1:numel(t)
    alias(upper(t{i}{1})) = upper(t{i}{2});
end

end
