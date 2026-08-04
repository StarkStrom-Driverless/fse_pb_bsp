function ss_simulink_mask_param(blk, idx, prompt, value)

m = Simulink.Mask.get(blk);
names = strtrim(strsplit(get_param(blk, 'Parameters'), ','));

if idx > numel(names)
    error('Block %s has only %d parameters, index %d requested.', ...
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
    error('Parameter %s not found on %s.', names{idx}, blk);
end

p.Prompt = prompt;
p.Value  = value;

end
