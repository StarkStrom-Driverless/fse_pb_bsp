function m = lc_ss_pwm()

here   = fileparts(mfilename('fullpath'));
host   = fullfile(here, 'host');
ss_inc = fullfile(here, '..', '..', 'ss', 'inc');

def1 = legacy_code('initialize');
def1.SFunctionName = 'ss_pwm_write_sfcn';
def1.StartFcnSpec  = 'ss_pwm_init(uint16 p1, uint32 p2)';
def1.OutputFcnSpec = 'void ss_pwm_write(uint16 p1, uint32 u1)';

def2 = legacy_code('initialize');
def2.SFunctionName = 'ss_pwm_write_highres_sfcn';
def2.StartFcnSpec  = 'ss_pwm_init_highres(uint16 p1, uint32 p2)';
def2.OutputFcnSpec = 'void ss_pwm_write_highres(uint16 p1, uint32 u1)';

defs = [def1, def2];
for k = 1:numel(defs)
    defs(k).HeaderFiles = {'ss_pwm.h'};
    defs(k).SourceFiles = {'ss_pwm_stub.c'};
    defs(k).IncPaths    = {host, here, ss_inc};
    defs(k).SrcPaths    = {host};
    defs(k).SampleTime  = 'parameterized';
end

m.name = 'ss_pwm';
m.defs = defs;

m.blocks(1).sfcn = 'ss_pwm_write_sfcn';
m.blocks(1).name = 'SS PWM Write';
m.blocks(1).pin  = 1;
m.blocks(1).inputs  = {'duty'};
m.blocks(1).params(1).idx    = 2;
m.blocks(1).params(1).prompt = 'Frequency (Hz)';
m.blocks(1).params(1).value  = '1000';

m.blocks(2).sfcn = 'ss_pwm_write_highres_sfcn';
m.blocks(2).name = 'SS PWM Write Highres';
m.blocks(2).pin  = 1;
m.blocks(2).inputs  = {'duty'};
m.blocks(2).params(1).idx    = 2;
m.blocks(2).params(1).prompt = 'Frequency (Hz)';
m.blocks(2).params(1).value  = '1000';

end
