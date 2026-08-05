function m = lc_ss_queue()

here   = fileparts(mfilename('fullpath'));
host   = fullfile(here, 'host');
ss_inc = fullfile(here, '..', '..', 'ss', 'inc');

width = 64;

def1 = legacy_code('initialize');
def1.SFunctionName = 'ss_queue_pop_sfcn';
def1.StartFcnSpec  = 'ss_queue_sl_open(uint8 p1, uint16 p2, uint8 p3)';
def1.OutputFcnSpec = sprintf(['void ss_queue_sl_pop(uint8 p1, uint16 p2, ' ...
                              'uint8 y1[%d], uint32 y2[1], uint8 y3[1])'], width);

def2 = legacy_code('initialize');
def2.SFunctionName = 'ss_queue_push_sfcn';
def2.StartFcnSpec  = 'ss_queue_sl_open(uint8 p1, uint16 p2, uint8 p3)';
def2.OutputFcnSpec = sprintf(['void ss_queue_sl_push(uint8 p1, uint16 p2, ' ...
                              'uint8 u1[%d], uint32 u2, uint8 u3, uint8 y1[1])'], width);

def3 = legacy_code('initialize');
def3.SFunctionName = 'ss_queue_get_sfcn';
def3.OutputFcnSpec = sprintf(['uint32 y1 = ss_queue_sl_get(uint8 u1[%d], ' ...
                              'uint16 p1, uint8 p2, uint8 p3)'], width);

def4 = legacy_code('initialize');
def4.SFunctionName = 'ss_queue_get_signed_sfcn';
def4.OutputFcnSpec = sprintf(['int32 y1 = ss_queue_sl_get_signed(uint8 u1[%d], ' ...
                              'uint16 p1, uint8 p2, uint8 p3)'], width);

defs = [def1, def2, def3, def4];
for k = 1:numel(defs)
    defs(k).HeaderFiles = {'ss_queue_sl.h'};
    defs(k).SourceFiles = {'ss_queue_stub.c'};
    defs(k).IncPaths    = {host, here, ss_inc};
    defs(k).SrcPaths    = {host};
    defs(k).SampleTime  = 'parameterized';
end

m.name = 'ss_queue';
m.defs = defs;

m.blocks(1).sfcn = 'ss_queue_pop_sfcn';
m.blocks(1).name = 'SS Queue Pop';
m.blocks(1).pin  = 0;
m.blocks(1).outputs = {'data', 'length', 'overrun'};
m.blocks(1).params(1).idx    = 1;
m.blocks(1).params(1).prompt = 'Descriptor';
m.blocks(1).params(1).value  = '0';
m.blocks(1).params(2).idx    = 2;
m.blocks(1).params(2).prompt = sprintf('Payload size (bytes, max %d)', width);
m.blocks(1).params(2).value  = num2str(width);
m.blocks(1).params(3).idx    = 3;
m.blocks(1).params(3).prompt = 'Queue depth';
m.blocks(1).params(3).value  = '8';
m.blocks(1).params(4).idx    = 4;
m.blocks(1).params(4).prompt = 'Sample time (s)';
m.blocks(1).params(4).value  = '0.001';

m.blocks(2).sfcn = 'ss_queue_push_sfcn';
m.blocks(2).name = 'SS Queue Push';
m.blocks(2).pin  = 0;
m.blocks(2).inputs  = {'data', 'length', 'enable'};
m.blocks(2).outputs = {'overrun'};
m.blocks(2).params(1).idx    = 1;
m.blocks(2).params(1).prompt = 'Descriptor';
m.blocks(2).params(1).value  = '0';
m.blocks(2).params(2).idx    = 2;
m.blocks(2).params(2).prompt = sprintf('Payload size (bytes, max %d)', width);
m.blocks(2).params(2).value  = num2str(width);
m.blocks(2).params(3).idx    = 3;
m.blocks(2).params(3).prompt = 'Queue depth';
m.blocks(2).params(3).value  = '8';
m.blocks(2).params(4).idx    = 4;
m.blocks(2).params(4).prompt = 'Sample time (s)';
m.blocks(2).params(4).value  = '0.001';

m.blocks(3).sfcn = 'ss_queue_get_sfcn';
m.blocks(3).name = 'SS Queue Get Signal';
m.blocks(3).pin  = 0;
m.blocks(3).inputs  = {'data'};
m.blocks(3).outputs = {'value'};
m.blocks(3).params(1).idx    = 1;
m.blocks(3).params(1).prompt = 'Start bit';
m.blocks(3).params(1).value  = '0';
m.blocks(3).params(2).idx    = 2;
m.blocks(3).params(2).prompt = 'Length (bits)';
m.blocks(3).params(2).value  = '32';
m.blocks(3).params(3).idx    = 3;
m.blocks(3).params(3).prompt = 'Byte order (0 = intel, 1 = motorola)';
m.blocks(3).params(3).value  = '0';

m.blocks(4).sfcn = 'ss_queue_get_signed_sfcn';
m.blocks(4).name = 'SS Queue Get Signal Signed';
m.blocks(4).pin  = 0;
m.blocks(4).inputs  = {'data'};
m.blocks(4).outputs = {'value'};
m.blocks(4).params(1).idx    = 1;
m.blocks(4).params(1).prompt = 'Start bit';
m.blocks(4).params(1).value  = '0';
m.blocks(4).params(2).idx    = 2;
m.blocks(4).params(2).prompt = 'Length (bits)';
m.blocks(4).params(2).value  = '32';
m.blocks(4).params(3).idx    = 3;
m.blocks(4).params(3).prompt = 'Byte order (0 = intel, 1 = motorola)';
m.blocks(4).params(3).value  = '0';

end
