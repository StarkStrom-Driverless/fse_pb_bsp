function id = PIN(bank, num)

if ~ischar(bank) && ~isstring(bank)
    error('PIN expects a bank letter, e.g. PIN(''C'', 4).');
end

id = (double(upper(char(bank))) - double('A')) * 16 + num;

if id < 0 || id > 255
    error('PIN(''%s'', %d) is outside the 0..255 pin id range.', bank, num);
end

end
