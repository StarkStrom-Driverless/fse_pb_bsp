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

pin = sprintf('pin_%d', idx);

p.Name     = pin;
p.Prompt   = 'Pin';
p.Type     = 'edit';
p.Evaluate = 'on';
p.Value    = 'PIN(''A'', 0)';

check = ['ss_pin_check(''' pin ''', ' pin ');'];
if isempty(m.Initialization)
    m.Initialization = check;
else
    m.Initialization = [m.Initialization newline check];
end

names{idx} = pin;
set_param(blk, 'Parameters', strjoin(names, ', '));

end
