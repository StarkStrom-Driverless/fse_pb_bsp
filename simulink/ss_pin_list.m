function ss_pin_list()

map = ss_pin_defines();
names = sort(keys(map));

if isempty(names)
    fprintf('no pin defines found in usr/inc\n');
    return;
end

fprintf('usr/inc:\n');
for k = 1:numel(names)
    id = map(names{k});
    fprintf('  %-20s %3d   P%c%d\n', names{k}, id, ...
            char(double('A') + floor(id / 16)), mod(id, 16));
end

fprintf(['\nWorkspace variables are resolved through the model, so they are ' ...
         'not listed here.\nUse the Model Explorer to see the model workspace.\n']);

end
