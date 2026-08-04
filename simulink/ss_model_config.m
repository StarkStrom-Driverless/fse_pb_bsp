function ss_model_config(mdl, step_s)

if nargin < 2 || isempty(step_s)
    step_s = 0.001;
end

here = fileparts(mfilename('fullpath'));
bsp  = fullfile(here, '..');
[~, root] = ss_model_dir();

cs = getActiveConfigSet(mdl);

switchTarget(cs, 'ert.tlc', []);

set_param(cs, 'TargetLang', 'C');
set_param(cs, 'GenCodeOnly', 'on');
set_param(cs, 'SolverType', 'Fixed-step');
set_param(cs, 'SolverName', 'FixedStepDiscrete');
set_param(cs, 'FixedStep', num2str(step_s));
set_param(cs, 'ProdHWDeviceType', 'ARM Compatible->ARM Cortex-M');
set_param(cs, 'GenerateSampleERTMain', 'off');
set_param(cs, 'CodeInterfacePackaging', 'Nonreusable function');

soft_set(cs, 'EnableMultiTasking', 'off');
soft_set(cs, 'SingleTaskRateTransMsg', 'none');
soft_set(cs, 'SupportNonFinite', 'off');
soft_set(cs, 'SupportComplex', 'off');
soft_set(cs, 'DefaultParameterBehavior', 'Inlined');
soft_set(cs, 'CombineOutputUpdateFcns', 'on');
soft_set(cs, 'SuppressErrorStatus', 'on');
soft_set(cs, 'IncludeMdlTerminateFcn', 'off');
soft_set(cs, 'MatFileLogging', 'off');
soft_set(cs, 'UtilityFuncGeneration', 'Auto');
soft_set(cs, 'GenerateReport', 'off');
soft_set(cs, 'PackageGeneratedCodeAndArtifacts', 'off');
soft_set(cs, 'RTWVerbose', 'off');

set_param(cs, 'CustomInclude', ['"' fullfile(bsp, 'ss', 'inc') '" "' ...
                                 fullfile(root, 'usr', 'inc') '"']);

end


function soft_set(cs, name, value)

try
    set_param(cs, name, value);
catch err
    fprintf('skipped %s: %s\n', name, err.message);
end

end
