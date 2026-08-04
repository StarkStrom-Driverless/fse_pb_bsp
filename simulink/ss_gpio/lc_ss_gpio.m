function m = lc_ss_gpio()

here   = fileparts(mfilename('fullpath'));
ss_inc = fullfile(here, '..', '..', 'ss', 'inc');

def1 = legacy_code('initialize');
def1.SFunctionName = 'ss_gpio_write_sfcn';
def1.OutputFcnSpec = 'void ss_io_write(uint16 p1, uint8 u1)';
def1.HeaderFiles   = {'ss_gpio.h'};
def1.SourceFiles   = {'ss_gpio_stub.c'};
def1.IncPaths      = {here, ss_inc};
def1.SrcPaths      = {here};
def1.SampleTime    = 'parameterized';

def2 = legacy_code('initialize');
def2.SFunctionName = 'ss_gpio_read_sfcn';
def2.OutputFcnSpec = 'uint16 y1 = ss_io_read(uint16 p1)';
def2.HeaderFiles   = {'ss_gpio.h'};
def2.SourceFiles   = {'ss_gpio_stub.c'};
def2.IncPaths      = {here, ss_inc};
def2.SrcPaths      = {here};
def2.SampleTime    = 'parameterized';

m.name = 'ss_gpio';
m.defs = [def1, def2];

m.blocks(1).sfcn = 'ss_gpio_write_sfcn';
m.blocks(1).name = 'SS GPIO Write';
m.blocks(1).pin  = 1;

m.blocks(2).sfcn = 'ss_gpio_read_sfcn';
m.blocks(2).name = 'SS GPIO Read';
m.blocks(2).pin  = 1;

end
