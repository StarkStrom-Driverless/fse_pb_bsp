function id = ss_pin_literal(name)

name = strtrim(name);

tok = regexp(upper(name), '^P([A-Z])(\d+)$', 'tokens', 'once');
if ~isempty(tok)
    id = (double(tok{1}) - double('A')) * 16 + str2double(tok{2});
    return;
end

map = ss_pin_defines();
if isKey(map, upper(name))
    id = map(upper(name));
    return;
end

known = sort(keys(map));
if isempty(known)
    error(['Unknown pin "%s". Use PA5 style, a workspace variable, ' ...
           'PIN(''C'', 4), or a define from usr/inc.'], name);
end

error(['Unknown pin "%s". Use PA5 style, a workspace variable, ' ...
       'PIN(''C'', 4), or one of: %s'], name, strjoin(known, ', '));

end
