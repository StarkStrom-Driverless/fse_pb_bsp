function [out, root] = ss_model_dir()

here = fileparts(mfilename('fullpath'));
root = char(java.io.File(fullfile(here, '..', '..')).getCanonicalPath());
out  = fullfile(root, 'usr', 'simulink');

end
