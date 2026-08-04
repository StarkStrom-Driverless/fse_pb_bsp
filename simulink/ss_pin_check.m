function id = ss_pin_check(name, id)

if ~isnumeric(id) || ~isscalar(id)
    error('Pin "%s" does not resolve to a scalar pin id.', name);
end

id = double(id);

if isnan(id) || id < 0 || id > 255
    error('Pin "%s" resolves to %g, which is outside the 0..255 pin id range.', name, id);
end

end
