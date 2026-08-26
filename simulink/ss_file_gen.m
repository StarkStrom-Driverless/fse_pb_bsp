function ss_file_gen()

out = ss_model_dir();

if ~exist(out, 'dir')
    mkdir(out);
end

try
    Simulink.fileGenControl('set', 'CodeGenFolder', out, 'CacheFolder', out, ...
                            'createDir', true);
catch err
    fprintf('file generation folder not set: %s\n', err.message);
end

end
