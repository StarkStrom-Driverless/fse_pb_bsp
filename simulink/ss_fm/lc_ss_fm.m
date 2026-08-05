function m = lc_ss_fm()

here   = fileparts(mfilename('fullpath'));
host   = fullfile(here, 'host');
ss_inc = fullfile(here, '..', '..', 'ss', 'inc');

def1 = legacy_code('initialize');
def1.SFunctionName = 'ss_fm_read_sfcn';
def1.StartFcnSpec  = 'ss_fm_init(uint16 p1, uint32 p2)';
def1.OutputFcnSpec = 'void ss_fm_read(uint16 p1, single y1[1])';
def1.HeaderFiles   = {'ss_fm.h'};
def1.SourceFiles   = {'ss_fm_stub.c'};
def1.IncPaths      = {host, here, ss_inc};
def1.SrcPaths      = {host};
def1.SampleTime    = 'parameterized';

m.name = 'ss_fm';
m.defs = def1;

m.blocks(1).sfcn = 'ss_fm_read_sfcn';
m.blocks(1).name = 'SS FM Read';
m.blocks(1).pin  = 1;
m.blocks(1).outputs = {'freq'};
m.blocks(1).params(1).idx    = 2;
m.blocks(1).params(1).prompt = 'Resolution (Hz)';
m.blocks(1).params(1).value  = '1000000';

end
