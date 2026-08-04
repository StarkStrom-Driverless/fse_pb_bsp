function mdl = ss_model_name()

[~, root] = ss_model_dir();
[~, base] = fileparts(root);

mdl = regexprep(base, '[^A-Za-z0-9_]', '_');
if isempty(mdl) || ~isletter(mdl(1))
    mdl = ['m_' mdl];
end

end
