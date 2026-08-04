function ss_simulink_mask_pin(blk, idx)

m = Simulink.Mask.get(blk);
names = strtrim(strsplit(get_param(blk, 'Parameters'), ','));

if idx > numel(names)
    error('Block %s has only %d parameters, pin index %d requested.', ...
          blk, numel(names), idx);
end

p = [];
for k = 1:numel(m.Parameters)
    if strcmp(m.Parameters(k).Name, names{idx})
        p = m.Parameters(k);
        break;
    end
end
if isempty(p)
    error('Pin parameter %s not found on %s.', names{idx}, blk);
end

var = sprintf('ss_pin_id_%d', idx);
str = sprintf('pin_str_%d', idx);

p.Name     = str;
p.Prompt   = 'Pin (e.g. PA5)';
p.Type     = 'edit';
p.Evaluate = 'off';
p.Value    = 'PA0';

init = [ ...
    'if numel(' str ') < 3 || upper(' str '(1)) ~= ''P''' newline ...
    '    error(''Pin must look like PA5.'');' newline ...
    'end' newline ...
    var ' = (double(upper(' str '(2))) - double(''A'')) * 16 + str2double(' str '(3:end));' newline ...
    'if isnan(' var ') || ' var ' < 0 || ' var ' > 255' newline ...
    '    error(''Pin must look like PA5.'');' newline ...
    'end' ...
];

if isempty(m.Initialization)
    m.Initialization = init;
else
    m.Initialization = [m.Initialization newline init];
end

names{idx} = var;
set_param(blk, 'Parameters', strjoin(names, ', '));

end
