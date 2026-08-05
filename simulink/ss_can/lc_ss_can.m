function m = lc_ss_can()

here   = fileparts(mfilename('fullpath'));
host   = fullfile(here, 'host');
ss_inc = fullfile(here, '..', '..', 'ss', 'inc');

def1 = legacy_code('initialize');
def1.SFunctionName = 'ss_can_receive_sfcn';
def1.StartFcnSpec  = 'void ss_can_sl_open(void **work1, uint8 p1, uint32 p2)';
def1.OutputFcnSpec = 'void ss_can_sl_read(void *work1, uint8 y1[8], uint8 y2[1], uint8 y3[1])';

def2 = legacy_code('initialize');
def2.SFunctionName = 'ss_can_get_signal_sfcn';
def2.OutputFcnSpec = 'uint32 y1 = ss_can_sl_get_signal(uint8 u1[8], uint8 p1, uint8 p2, uint8 p3)';

def5 = legacy_code('initialize');
def5.SFunctionName = 'ss_can_get_signal_signed_sfcn';
def5.OutputFcnSpec = 'int32 y1 = ss_can_sl_get_signal_signed(uint8 u1[8], uint8 p1, uint8 p2, uint8 p3)';

def3 = legacy_code('initialize');
def3.SFunctionName = 'ss_can_set_signal_sfcn';
def3.OutputFcnSpec = 'void ss_can_sl_set_signal(uint8 u1[8], uint8 p1, uint8 p2, uint32 u2, uint8 p3, uint8 y1[8])';

def4 = legacy_code('initialize');
def4.SFunctionName = 'ss_can_send_sfcn';
def4.OutputFcnSpec = 'void ss_can_sl_send(uint8 p1, uint32 p2, uint8 p3, uint8 u1[8], uint8 u2)';

defs = [def1, def2, def5, def3, def4];
for k = 1:numel(defs)
    defs(k).HeaderFiles = {'ss_can_sl.h'};
    defs(k).SourceFiles = {'ss_can_stub.c'};
    defs(k).IncPaths    = {host, here, ss_inc};
    defs(k).SrcPaths    = {host};
    defs(k).SampleTime  = 'parameterized';
end

m.name = 'ss_can';
m.defs = defs;

m.blocks(1).sfcn = 'ss_can_receive_sfcn';
m.blocks(1).name = 'SS CAN Receive';
m.blocks(1).pin  = 0;
m.blocks(1).params(1).idx    = 1;
m.blocks(1).params(1).prompt = 'CAN channel (1 or 2)';
m.blocks(1).params(1).value  = '1';
m.blocks(1).params(2).idx    = 2;
m.blocks(1).params(2).prompt = 'CAN ID (e.g. 0x190)';
m.blocks(1).params(2).value  = '0x100';

m.blocks(2).sfcn = 'ss_can_get_signal_sfcn';
m.blocks(2).name = 'SS CAN Get Signal';
m.blocks(2).pin  = 0;
m.blocks(2).params(1).idx    = 1;
m.blocks(2).params(1).prompt = 'Start bit';
m.blocks(2).params(1).value  = '0';
m.blocks(2).params(2).idx    = 2;
m.blocks(2).params(2).prompt = 'Length (bits)';
m.blocks(2).params(2).value  = '8';
m.blocks(2).params(3).idx    = 3;
m.blocks(2).params(3).prompt = 'Byte order (0 = intel, 1 = motorola)';
m.blocks(2).params(3).value  = '0';

m.blocks(5).sfcn = 'ss_can_get_signal_signed_sfcn';
m.blocks(5).name = 'SS CAN Get Signal Signed';
m.blocks(5).pin  = 0;
m.blocks(5).params(1).idx    = 1;
m.blocks(5).params(1).prompt = 'Start bit';
m.blocks(5).params(1).value  = '0';
m.blocks(5).params(2).idx    = 2;
m.blocks(5).params(2).prompt = 'Length (bits)';
m.blocks(5).params(2).value  = '8';
m.blocks(5).params(3).idx    = 3;
m.blocks(5).params(3).prompt = 'Byte order (0 = intel, 1 = motorola)';
m.blocks(5).params(3).value  = '0';

m.blocks(3).sfcn = 'ss_can_set_signal_sfcn';
m.blocks(3).name = 'SS CAN Set Signal';
m.blocks(3).pin  = 0;
m.blocks(3).params(1).idx    = 1;
m.blocks(3).params(1).prompt = 'Start bit';
m.blocks(3).params(1).value  = '0';
m.blocks(3).params(2).idx    = 2;
m.blocks(3).params(2).prompt = 'Length (bits)';
m.blocks(3).params(2).value  = '8';
m.blocks(3).params(3).idx    = 3;
m.blocks(3).params(3).prompt = 'Byte order (0 = intel, 1 = motorola)';
m.blocks(3).params(3).value  = '0';

m.blocks(4).sfcn = 'ss_can_send_sfcn';
m.blocks(4).name = 'SS CAN Send';
m.blocks(4).pin  = 0;
m.blocks(4).params(1).idx    = 1;
m.blocks(4).params(1).prompt = 'CAN channel (1 or 2)';
m.blocks(4).params(1).value  = '1';
m.blocks(4).params(2).idx    = 2;
m.blocks(4).params(2).prompt = 'CAN ID (e.g. 0x190)';
m.blocks(4).params(2).value  = '0x100';
m.blocks(4).params(3).idx    = 3;
m.blocks(4).params(3).prompt = 'DLC';
m.blocks(4).params(3).value  = '8';

end
