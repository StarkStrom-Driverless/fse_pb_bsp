function m = lc_ss_iob()

here   = fileparts(mfilename('fullpath'));
host   = fullfile(here, 'host');
ss_inc = fullfile(here, '..', '..', 'ss', 'inc');

def1 = legacy_code('initialize');
def1.SFunctionName = 'ss_iob_read_sfcn';
def1.StartFcnSpec  = 'ss_iob_add(uint16 p1, uint8 p2)';
def1.OutputFcnSpec = 'uint8 y1 = ss_iob_get(uint16 p1)';
def1.HeaderFiles   = {'ss_iob.h'};
def1.SourceFiles   = {'ss_iob_stub.c'};
def1.IncPaths      = {host, here, ss_inc};
def1.SrcPaths      = {host};
def1.SampleTime    = 'parameterized';

m.name = 'ss_iob';
m.defs = def1;

m.blocks(1).sfcn = 'ss_iob_read_sfcn';
m.blocks(1).name = 'SS IOB Read';
m.blocks(1).pin  = 1;
m.blocks(1).outputs = {'state'};
m.blocks(1).params(1).idx    = 2;
m.blocks(1).params(1).prompt = 'Polarity (0 = rising, 1 = falling)';
m.blocks(1).params(1).value  = '0';

end
