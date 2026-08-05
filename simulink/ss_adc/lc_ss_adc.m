function m = lc_ss_adc()

here   = fileparts(mfilename('fullpath'));
host   = fullfile(here, 'host');
ss_inc = fullfile(here, '..', '..', 'ss', 'inc');

def1 = legacy_code('initialize');
def1.SFunctionName = 'ss_adc_read_sfcn';
def1.StartFcnSpec  = 'ss_adc_init(uint16 p1)';
def1.OutputFcnSpec = 'void ss_adc_read(uint16 p1, uint16 y1[1])';
def1.HeaderFiles   = {'ss_adc.h'};
def1.SourceFiles   = {'ss_adc_stub.c'};
def1.IncPaths      = {host, here, ss_inc};
def1.SrcPaths      = {host};
def1.SampleTime    = 'parameterized';

m.name = 'ss_adc';
m.defs = def1;

m.blocks(1).sfcn = 'ss_adc_read_sfcn';
m.blocks(1).name = 'SS ADC Read';
m.blocks(1).pin  = 1;
m.blocks(1).outputs = {'raw'};

end
