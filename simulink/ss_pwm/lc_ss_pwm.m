function m = lc_ss_pwm()

here   = fileparts(mfilename('fullpath'));
host   = fullfile(here, 'host');
ss_inc = fullfile(here, '..', '..', 'ss', 'inc');

def1 = legacy_code('initialize');
def1.SFunctionName = 'ss_pwm_write_sfcn';
def1.StartFcnSpec  = 'ss_pwm_init(uint16 p1, uint32 p2)';
def1.OutputFcnSpec = 'void ss_pwm_write(uint16 p1, uint32 u1)';
def1.HeaderFiles   = {'ss_pwm.h'};
def1.SourceFiles   = {'ss_pwm_stub.c'};
def1.IncPaths      = {host, here, ss_inc};
def1.SrcPaths      = {host};
def1.SampleTime    = 'parameterized';

m.name = 'ss_pwm';
m.defs = def1;

m.blocks(1).sfcn = 'ss_pwm_write_sfcn';
m.blocks(1).name = 'SS PWM Write';
m.blocks(1).pin  = 1;
m.blocks(1).params(1).idx    = 2;
m.blocks(1).params(1).prompt = 'Frequency (Hz)';
m.blocks(1).params(1).value  = '1000';

end
