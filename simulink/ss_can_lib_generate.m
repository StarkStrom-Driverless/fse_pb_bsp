function ss_can_lib_generate(spec_file)

spec = jsondecode(fileread(spec_file));
msgs = as_cell(spec.messages);

if ~isfield(spec, 'with_valid')
    spec.with_valid = false;
end
if ~isfield(spec, 'tx_cycle_ms')
    spec.tx_cycle_ms = 100;
end
if ~isfield(spec, 'rx_poll_ms')
    spec.rx_poll_ms = 1;
end

out = ss_model_dir();
lib_name = ['can_' spec.name '_lib'];
lib_path = fullfile(out, [lib_name '.slx']);

if bdIsLoaded(lib_name)
    close_system(lib_name, 0);
end
if exist(lib_path, 'file')
    delete(lib_path);
end

new_system(lib_name, 'Library');

y = 0;
for k = 1:numel(msgs)
    build_rx(lib_name, msgs{k}, spec.channel, y, spec.with_valid, spec.rx_poll_ms);
    build_tx(lib_name, msgs{k}, spec.channel, y + 60, spec.tx_cycle_ms);
    y = y + 120;
end

set_param(lib_name, 'EnableLBRepository', 'on');
save_system(lib_name, lib_path);
close_system(lib_name, 0);

write_slblocks(out, lib_name, upper(strrep(spec.name, '_', ' ')));

fprintf('generated %s with %d messages\n', lib_path, numel(msgs));

end


function build_rx(lib_name, msg, channel, y, with_valid, poll_ms)

sub = [lib_name '/' msg.name ' RX'];
add_block('built-in/Subsystem', sub, 'Position', [40, y, 240, y + 40]);

rx = [sub '/CAN Frame'];
add_block('ss_can_lib/SS CAN Receive', rx, 'Position', [60, 40, 200, 100], ...
          'SParameter1', num2str(channel), 'SParameter2', num2str(msg.id), ...
          'SampleTime', 'rx_sample_time');

sigs = as_cell(msg.signals);

dec = [sub '/Decode Signals'];
add_block('built-in/Subsystem', dec, 'Position', [280, 40, 420, 60 * numel(sigs) + 60]);
add_block('built-in/EnablePort', [dec '/Enable Port'], 'Position', [90, 15, 110, 35]);
add_block('built-in/Inport', [dec '/Frame Data'], 'Position', [40, 60, 70, 80], 'Port', '1');

py = 40;
for k = 1:numel(sigs)
    build_decode(dec, sigs{k}, k, py);
    py = py + 60;
end

add_line(sub, 'CAN Frame/1', 'Decode Signals/1');

cp = get_param(rx, 'PortHandles');
dp = get_param(dec, 'PortHandles');
add_line(sub, cp.Outport(3), dp.Enable(1));

for k = 1:numel(sigs)
    port = [sub '/' sigs{k}.name];
    add_block('built-in/Outport', port, 'Position', [480, 60 * k, 510, 60 * k + 20], ...
              'Port', num2str(k));
    add_line(sub, sprintf('Decode Signals/%d', k), [sigs{k}.name '/1']);
end

if with_valid
    valid = [sub '/valid'];
    add_block('built-in/Outport', valid, 'Position', [480, py + 10, 510, py + 30], ...
              'Port', num2str(numel(sigs) + 1));
    add_line(sub, cp.Outport(3), get_param(valid, 'PortHandles').Inport(1));
end

term = [sub '/DLC Unused'];
add_block('built-in/Terminator', term, 'Position', [280, py + 60, 300, py + 80]);
add_line(sub, 'CAN Frame/2', 'DLC Unused/1');

mask = Simulink.Mask.create(sub);
mask.addParameter('Name', 'rx_poll_ms', 'Type', 'edit', 'Evaluate', 'on', ...
                  'Prompt', 'Poll interval (ms), 0 = inherit', ...
                  'Value', num2str(poll_ms));
mask.Initialization = [ ...
    'if rx_poll_ms > 0' newline ...
    '    rx_sample_time = rx_poll_ms / 1000;' newline ...
    'else' newline ...
    '    rx_sample_time = -1;' newline ...
    'end' ...
];

end


function build_decode(dec, s, idx, py)

if s.signed
    src = 'ss_can_lib/SS CAN Get Signal Signed';
else
    src = 'ss_can_lib/SS CAN Get Signal';
end

get = [dec '/' s.name '_get'];
add_block(src, get, 'Position', [140, py, 280, py + 40], ...
          'SParameter1', num2str(s.start), 'SParameter2', num2str(s.length));
add_line(dec, 'Frame Data/1', [s.name '_get/1']);

last = [s.name '_get'];
x = 320;

if s.scale ~= 1 || s.offset ~= 0
    cast = [s.name '_cast'];
    add_block('simulink/Signal Attributes/Data Type Conversion', [dec '/' cast], ...
              'Position', [x, py, x + 60, py + 40], 'OutDataTypeStr', 'single');
    add_line(dec, [last '/1'], [cast '/1']);
    last = cast;
    x = x + 100;
end

if s.scale ~= 1
    gain = [s.name '_scale'];
    add_block('simulink/Math Operations/Gain', [dec '/' gain], ...
              'Position', [x, py, x + 40, py + 40], ...
              'Gain', num2str(s.scale, '%.10g'), 'OutDataTypeStr', 'single');
    add_line(dec, [last '/1'], [gain '/1']);
    last = gain;
    x = x + 80;
end

if s.offset ~= 0
    bias = [s.name '_offset'];
    add_block('simulink/Math Operations/Bias', [dec '/' bias], ...
              'Position', [x, py, x + 40, py + 40], ...
              'Bias', num2str(s.offset, '%.10g'));
    add_line(dec, [last '/1'], [bias '/1']);
    last = bias;
    x = x + 80;
end

port = [dec '/' s.name];
add_block('built-in/Outport', port, 'Position', [x, py + 10, x + 30, py + 30], ...
          'Port', num2str(idx), 'OutputWhenDisabled', 'held', 'InitialOutput', '0');
add_line(dec, [last '/1'], [s.name '/1']);

end


function build_tx(lib_name, msg, channel, y, default_cycle_ms)

sub = [lib_name '/' msg.name ' TX'];
add_block('built-in/Subsystem', sub, 'Position', [320, y, 520, y + 40]);

zero = [sub '/Empty Frame'];
add_block('simulink/Sources/Constant', zero, 'Position', [40, 40, 100, 80], ...
          'Value', 'zeros(8,1)', 'OutDataTypeStr', 'uint8');

sigs = as_cell(msg.signals);
last = 'Empty Frame';
py = 40;

for k = 1:numel(sigs)
    s = sigs{k};

    port = [sub '/' s.name];
    add_block('built-in/Inport', port, 'Position', [40, py + 120, 70, py + 140], ...
              'Port', num2str(k));

    cast = [s.name '_cast'];
    add_block('simulink/Signal Attributes/Data Type Conversion', [sub '/' cast], ...
              'Position', [120, py + 120, 180, py + 160], 'OutDataTypeStr', 'uint32');
    add_line(sub, [s.name '/1'], [cast '/1']);

    set = [s.name '_set'];
    add_block('ss_can_lib/SS CAN Set Signal', [sub '/' set], ...
              'Position', [240, py, 380, py + 60], ...
              'SParameter1', num2str(s.start), 'SParameter2', num2str(s.length));
    add_line(sub, [last '/1'], [set '/1']);
    add_line(sub, [cast '/1'], [set '/2']);

    last = set;
    py = py + 100;
end

tx = [sub '/CAN Frame'];
add_block('ss_can_lib/SS CAN Send', tx, 'Position', [440, 40, 580, 100], ...
          'SParameter1', num2str(channel), 'SParameter2', num2str(msg.id), ...
          'SParameter3', num2str(msg.dlc), 'SampleTime', 'tx_sample_time');
add_line(sub, [last '/1'], 'CAN Frame/1');

en = [sub '/enable'];
add_block('built-in/Inport', en, 'Position', [280, py + 40, 310, py + 60], ...
          'Port', num2str(numel(sigs) + 1));

en_cast = [sub '/Enable Cast'];
add_block('simulink/Signal Attributes/Data Type Conversion', en_cast, ...
          'Position', [340, py + 30, 400, py + 70], 'OutDataTypeStr', 'uint8');
add_line(sub, 'enable/1', 'Enable Cast/1');
add_line(sub, 'Enable Cast/1', 'CAN Frame/2');

mask = Simulink.Mask.create(sub);
mask.addParameter('Name', 'tx_cycle_ms', 'Type', 'edit', 'Evaluate', 'on', ...
                  'Prompt', 'Cycle time (ms), 0 = every model step', ...
                  'Value', num2str(default_cycle_ms));
mask.Initialization = [ ...
    'if tx_cycle_ms > 0' newline ...
    '    tx_sample_time = tx_cycle_ms / 1000;' newline ...
    'else' newline ...
    '    tx_sample_time = -1;' newline ...
    'end' ...
];

end


function write_slblocks(out, lib_name, display_name)

f = fullfile(out, 'slblocks.m');
libs = {};
names = {};

d = dir(fullfile(out, 'can_*_lib.slx'));
for k = 1:numel(d)
    [~, lib] = fileparts(d(k).name);
    libs{end+1} = lib;
    if strcmp(lib, lib_name)
        names{end+1} = display_name;
    else
        names{end+1} = upper(strrep(strrep(lib, 'can_', ''), '_lib', ''));
    end
end

fid = fopen(f, 'w');
fprintf(fid, 'function blkStruct = slblocks\n\n');
fprintf(fid, 'Browser = struct(''Library'', {}, ''Name'', {}, ''IsFlat'', {});\n\n');
for k = 1:numel(libs)
    fprintf(fid, 'Browser(end+1).Library = ''%s'';\n', libs{k});
    fprintf(fid, 'Browser(end).Name      = ''CAN %s'';\n', names{k});
    fprintf(fid, 'Browser(end).IsFlat    = 1;\n\n');
end
fprintf(fid, 'blkStruct.Browser = Browser;\n\nend\n');
fclose(fid);

end


function c = as_cell(v)

if iscell(v)
    c = v;
elseif isstruct(v)
    c = num2cell(v);
else
    c = {v};
end

end
